#include "assembly_segment.h"

#include <iostream>
#include <cstdlib>
#include <limits>
#include <cstring>
#include <sstream>

namespace gnossen {
namespace assembly {

static constexpr uint32_t k8BitMax = std::numeric_limits<int8_t>::max();
static constexpr uint32_t k16BitMax = std::numeric_limits<int16_t>::max();
static constexpr uint32_t k32BitMax = std::numeric_limits<int32_t>::max();

const uint8_t NoOp::kCode[] = {};

const uint8_t ConsumingMatchNonConsumingNonMatch::kCodeRel8[] = {
  0xb0, 0x00,       // mov LETTER, %al
  0xae,             // scas
  0x74, 0x00,       // je .section_X
  0x48, 0xff, 0xcf  // dec %rdi
};

const uint8_t ConsumingMatchNonConsumingNonMatch::kCodeRel16Or32[] = {
  0xb0, 0x00,                         // mov LETTER, %al
  0xae,                               // scas
  0x0f, 0x84, 0x00, 0x00, 0x00, 0x00, // je .section_X ; rel16 or rel32
  0x48, 0xff, 0xcf                    // dec %rdi
};

void ConsumingMatchNonConsumingNonMatch::write_code(
    uint8_t** code, const OffsetInterface* offset_if) noexcept {

  // NOTE: We rely on the compiling and target architecture being the same for
  // both endianness and for two's complement representation.
  const size_t this_segment_start = offset_if->absolute_offset(index_);
  const size_t other_segment_start = offset_if->absolute_offset(jmp_index_);
  if (offset_size_ == 8) {
    const size_t jmp_relative = this_segment_start + 5;
    const int8_t offset = jmp_relative - other_segment_start;
    relative_offset_ = offset;
    memcpy(*code, kCodeRel8, sizeof(kCodeRel8));
    ((char*)*code)[1] = letter_;
    ((int8_t*)*code)[4] = offset;
    *code += sizeof(kCodeRel8);
  } else if (offset_size_ == 16 || offset_size_ == 32) {
    const size_t jmp_relative = this_segment_start + 9;
    const int32_t offset = jmp_relative - other_segment_start;
    relative_offset_ = offset;
    memcpy(*code, kCodeRel16Or32, sizeof(kCodeRel16Or32));
    ((char*)*code)[1] = letter_;
    ((int32_t*)*code)[5] = offset;
    *code += sizeof(kCodeRel16Or32);
  } else {
    std::cerr << "write_code() called before determine_size()." << std::endl;
    exit(1);
  }
}

void ConsumingMatchNonConsumingNonMatch::determine_size(
    const OffsetInterface* offset_if) noexcept
{
  const size_t max_inter_segment_distance = offset_if->maximum_distance(index_, jmp_index_);

  // Offset is relative to the end of the jump statement.
  const size_t max_intra_segment_distance = 9;
  const size_t max_distance = max_inter_segment_distance + max_intra_segment_distance;
  if (max_distance < k8BitMax) {
    offset_size_ = 8;
  } else if (max_distance < k16BitMax) {
    offset_size_ = 16;
  } else if (max_distance < k32BitMax) {
    offset_size_ = 32;
  } else {
    std::cerr << "Encountered offset " << max_distance << "not representable with 32 bits." << std::endl;
    exit(1);
  }
}

size_t ConsumingMatchNonConsumingNonMatch::max_size() const noexcept {
  // TODO: There are fancy things we can do to make this smaller under certain
  // circumstances.
  return sizeof(kCodeRel16Or32);
}

size_t ConsumingMatchNonConsumingNonMatch::size() const noexcept {
  if (offset_size_ == 8) {
    return sizeof(kCodeRel8);
  } else if (offset_size_ == 16 || offset_size_ == 32) {
    return sizeof(kCodeRel16Or32);
  } else {
    std::cerr << "size() called before determine_size()." << std::endl;
    exit(1);
  }
}

std::string ConsumingMatchNonConsumingNonMatch::debug_string() const {
  if  (offset_size_ == 0 || relative_offset_ == 0) {
    std::cerr << "debug_string() called before write_code()." << std::endl;
    exit(1);
  }

  std::stringstream ss;
  ss <<
  ".section_" << index_ << ":" << std::endl <<
  "    mov 0x" << std::hex << (uint8_t)letter_ <<
      std::dec << "  ; " << letter_ <<  std::endl <<
  "    scasb" << std::endl <<
  "    je .section_" << jmp_index_ <<  "  ; Offset " <<
      std::hex << relative_offset_  << std::dec << std::endl <<
  "    dec %rdi" << std::endl;
  return ss.str();
}

} // end namespace assembly
} // end namespace gnossen
