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

#ifndef GC_DEBUG_INFO_H_
#define GC_DEBUG_INFO_H_

#include "mk/ibof_config.h"
#include "src/allocator/include/allocator_const.h"
#include "src/gc/copier.h"
#include "cereal/archives/json.hpp"

namespace pos
{
class GcFlushSubmission;
class GcMapUpdateRequest;
class ReverseMapLoadCompletion;
class StripeCopySubmission;

struct AllocatedSegmentInfo
{
    int segmentId;
    time_t allocatedTime;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(segmentId),
            CEREAL_NVP(allocatedTime));
    }
};

struct VictimSegmentInfo
{
    int segmentId;
    int arrayId;
    int numFreeSegments;
    int validBlockCount;
    time_t selectedTime;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(segmentId),
            CEREAL_NVP(arrayId),
            CEREAL_NVP(numFreeSegments), 
            CEREAL_NVP(validBlockCount),
            CEREAL_NVP(selectedTime));
    }
};

struct FreedSegmentInfo
{
    int segmentId;
    time_t freedTime;
};

struct CopierState
{
    CopierStateType state;
    time_t beginTime;
    time_t endTime;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(state),
            CEREAL_NVP(beginTime),
            CEREAL_NVP(endTime));
    }
};

struct CopierInfo
{
    int segmentId;
    int invalidBlockCount;
    int copiedBlockCount;
    time_t beginTime;
    time_t endTime;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(segmentId),
            CEREAL_NVP(invalidBlockCount),
            CEREAL_NVP(copiedBlockCount), 
            CEREAL_NVP(beginTime),
            CEREAL_NVP(endTime));
    }
};

struct CopyDoneInfo
{
    int requestStripeCount;
    int requestBlockCount;
    int doneBlockCount;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(requestStripeCount),
            CEREAL_NVP(requestBlockCount),
            CEREAL_NVP(doneBlockCount));
    }
};

struct StripeCopySubmissionInfo
{
    StripeCopySubmission* stripeCopySubmission;
    BackendEvent event;
    time_t beginTime;
};

struct ReverseMapLoadCompletionInfo
{
    ReverseMapLoadCompletion* reverseMapLoadCompletion;
    BackendEvent event;
    time_t beginTime;
};

struct GcFlushSubmissionInfo
{
    int lsid;
    GcFlushSubmission* gcFlushSubmission;
    time_t beginTime;
};

struct GcMapUpdateRequestInfo
{
    GcMapUpdateRequest* gcMapUpdateRequest;
    time_t beginTime;
};

struct GcModeInfo
{
    GcMode mode;
    int initialFreeSegmentCount;
    int remainFreeSegmentCount;
    GcState gcState;
    CopierState copierState;
    bool gcRunning;

    CopyDoneInfo copyDoneInfo;

    time_t beginTime;
    time_t endTime;

    GcModeInfo& operator=(const GcModeInfo from)
    {
        this->beginTime = from.beginTime;
        this->copierState = from.copierState;
        this->copyDoneInfo = from.copyDoneInfo;
        this->endTime = from.endTime;
        this->gcRunning = from.gcRunning;
        this->gcState = from.gcState;
        this->initialFreeSegmentCount = from.initialFreeSegmentCount;
        this->mode = from.mode;
        this->remainFreeSegmentCount = from.remainFreeSegmentCount;

        return *this;
    }

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(mode),
            CEREAL_NVP(initialFreeSegmentCount),
            CEREAL_NVP(remainFreeSegmentCount),
            CEREAL_NVP(gcState),
            CEREAL_NVP(copierState),
            CEREAL_NVP(gcRunning),
            CEREAL_NVP(copyDoneInfo),
            CEREAL_NVP(beginTime),
            CEREAL_NVP(endTime));
    }
};

class GcDebugInfo
{
public:
    GcDebugInfo(void);
    virtual ~GcDebugInfo(void);

    virtual void ClearReverseMapLoadCompletionLog(int index);
    virtual void ClearStripeCopySubmissionLog(int baseStripeId);
    virtual void ClearGcFlushSubmission(int lsid);
    virtual void ClearGcMapUpdateRequest(int lsid);
    
    GcModeInfo GetPrevGcModeInfo(void) { return prevGcModeInfo; }
    GcModeInfo GetCurGcModeInfo(void) { return curGcModeInfo; }

    int GetNormalModeThreshold(void) { return normalModeThreshold; }
    int GetUrgentModeThreshold(void) { return urgentModeThreshold; }
    int GetCurrentGcModeRemainSegmentNum(void) { return curGcModeInfo.remainFreeSegmentCount; }
    CopierState GetPrevCopierState(void) { return prevGcModeInfo.copierState; }
    CopierState GetCurrentCopierState(void) { return curGcModeInfo.copierState; }

    GcMode GetCurrentGcMode(void) { return curGcModeInfo.mode; }
    GcMode GetPrevGcMode(void) { return prevGcModeInfo.mode; }

    GcFlushSubmissionInfo GetGcFlushSubmissionLog(int lsid);

    virtual void SetNormalModeThreshold(int threshold) { normalModeThreshold = threshold; }
    virtual void SetUrgentModeThreshold(int threshold) { urgentModeThreshold = threshold; }
    virtual void UpdateGcModeInfo(GcMode mode, int numFreeSegment);
    virtual void UpdateCopierState(CopierStateType state);
    virtual void UpdateAllocatedVictimInfo(int victimId, int validBlockCount, int arrayId, int numFreeSegments);
    virtual void UpdateStripeCopySubmissionLog(int baseStripeId, BackendEvent event, StripeCopySubmission* callback);
    virtual void UpdateReverseMapLoadCompletionInfo(int lsid, ReverseMapLoadCompletion* callback);
    virtual void UpdateGcFlushSubmission(int lsid, GcFlushSubmission* callback);
    virtual void UpdateGcMapUpdateRequest(int lsid, GcMapUpdateRequest* callback);
    virtual void UpdateAllocatedSegmentInfo(int segmentId);

    void Snapshot(void);

private:
    const uint32_t MAX_LOG_COUNT = 20;

    int normalModeThreshold;
    int urgentModeThreshold;
    
    GcModeInfo prevGcModeInfo;
    GcModeInfo curGcModeInfo;

    std::queue<AllocatedSegmentInfo> allocatedFreeSegmentHistory;
    std::queue<VictimSegmentInfo> victimSegmentHistory;
    std::queue<CopierInfo> copierHistory;

    std::map<int, StripeCopySubmissionInfo> stripeCopySubmissionLog;
    std::map<int, ReverseMapLoadCompletionInfo> reverseMapLoadCompletionLog;
    std::map<int, GcFlushSubmissionInfo> gcFlushSubmissionLog;
    std::map<int, GcMapUpdateRequestInfo> gcMapUpdateRequestLog;
};
} // namespace pos
#endif // GC_DEBUG_INFO_H_
