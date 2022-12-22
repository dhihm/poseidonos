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

#include "src/debug/gc_debug_info.h"
#include "src/gc/gc_flush_submission.h"
#include "src/gc/stripe_copy_submission.h"
#include "src/gc/reverse_map_load_completion.h"
#include "src/gc/gc_map_update_request.h"

namespace pos
{
GcDebugInfo::GcDebugInfo(void)
: normalModeThreshold(0),
  urgentModeThreshold(0)
{
    prevGcModeInfo.mode = MODE_NO_GC;
    prevGcModeInfo.gcRunning = false;
    prevGcModeInfo.gcState = GC_STATE_NO_GC;
    prevGcModeInfo.beginTime = 0;
    prevGcModeInfo.endTime = 0;
    prevGcModeInfo.copierState.beginTime = 0;
    prevGcModeInfo.copierState.endTime = 0;
    prevGcModeInfo.copierState.state = COPIER_READY_TO_END_STATE;
    prevGcModeInfo.initialFreeSegmentCount = UINT32_MAX;
    prevGcModeInfo.remainFreeSegmentCount = UINT32_MAX;

    prevGcModeInfo.copyDoneInfo.doneBlockCount = 0;
    prevGcModeInfo.copyDoneInfo.requestBlockCount = 0;
    prevGcModeInfo.copyDoneInfo.requestStripeCount = 0;

    curGcModeInfo.mode = MODE_NO_GC;
    curGcModeInfo.gcRunning = false;
    curGcModeInfo.gcState = GC_STATE_NO_GC;
    curGcModeInfo.beginTime = 0;
    curGcModeInfo.endTime = 0;
    curGcModeInfo.copierState.beginTime = 0;
    curGcModeInfo.copierState.endTime = 0;
    curGcModeInfo.copierState.state = COPIER_READY_TO_END_STATE;
    curGcModeInfo.initialFreeSegmentCount = UINT32_MAX;
    curGcModeInfo.remainFreeSegmentCount = UINT32_MAX;

    curGcModeInfo.copyDoneInfo.doneBlockCount = 0;
    curGcModeInfo.copyDoneInfo.requestBlockCount = 0;
    curGcModeInfo.copyDoneInfo.requestStripeCount = 0;
}

GcDebugInfo::~GcDebugInfo(void)
{

}

void
GcDebugInfo::UpdateGcModeInfo(GcMode mode, int numFreeSegment)
{
    if (curGcModeInfo.mode != mode)
    {
        time(&prevGcModeInfo.endTime);
        prevGcModeInfo.mode = curGcModeInfo.mode;
        prevGcModeInfo.initialFreeSegmentCount = curGcModeInfo.initialFreeSegmentCount;
        prevGcModeInfo.remainFreeSegmentCount = numFreeSegment;
        prevGcModeInfo.beginTime = curGcModeInfo.beginTime;

        curGcModeInfo.mode = mode;
        time(&curGcModeInfo.beginTime);
        curGcModeInfo.endTime = 0;
        curGcModeInfo.initialFreeSegmentCount = numFreeSegment;
    }
}

void
GcDebugInfo::UpdateCopierState(CopierStateType state)
{
    if (curGcModeInfo.copierState.state != state)
    {
        prevGcModeInfo.copierState.state = curGcModeInfo.copierState.state;
        time(&prevGcModeInfo.copierState.endTime);
        prevGcModeInfo.copierState.beginTime = curGcModeInfo.copierState.beginTime;

        curGcModeInfo.copierState.state = state;
        time(&curGcModeInfo.copierState.beginTime);
    }
}

void
GcDebugInfo::UpdateAllocatedVictimInfo(int victimId, int validBlockCount, int arrayId, int numFreeSegments)
{
    VictimSegmentInfo info;
    info.segmentId = victimId;
    info.validBlockCount = validBlockCount;
    info.arrayId = arrayId;
    info.numFreeSegments = numFreeSegments;
    time(&info.selectedTime);

    if (MAX_LOG_COUNT < victimSegmentHistory.size())
    {
        victimSegmentHistory.pop();
    }

    victimSegmentHistory.push(info);
}

void
GcDebugInfo::UpdateFreedSegmentInfo(int victimId)
{
    FreedSegmentInfo info;
    info.segmentId = victimId;
    time(&info.freedTime);

    if (MAX_LOG_COUNT < freedSegmentHistory.size())
    {
        freedSegmentHistory.pop();
    }

    freedSegmentHistory.push(info);
}

void 
GcDebugInfo::UpdateStripeCopySubmissionLog(int baseStripeId, BackendEvent event, StripeCopySubmission* callback)
{
    StripeCopySubmissionInfo info;
    info.event = event;
    info.stripeCopySubmission = callback;
    time(&info.beginTime);

    stripeCopySubmissionLog[baseStripeId] = info;
}

void
GcDebugInfo::ClearStripeCopySubmissionLog(int baseStripeId)
{
    stripeCopySubmissionLog.erase(baseStripeId);
}

void
GcDebugInfo::UpdateReverseMapLoadCompletionInfo(int lsid, ReverseMapLoadCompletion* callback)
{
    ReverseMapLoadCompletionInfo info;
    info.event = BackendEvent_GC;
    info.reverseMapLoadCompletion = callback;
    time(&info.beginTime);

    reverseMapLoadCompletionLog[lsid] = info;
}

void
GcDebugInfo::ClearReverseMapLoadCompletionLog(int lsid)
{
    reverseMapLoadCompletionLog.erase(lsid);
}

void
GcDebugInfo::UpdateGcFlushSubmission(int lsid, GcFlushSubmission* callback)
{
    GcFlushSubmissionInfo info;
    info.lsid = lsid;
    info.gcFlushSubmission = callback;
    time(&info.beginTime);

    gcFlushSubmissionLog[lsid] = info;
}

void
GcDebugInfo::ClearGcFlushSubmission(int lsid)
{
    gcFlushSubmissionLog.erase(lsid);
}

void
GcDebugInfo::UpdateGcMapUpdateRequest(int lsid, GcMapUpdateRequest* callback)
{
    GcMapUpdateRequestInfo info;
    info.gcMapUpdateRequest = callback;
    time(&info.beginTime);

    gcMapUpdateRequestLog[lsid] = info;
}

void 
GcDebugInfo::ClearGcMapUpdateRequest(int lsid)
{
    gcMapUpdateRequestLog.erase(lsid);
}

GcFlushSubmissionInfo
GcDebugInfo::GetGcFlushSubmissionInfo(int lsid)
{
    return gcFlushSubmissionLog[lsid];
}

} // namespace pos
