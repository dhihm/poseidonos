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

#include "src/include/smart_ptr_type.h"
#include "src/io/frontend_io/aio.h"
#include "src/wbt/fast_sustain_wbt_command.h"
#include "src/io/frontend_io/unvmf_io_handler.h"
#include "src/volume/volume_manager.h"

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
int
FastSustainWbtCommand::Execute(Args& argv, JsonElement& elem)
{
    int result = InternalExecute();
    return result;
}

void
frontend_io_complete(struct pos_io* posIo, int status)
{
    delete posIo->iov;
    delete posIo;
}

int
FastSustainWbtCommand::InternalExecute(void)
{
    int arrayId = 0;
    int volumeId = 0;
    int rba = 0;
    int lastRba = -1;
    int numChunks = 128;

    vector<const ComponentsInfo*> arrayInfoList = ArrayMgr()->GetInfo();
    if (0 == arrayInfoList.size())
    {
        int errorId = EID(LIST_VOL_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_ERROR(errorId, "Failed to retrieve array list.");
        return errorId;
    }

    void* mem = pos::Memory<512>::Alloc(128 * 1 * 1);
    char arrayNameDefault[32] = "POSArray";

    for (const ComponentsInfo* ci : arrayInfoList)
    {
        IArrayInfo* arrayInfo = ci->arrayInfo;
        if (ArrayStateEnum::NORMAL != arrayInfo->GetState())
        {
            // POS_TRACE_WARN(EID(),
            //     "Fast sustain I/O skipped. Array {}, state {} ", arrayInfo->GetName(), arrayInfo->GetState());
            continue;
        }

        arrayId = arrayInfo->GetIndex();
        const PartitionLogicalSize* partitionSize = arrayInfo->GetSizeInfo(PartitionType::USER_DATA);
        numChunks = partitionSize->blksPerChunk;
        lastRba = 130023423;
        // Get Volume Id

        while (rba <= lastRba)
        {
            UNVMfCompleteHandler();

            // make volume io
            AIO aio;
            pos_io* posIo = new pos_io;
            struct iovec* iov = new iovec;

            posIo->ioType = IO_TYPE::WRITE;
            posIo->array_id = arrayId;
            posIo->volume_id = volumeId;
            posIo->offset = ChangeSectorToByte(rba);
            posIo->length = 4096;
            posIo->iov = iov;
            posIo->arrayName = arrayNameDefault;
            iov->iov_base = (char*)mem + ((rba / posIo->length) * 4096);
            posIo->iovcnt = 1;
            posIo->complete_cb = frontend_io_complete;

            POS_TRACE_INFO(-1, "posIo: array_id {}, volume_id {}, offset {}",
                posIo->array_id, posIo->volume_id, posIo->offset);

            VolumeIoSmartPtr volIo = aio.CreateVolumeIo(*posIo);

            IVolumeIoManager* volumeManager = VolumeServiceSingleton::Instance()->GetVolumeManager(posIo->array_id);
            if (unlikely(EID(SUCCESS) != volumeManager->IncreasePendingIOCountIfNotZero(posIo->volume_id, static_cast<VolumeIoType>(posIo->ioType))))
            {
                // TODO
                // IoCompleter ioCompleter(volumeIo);
                // ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::VOLUME_UMOUNTED, true);
                return POS_IO_STATUS_SUCCESS;
            }

            aio.SubmitAsyncIO(volIo);

            // UNVMfSubmitHandler(&posIo);

            // Update rba (TODO: 128KB write)
            rba += 8; // length / BLOCK_SIZE (512)
        }

        UNVMfCompleteHandler();
    }

    return 0;
}
} // namespace pos
