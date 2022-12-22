#include <gmock/gmock.h>
#include "src/debug/gc_debug_info.h"

namespace pos
{
class MockGcDebugInfo : public GcDebugInfo
{
public:
    using GcDebugInfo::GcDebugInfo;
    MOCK_METHOD(void, ClearGcFlushSubmission, (int lsid), (override));
    MOCK_METHOD(void, ClearReverseMapLoadCompletionLog, (int index), (override));
    MOCK_METHOD(void, ClearStripeCopySubmissionLog, (int baseStripeId), (override));
    MOCK_METHOD(void, ClearGcMapUpdateRequest, (int lsid), (override));

    MOCK_METHOD(void, SetNormalModeThreshold, (int threshold), (override));
    MOCK_METHOD(void, SetUrgentModeThreshold, (int threshold), (override));
    MOCK_METHOD(void, UpdateGcModeInfo, (GcMode mode, int numFreeSegment), (override));
    MOCK_METHOD(void, UpdateCopierState, (CopierStateType state), (override));
    MOCK_METHOD(void, UpdateAllocatedVictimInfo,
        (int victimId, int validBlockCount, int arrayId, int numFreeSegments), (override));
    MOCK_METHOD(void, UpdateStripeCopySubmissionLog,
        (int baseStripeId, BackendEvent event, StripeCopySubmission* callback), (override));
    MOCK_METHOD(void, UpdateReverseMapLoadCompletionInfo, (int lsid, ReverseMapLoadCompletion* callback), (override));
    MOCK_METHOD(void, UpdateGcFlushSubmission, (int lsid, GcFlushSubmission* callback), (override));
    MOCK_METHOD(void, UpdateGcMapUpdateRequest, (int lsid, GcMapUpdateRequest* callback), (override));
    MOCK_METHOD(void, UpdateAllocatedSegmentInfo, (int victimId), (override));
};

} // namespace pos
