#include "src/debug/debug_info.h"
#include "src/qos/qos_spdk_manager.h"
#include <gtest/gtest.h>
#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "src/debug/gc_debug_info.h"
#include "test/unit-tests/debug/gc_debug_info_mock.h"

using namespace ::testing;
using testing::NiceMock;

namespace pos
{
TEST(DebugInfo, DebugInfo)
{
    // Given : new debug Info object is constructed.
    DebugInfo* localDebugInfo = new DebugInfo;
    // When : Nothing
    // Then : delete.
    delete localDebugInfo;
    debugInfo = nullptr;
}

TEST(DebugInfo, UpdateGcDebugInfo_CheckGcModeThresholdUpdate)
{
    GcDebugInfo* gcDebugInfo = new GcDebugInfo();
    debugInfo = new DebugInfo(gcDebugInfo);

    int normalModeThreshold = 20;
    int urgentModeThreshold = 5;

    GcCtx* gcCtx = new GcCtx();

    gcCtx->SetNormalGcThreshold(normalModeThreshold);
    gcCtx->SetUrgentThreshold(urgentModeThreshold);

    int updatedNormalModeThreshold = gcCtx->GetNormalGcThreshold();
    int updatedUrgentModeThreshold = gcCtx->GetUrgentThreshold();

    EXPECT_EQ(normalModeThreshold, updatedNormalModeThreshold);
    EXPECT_EQ(urgentModeThreshold, updatedUrgentModeThreshold);

    int normalModeThresholdInDebugInfo = gcDebugInfo->GetNormalModeThreshold();
    int urgentModeThresholdInDebugInfo = gcDebugInfo->GetUrgentModeThreshold();

    EXPECT_EQ(normalModeThreshold, normalModeThresholdInDebugInfo);
    EXPECT_EQ(urgentModeThreshold, urgentModeThresholdInDebugInfo);

    delete gcCtx;
    delete gcDebugInfo;
    delete debugInfo;
    debugInfo = nullptr;
}

TEST(DebugInfo, UpdateGcDebugInfo_CheckGcModeUpdate)
{
    GcDebugInfo* gcDebugInfo = new GcDebugInfo();
    debugInfo = new DebugInfo(gcDebugInfo);

    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    int arrayId = 0;
    GcCtx* gcCtx = new GcCtx(blockAllocStatus, arrayId);

    int numFreeSegment = 19;
    gcCtx->UpdateCurrentGcMode(numFreeSegment);

    GcMode curGcMode = gcCtx->GetCurrentGcMode();
    GcMode curGcModeInDebugInfo = gcDebugInfo->GetCurrentGcMode();
    EXPECT_EQ(curGcMode, curGcModeInDebugInfo);

    numFreeSegment = 4;
    gcCtx->UpdateCurrentGcMode(numFreeSegment);

    curGcMode = gcCtx->GetCurrentGcMode();
    curGcModeInDebugInfo = gcDebugInfo->GetCurrentGcMode();
    EXPECT_EQ(curGcMode, curGcModeInDebugInfo);

    numFreeSegment = 21;
    gcCtx->UpdateCurrentGcMode(numFreeSegment);

    curGcMode = gcCtx->GetCurrentGcMode();
    curGcModeInDebugInfo = gcDebugInfo->GetCurrentGcMode();
    EXPECT_EQ(curGcMode, curGcModeInDebugInfo);

    delete gcCtx;
    delete gcDebugInfo;
    delete debugInfo;
    debugInfo = nullptr;
}

TEST(DebugInfo, UpdateGcDebugInfo_CheckGcModeInfoUpdate)
{
    GcDebugInfo* gcDebugInfo = new GcDebugInfo();
    debugInfo = new DebugInfo(gcDebugInfo);

    NiceMock<MockBlockAllocationStatus>* blockAllocStatus = new NiceMock<MockBlockAllocationStatus>();
    int arrayId = 0;
    GcCtx* gcCtx = new GcCtx(blockAllocStatus, arrayId);

    int normalModeThreshold = 20;
    int urgentModeThreshold = 5;

    gcCtx->SetNormalGcThreshold(normalModeThreshold);
    gcCtx->SetUrgentThreshold(urgentModeThreshold);
    
    //TODO: make test fixture. 
    int numFreeSegment = 30;
    gcCtx->UpdateCurrentGcMode(numFreeSegment);

    GcMode prevGcModeInDebugInfo = gcDebugInfo->GetPrevGcMode();
    GcMode curGcModeInDebugInfo = gcDebugInfo->GetCurrentGcMode();

    GcMode expectedCurGcMode = MODE_NO_GC;
    GcMode expectedPrevGcMode = curGcModeInDebugInfo;
    
    EXPECT_EQ(expectedPrevGcMode, prevGcModeInDebugInfo);
    EXPECT_EQ(expectedCurGcMode, curGcModeInDebugInfo);

    numFreeSegment = 19;
    gcCtx->UpdateCurrentGcMode(numFreeSegment);
    expectedPrevGcMode = curGcModeInDebugInfo;
    expectedCurGcMode = MODE_NORMAL_GC;

    prevGcModeInDebugInfo = gcDebugInfo->GetPrevGcMode();
    curGcModeInDebugInfo = gcDebugInfo->GetCurrentGcMode();

    EXPECT_EQ(expectedPrevGcMode, prevGcModeInDebugInfo);
    EXPECT_EQ(expectedCurGcMode, curGcModeInDebugInfo);

    numFreeSegment = 3;
    gcCtx->UpdateCurrentGcMode(numFreeSegment);
    expectedPrevGcMode = curGcModeInDebugInfo;
    expectedCurGcMode = MODE_URGENT_GC;

    prevGcModeInDebugInfo = gcDebugInfo->GetPrevGcMode();
    curGcModeInDebugInfo = gcDebugInfo->GetCurrentGcMode();

    EXPECT_EQ(expectedPrevGcMode, prevGcModeInDebugInfo);
    EXPECT_EQ(expectedCurGcMode, curGcModeInDebugInfo);

    numFreeSegment = 21;
    gcCtx->UpdateCurrentGcMode(numFreeSegment);
    expectedPrevGcMode = curGcModeInDebugInfo;
    expectedCurGcMode = MODE_NO_GC;

    prevGcModeInDebugInfo = gcDebugInfo->GetPrevGcMode();
    curGcModeInDebugInfo = gcDebugInfo->GetCurrentGcMode();

    EXPECT_EQ(expectedPrevGcMode, prevGcModeInDebugInfo);
    EXPECT_EQ(expectedCurGcMode, curGcModeInDebugInfo);

    delete blockAllocStatus;
    delete gcCtx;
    delete gcDebugInfo;
    delete debugInfo;
    debugInfo = nullptr;
}
} // namespace pos
