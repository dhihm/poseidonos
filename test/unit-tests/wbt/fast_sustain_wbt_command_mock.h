#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/fast_sustain_wbt_command.h"

namespace pos
{
class MockFastSustainWbtCommand : public FastSustainWbtCommand
{
public:
    using FastSustainWbtCommand::FastSustainWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
