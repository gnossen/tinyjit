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

// TODO: Add test for `write_code`.


// I don't know that I actually want to write a test for just the
// assembly_segment file. Since it's so coupled to everything else, it doesn't
// feel particularly useful. This test exists as an aid for me to ensure that
// I'm writing something that functions before getting to the end stage. I'll
// probably delete this after I've written my end-to-end level test.

}
} // end namespace assembly
} // end namespace gnossen
