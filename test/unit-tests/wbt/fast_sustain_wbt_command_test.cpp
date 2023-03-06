#include "src/wbt/fast_sustain_wbt_command.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(FastSustainWbtCommand, Execute)
{
    // MockWBT
    // JsonElement retElem("json");

    // FastSustainWbtCommand fastSustainWbtCmd;
    // fastSustainWbtCmd.Execute(request, retElem);

    FastSustainWbtCommand fastSustainWbtCmd;
    fastSustainWbtCmd.InternalExecute();
}

} // namespace pos
