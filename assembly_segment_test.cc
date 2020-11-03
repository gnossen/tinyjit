#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "assembly_segment.h"

#include <memory>

namespace gnossen {
namespace assembly {
namespace {

TEST(AssemblySegmentTest, CanBuild) {
  AssemblySubroutine subroutine;
  subroutine.add_segment(std::move(std::make_unique<NoOp>(0)));
  subroutine.finalize();
}

}
} // end namespace assembly
} // end namespace gnossen
