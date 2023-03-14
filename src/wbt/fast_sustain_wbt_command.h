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

#pragma once

#include "src/wbt/wbt_command.h"
#include "src/wbt/fast_sustain/io_generator.h"

struct pos_io;

namespace pos
{
class FastSustainWbtCommand : public WbtCommand
{
public:
    FastSustainWbtCommand(void);
    virtual ~FastSustainWbtCommand(void);

    virtual int Execute(Args& argv, JsonElement& elem);

private:
    static const int VALID_BLOCK_RATIO = 30;
    static const int BLOCK_SIZE_512B = 512;
    static const int BYTES_IN_128KB = 131072;
    static const int ALLOC_COUNT_512B = BYTES_IN_128KB / BLOCK_SIZE_512B;

    pos_io* GenerateWriteIoPacket(int arrayId, int volumeId, uint64_t rba, void* mem);
    bool IssueIo(int arrayId, int volumeId, uint64_t lba);
    void WaitAllIoDone(uint32_t waitIoCount);
    uint32_t CalculateNumOfIoCountToStartInvalidate(uint32_t numOfIoCount);
};

} // namespace pos
