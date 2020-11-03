#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "assembly_segment.h"

#include <memory>

#include <iostream>

namespace gnossen {
namespace assembly {
namespace {

TEST(AssemblySegmentTest, CanBuild) {
  AssemblySubroutine subroutine;
  unsigned int id = 0;
  subroutine.add_segment(std::move(std::make_unique<StackManagementSegment>(id++)));
  subroutine.add_segment(std::move(std::make_unique<NoOp>(id++)));
  subroutine.add_segment(std::move(
        std::make_unique<ConsumingMatchNonConsumingNonMatch>(id++, 'a', 1)));
  subroutine.add_segment(std::move(std::make_unique<SuccessSegment>(id++)));
  subroutine.add_segment(std::move(std::make_unique<FailureSegment>(id++)));
  subroutine.finalize();

  std::cerr << subroutine.debug_string();
}

}
} // end namespace assembly
} // end namespace gnossen
