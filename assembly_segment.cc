#include "assembly_segment.h"

#include <iostream>
#include <cstdlib>
#include <limits>
#include <cstring>
#include <sstream>
#include <cassert>

namespace gnossen {
namespace assembly {

static constexpr uint32_t k8BitMax = std::numeric_limits<int8_t>::max();
static constexpr uint32_t k16BitMax = std::numeric_limits<int16_t>::max();
static constexpr uint32_t k32BitMax = std::numeric_limits<int32_t>::max();

size_t AssemblySubroutine::maximum_distance(unsigned int a, unsigned int b) const {
  if (a == b) {
    return 0;
  } else if (b < a) {
    return maximum_distance(b, a);
  } else {
    size_t offset = 0;
    for (unsigned int i = a; i < b; ++i) {
      offset += segments_[index_mapping_.at(i)]->max_size();
    }
    return offset;
  }
}

size_t AssemblySubroutine::absolute_offset(unsigned int segment_index) const {
  size_t offset = 0;
  for (unsigned int i = 0; i < segment_index; ++i) {
    offset += segments_[index_mapping_.at(i)]->size();
  }
  return offset;
}


void AssemblySubroutine::add_segment(std::unique_ptr<AssemblySegment> segment) {
  index_mapping_[segment->id()] = segments_.size();
  segments_.push_back(std::move(segment));
}

void AssemblySubroutine::finalize() {
  for (auto&& segment : segments_) {
    segment->determine_size(this);
  }
  for (auto&& segment : segments_) {
    segment->determine_offset(this);
  }
}

void AssemblySubroutine::write_code(uint8_t* buffer) const {
  uint8_t* code = buffer;
  for (auto&& segment : segments_) {
    segment->write_code(&code);
  }
}

size_t AssemblySubroutine::size() const {
  size_t sum = 0;
  for (auto&& segment : segments_) {
    sum += segment->size();
  }
  return sum;
}

std::string AssemblySubroutine::debug_string() const {
  std::stringstream ss;
  for (auto&& segment : segments_) {
    ss << segment->debug_string();
  }
  return ss.str();
}

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
    uint8_t** code) const noexcept {

  // NOTE: We rely on the compiling and target architecture being the same for
  // both endianness and for two's complement representation.
  if (offset_size_ == 8) {
    const int8_t offset = relative_offset_;
    memcpy(*code, kCodeRel8, sizeof(kCodeRel8));
    ((char*)*code)[1] = letter_;
    ((int8_t*)*code)[4] = offset;
    *code += sizeof(kCodeRel8);
  } else if (offset_size_ == 16 || offset_size_ == 32) {
    const int32_t offset = relative_offset_;
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

void ConsumingMatchNonConsumingNonMatch::determine_offset(
    const OffsetInterface* offset_if) noexcept
{
  const size_t this_segment_start = offset_if->absolute_offset(index_);
  const size_t other_segment_start = offset_if->absolute_offset(jmp_index_);
  if (offset_size_ == 8) {
    const size_t jmp_relative = this_segment_start + 5;
    relative_offset_ = jmp_relative - other_segment_start;
  } else if (offset_size_ == 16 || offset_size_ == 32) {
    const size_t jmp_relative = this_segment_start + 9;
    relative_offset_ = jmp_relative - other_segment_start;
  } else {
    std::cerr << "determine_offset() called before determine_size()." << std::endl;
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
    std::cerr << "debug_string() called before determine_offset()." << std::endl;
    exit(1);
  }

  std::stringstream ss;
  ss <<
  ".section_" << index_ << ":" << std::endl <<
  "    mov $0x" << std::hex << (unsigned int)letter_ <<
      std::dec << ", %al  ; '" << letter_ << "'" <<  std::endl <<
  "    scasb" << std::endl <<
  "    je .section_" << jmp_index_ <<  "  ; Offset " <<
      std::hex << relative_offset_  << std::dec << std::endl <<
  "    dec %rdi" << std::endl;
  return ss.str();
}

void StaticCodeSegment::write_code(uint8_t** code) const noexcept{
  memcpy(*code, code_, code_size_);
}

size_t StaticCodeSegment::size() const noexcept{
  return code_size_;
}

size_t StaticCodeSegment::max_size() const noexcept{
  return code_size_;
}

const uint8_t StackManagementSegment::kCode[] = {
  0x55,             // push %rbp
  0x48, 0x89, 0xe5  // mov %rsp, %rbp
};

StackManagementSegment::StackManagementSegment(unsigned int id) :
  StaticCodeSegment(id, kCode, sizeof(kCode)) {}

std::string StackManagementSegment::debug_string() const {
  std::stringstream ss;
  ss <<
  "    push %rbp" << std::endl <<
  "    mov %rsp, %rbp" << std::endl;
  return ss.str();
}

// TODO: Maybe xor %rax,%rax followed by inc would be faster?
const uint8_t SuccessSegment::kCode[] = {
  0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00,   // mov $01, %rax
  0x5d,                                       // pop %rbp
  0xc3                                        // retq
};

SuccessSegment::SuccessSegment(unsigned int id) :
  StaticCodeSegment(id, kCode, sizeof(kCode)) {}

std::string SuccessSegment::debug_string() const {
  std::stringstream ss;
  ss <<
  ".section_" << id() << ":  ; success" << std::endl <<
  "    mov $01, %rax" << std::endl <<
  "    pop %rbp" << std::endl <<
  "    retq" << std::endl;
  return ss.str();
}

const uint8_t FailureSegment::kCode[] = {
  0x31, 0xc0,   // xor %eax, %eax
  0x5d,         // pop %rbp
  0xc3          // retq
};

FailureSegment::FailureSegment(unsigned int id) :
  StaticCodeSegment(id, kCode, sizeof(kCode)) {}

std::string FailureSegment::debug_string() const {
  std::stringstream ss;
  ss <<
  ".section_" << id() << ":  ; failure" << std::endl <<
  "    xor %eax, %eax" << std::endl <<
  "    pop %rbp" << std::endl <<
  "    retq" << std::endl;
  return ss.str();
}

} // end namespace assembly
} // end namespace gnossen
