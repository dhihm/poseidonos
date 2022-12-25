#include "src/gc/reverse_map_load_completion.h"

#include <gtest/gtest.h>
#include "src/debug/debug_info.h"
#include "test/unit-tests/debug/gc_debug_info_mock.h"

using namespace ::testing;
using testing::NiceMock;

namespace pos
{
TEST(ReverseMapLoadCompletion, Execute_Invoke)
{
    NiceMock<MockGcDebugInfo>* gcDebugInfo = new NiceMock<MockGcDebugInfo>();
    debugInfo = new DebugInfo(gcDebugInfo);

    EXPECT_CALL(*gcDebugInfo, ClearReverseMapLoadCompletionLog(_)).Times(1);

    ReverseMapLoadCompletion* revMapLoadCompletion = new ReverseMapLoadCompletion();
    EXPECT_TRUE(revMapLoadCompletion->Execute() == true); // trival no op in revMapLoadCompletion
    delete revMapLoadCompletion;
    
    delete gcDebugInfo;
    gcDebugInfo = nullptr;

    delete debugInfo;
    debugInfo = nullptr;
}

} // namespace pos
