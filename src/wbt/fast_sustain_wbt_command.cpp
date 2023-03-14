/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/wbt/fast_sustain/io_generator.h"

#include "src/include/smart_ptr_type.h"
#include "src/io/frontend_io/aio.h"
#include "src/wbt/fast_sustain_wbt_command.h"
#include "src/io/frontend_io/unvmf_io_handler.h"
#include "src/volume/volume_manager.h"
#include "src/event_scheduler/io_completer.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/allocator/i_context_manager.h"

#include <atomic>

namespace pos
{
FastSustainWbtCommand::FastSustainWbtCommand(void)
: WbtCommand(FAST_SUSTAIN, "fast_sustain")
{
}
// LCOV_EXCL_START
FastSustainWbtCommand::~FastSustainWbtCommand(void)
{
}
// LCOV_EXCL_STOP

volatile static uint32_t doneCount = 0;
volatile static uint64_t lba = 0;

void
complete_io(struct pos_io* posIo, int status)
{
    if (nullptr != posIo->iov->iov_base)
    {
        pos::Memory<512>::Free(posIo->iov->iov_base);
    }

    doneCount++;
    delete posIo->iov;
    delete posIo;
}

pos_io*
FastSustainWbtCommand::GenerateWriteIoPacket(int arrayId, int volumeId, uint64_t rba, void* mem)
{
    pos_io* posIo = new pos_io;
    struct iovec* iov = new iovec;

    char arrayNameDefault[32] = "POSArray";

    posIo->ioType = IO_TYPE::WRITE;
    posIo->arrayName = arrayNameDefault;
    posIo->array_id = arrayId;
    posIo->volume_id = volumeId;
    posIo->offset = ChangeSectorToByte(rba);

    posIo->length = BYTES_IN_128KB;
    posIo->iov = iov;
    iov->iov_base = (char*)mem;
    posIo->iovcnt = 1;

    posIo->complete_cb = complete_io;

    return posIo;
}

void
FastSustainWbtCommand::WaitAllIoDone(uint32_t waitIoCount)
{
    while (doneCount != waitIoCount)
    {
        usleep(1);
    }

    doneCount = 0;
}

bool
FastSustainWbtCommand::IssueIo(int arrayId, int volumeId, uint64_t lba)
{
    void* bufferPool = pos::Memory<BLOCK_SIZE_512B>::Alloc(ALLOC_COUNT_512B);
    pos_io* posIo = GenerateWriteIoPacket(arrayId, volumeId, lba, bufferPool);
    EventSmartPtr submitIoEvent(new IoGenerator(posIo, arrayId, volumeId));
    return SpdkEventScheduler::SendSpdkEvent(0, submitIoEvent);
}

uint32_t
FastSustainWbtCommand::CalculateNumOfIoCountToStartInvalidate(uint32_t numOfIoCount)
{
    uint32_t numOfIoCountToStartInvalidate = 0;
    if (0 != (numOfIoCount % 2))
    {
        numOfIoCountToStartInvalidate = (numOfIoCount + 1) / 2;
    }
    else
    {
        numOfIoCountToStartInvalidate = numOfIoCount / 2;
    }
    return numOfIoCountToStartInvalidate;
}

int
FastSustainWbtCommand::Execute(Args& argv, JsonElement& elem)
{
    POS_TRACE_INFO(-1, "Start fast sustain WBT command");
    vector<const ComponentsInfo*> arrayInfoList = ArrayMgr()->GetInfo();
    if (0 == arrayInfoList.size())
    {
        int errorId = EID(LIST_VOL_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_ERROR(errorId, "Failed to retrieve array list.");
        return errorId;
    }

    for (const ComponentsInfo* ci : arrayInfoList)
    {
        IArrayInfo* arrayInfo = ci->arrayInfo;
        if (ArrayStateEnum::NORMAL != arrayInfo->GetState())
        {
            continue;
        }

        int arrayId = arrayInfo->GetIndex();
        IVolumeInfoManager* volumeInfoManager = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayId);
        VolumeList* volumeList = volumeInfoManager->GetVolumeList();

        int volumeId = -1;
        while (true)
        {
            VolumeBase* vol = volumeList->Next(volumeId);
            if (nullptr == vol)
            {
                break;
            }

            uint32_t capacity = vol->GetTotalSize() - 1;
            lba = 0;
            doneCount = 0;
            uint64_t endLba = (lba + (capacity / BLOCK_SIZE_512B)) - 256;
            uint64_t validBlckEndLba = (endLba * VALID_BLOCK_RATIO) / 100;

            POS_TRACE_INFO(-1, "arrayId {}, volume{}, capacity {}, lba {}, endLba {}",
                arrayId, volumeId, capacity, lba, endLba);

            while (true)
            {
                volatile uint32_t issueCount = 0;
                while (lba <= validBlckEndLba)
                {
                    if (lba + ((BYTES_IN_128KB / BLOCK_SIZE_512B)) >= validBlckEndLba)
                    {
                        break;
                    }

                    bool success = IssueIo(arrayId, volumeId, lba);
                    if (false == success)
                    {
                        POS_TRACE_ERROR(-1, "send spdk event error, origirnCore {} \n", 0);
                        break;
                    }
                    issueCount++;
                    lba += (BYTES_IN_128KB / BLOCK_SIZE_512B);
                }

                while (issueCount != doneCount)
                {
                    usleep(1);
                }

                for (int i = 0; i < 2; i++)
                {
                    lba = validBlckEndLba;
                    uint32_t secondFillEndLba = (lba + endLba) / 2;
                    while (lba <= secondFillEndLba)
                    {
                        if (lba + ((BYTES_IN_128KB / BLOCK_SIZE_512B)) >= secondFillEndLba)
                        {
                            break;
                        }

                        bool success = IssueIo(arrayId, volumeId, lba);
                        if (false == success)
                        {
                            POS_TRACE_ERROR(-1, "send spdk event error, origirnCore {} \n", 0);
                            break;
                        }

                        issueCount++;
                        lba += (BYTES_IN_128KB / BLOCK_SIZE_512B);
                    }

                    while (issueCount != doneCount)
                    {
                        usleep(1);
                    }
                }

                IContextManager* contextManager = AllocatorServiceSingleton::Instance()->GetIContextManager("POSArray");
                uint32_t numOfFreeSegment = contextManager->GetSegmentCtx()->GetNumOfFreeSegment();

                POS_TRACE_INFO(-1, "volume {} write, numOfFreeSegment {}", volumeId, numOfFreeSegment);

                break;
            }
        }
    }

    return 0;
}
} // namespace pos
