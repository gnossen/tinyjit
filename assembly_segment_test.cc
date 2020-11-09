#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "assembly_segment.h"

#include <memory>

#include <iostream>
#include <fstream>

namespace gnossen {
namespace assembly {
namespace {

static std::string artifact_path(const std::string& filename) {
  std::string extra_artifacts_dir(getenv("TEST_UNDECLARED_OUTPUTS_DIR"));
  return extra_artifacts_dir + "/" + filename;
}

TEST(AssemblySegmentTest, CanBuild) {

  // Matches "a*b".
  AssemblySubroutine subroutine;
  subroutine.add_segment(std::move(std::make_unique<StackManagementSegment>(0)));
  subroutine.add_segment(std::move(std::make_unique<NoOp>(1)));
  subroutine.add_segment(std::move(
        std::make_unique<ConsumingMatchNonConsumingNonMatch>(2, 'a', 2)));
  subroutine.add_segment(std::move(
        std::make_unique<ConsumingMatchElse>(3, 'b', 5)));
  subroutine.add_segment(std::move(std::make_unique<SuccessSegment>(4)));
  subroutine.add_segment(std::move(std::make_unique<FailureSegment>(5)));
  subroutine.finalize();

  std::cerr << subroutine.debug_string();
  std::ofstream asm_file(artifact_path("regex1.S"), std::ofstream::out);
  asm_file << subroutine.debug_string();

  uint8_t* code = new uint8_t[subroutine.size()];
  subroutine.write_code(code);

  std::string binary_path = artifact_path("regex1.bin");
  std::ofstream binfile(binary_path, std::ofstream::binary);
  binfile.write((char*)code, subroutine.size());
  delete [] code;
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
