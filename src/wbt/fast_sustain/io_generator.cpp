/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
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
#include "src/io/frontend_io/aio.h"
#include "src/logger/logger.h"
#include "src/include/smart_ptr_type.h"
#include "src/volume/volume_manager.h"

using namespace std;

namespace pos
{
IoGenerator::IoGenerator(pos_io* posIo_, int arrayId_, int volumeId_)
: posIo(posIo_),
  arrayId(arrayId_),
  volumeId(volumeId_)
{
}

bool
IoGenerator::Execute(void)
{
    AIO aio;
    VolumeIoSmartPtr volIo = aio.CreateVolumeIo(*posIo);

    IVolumeIoManager* volumeIoManager =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayId);
    if (unlikely(EID(SUCCESS) !=
        volumeIoManager->IncreasePendingIOCountIfNotZero(volumeId,
            static_cast<VolumeIoType>(posIo->ioType))))
    {
        // TODO EID
        POS_TRACE_WARN(-1,
            "Increase pending io count was failed, array id: {}, volume id: {}",
            arrayId, volumeId);

        if (nullptr != posIo->iov->iov_base)
        {
            pos::Memory<512>::Free(posIo->iov->iov_base);
        }
        return false;
    }

    aio.SubmitAsyncIO(volIo);

    return true;
}
} // namespace pos
