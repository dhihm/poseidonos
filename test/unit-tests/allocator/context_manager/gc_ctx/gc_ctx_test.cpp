#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"

#include <gtest/gtest.h>
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "src/debug/debug_info.h"
#include "test/unit-tests/debug/gc_debug_info_mock.h"

using testing::NiceMock;

namespace pos
{
TEST(GcCtx, GetCurrentGcMode_TestModeNoGC)
{
    // given
    NiceMock<MockBlockAllocationStatus> allocStatus;
    GcCtx gcCtx(&allocStatus, 0);

    gcCtx.SetNormalGcThreshold(10);
    gcCtx.SetUrgentThreshold(5);

    // when 1
    uint32_t numFreeSegments = 8;
    GcMode gcMode = gcCtx.UpdateCurrentGcMode(numFreeSegments);

    // then 2
    EXPECT_EQ(gcMode, GcMode::MODE_NORMAL_GC);
    EXPECT_EQ(gcMode, gcCtx.GetCurrentGcMode());
}

TEST(GcCtx, UpdateCurrentGcModeAndDebugInfo_ByNumberOfFreeSegment)
{
    if (nullptr != debugInfo)
    {
        delete debugInfo;
    }

    GcDebugInfo* gcDebugInfo = new GcDebugInfo();
    debugInfo = new DebugInfo(gcDebugInfo);

    NiceMock<MockBlockAllocationStatus> blockAllocStatus;
    GcCtx* gcCtx = new GcCtx(&blockAllocStatus, 0);

    gcCtx->SetNormalGcThreshold(10);
    gcCtx->SetUrgentThreshold(5);

    // when 1.
    uint32_t numFreeSegments = 11;
    GcMode ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);

    GcModeInfo prevModeInfo = debugInfo->GetGcDebugInfo()->GetPrevGcModeInfo();
    GcModeInfo curModeInfo = debugInfo->GetGcDebugInfo()->GetCurGcModeInfo();

    // then 1.
    EXPECT_EQ(MODE_NO_GC, ret);
    EXPECT_EQ(MODE_NO_GC, prevModeInfo.mode);
    EXPECT_EQ(MODE_NO_GC, curModeInfo.mode);

    // when 2.
    numFreeSegments = 9;
    ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);

    prevModeInfo = debugInfo->GetGcDebugInfo()->GetPrevGcModeInfo();
    curModeInfo = debugInfo->GetGcDebugInfo()->GetCurGcModeInfo();

    // then 2.
    EXPECT_EQ(MODE_NORMAL_GC, ret);
    EXPECT_EQ(MODE_NO_GC, prevModeInfo.mode);
    EXPECT_EQ(MODE_NORMAL_GC, curModeInfo.mode);

    // when 3.
    numFreeSegments = 3;
    ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);

    prevModeInfo = debugInfo->GetGcDebugInfo()->GetPrevGcModeInfo();
    curModeInfo = debugInfo->GetGcDebugInfo()->GetCurGcModeInfo();

    // then 3.
    EXPECT_EQ(MODE_URGENT_GC, ret);
    EXPECT_EQ(MODE_NORMAL_GC, prevModeInfo.mode);
    EXPECT_EQ(MODE_URGENT_GC, curModeInfo.mode);

    // when 4.
    numFreeSegments = 8;
    ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);

    prevModeInfo = debugInfo->GetGcDebugInfo()->GetPrevGcModeInfo();
    curModeInfo = debugInfo->GetGcDebugInfo()->GetCurGcModeInfo();

    // then 4.
    EXPECT_EQ(MODE_NORMAL_GC, ret);
    EXPECT_EQ(MODE_URGENT_GC, prevModeInfo.mode);
    EXPECT_EQ(MODE_NORMAL_GC, curModeInfo.mode);

    // when 5.
    numFreeSegments = 11;
    ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);

    prevModeInfo = debugInfo->GetGcDebugInfo()->GetPrevGcModeInfo();
    curModeInfo = debugInfo->GetGcDebugInfo()->GetCurGcModeInfo();

    // then 5.
    EXPECT_EQ(MODE_NO_GC, ret);
    EXPECT_EQ(MODE_NORMAL_GC, prevModeInfo.mode);
    EXPECT_EQ(MODE_NO_GC, curModeInfo.mode);

    delete gcDebugInfo;
    delete debugInfo;
    debugInfo = nullptr;

    delete gcCtx;
}
} // namespace pos
