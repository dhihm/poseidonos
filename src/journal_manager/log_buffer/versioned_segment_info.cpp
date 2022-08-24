/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "versioned_segment_info.h"

#include <unordered_map>

namespace pos
{
VersionedSegmentInfo::VersionedSegmentInfo(void)
{
    pthread_rwlock_init(&lock, nullptr);
}

VersionedSegmentInfo::~VersionedSegmentInfo(void)
{
    changedValidBlockCount.clear();
    changedOccupiedStripeCount.clear();

    pthread_rwlock_destroy(&lock);
}

void
VersionedSegmentInfo::Reset(void)
{
    pthread_rwlock_wrlock(&lock);
    changedOccupiedStripeCount.clear();
    changedValidBlockCount.clear();
    pthread_rwlock_unlock(&lock);
}

void
VersionedSegmentInfo::IncreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    pthread_rwlock_wrlock(&lock);
    changedValidBlockCount[segId] += cnt;
    pthread_rwlock_unlock(&lock);
}

void
VersionedSegmentInfo::DecreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    pthread_rwlock_wrlock(&lock);
    changedValidBlockCount[segId] -= cnt;
    pthread_rwlock_unlock(&lock);
}

void
VersionedSegmentInfo::IncreaseOccupiedStripeCount(SegmentId segId)
{
    pthread_rwlock_wrlock(&lock);
    changedOccupiedStripeCount[segId]++;
    pthread_rwlock_unlock(&lock);
}

std::unordered_map<SegmentId, int>
VersionedSegmentInfo::GetChangedValidBlockCount(void)
{
    pthread_rwlock_rdlock(&lock);
    auto var = this->changedValidBlockCount;
    pthread_rwlock_unlock(&lock);
    return var;
}

std::unordered_map<SegmentId, uint32_t>
VersionedSegmentInfo::GetChangedOccupiedStripeCount(void)
{
    pthread_rwlock_rdlock(&lock);
    auto var = this->changedOccupiedStripeCount;
    pthread_rwlock_unlock(&lock);
    return var;
}
} // namespace pos
