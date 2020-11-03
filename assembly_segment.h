#include <cstdint>
#include <string>

namespace gnossen {
namespace assembly {

class OffsetInterface {
public:

  // Returns the maximum offset distance between two code segments. It is valid
  // to call this method regardless of the finalization state of either index.
  virtual size_t maximum_distance(unsigned int a, unsigned int b) const = 0;

  // Returns the absolute offset of the segment with index `segment_index`.
  virtual size_t absolute_offset(unsigned int segment_index) const = 0;
};

/* There is a lifecycle associated with an assembly segment. It starts off
 * with both indeterminate size and contents. After the, determine_size method
 * has been called, the segment's size is known, but its contents may still be
 * indeterminate.
 *
 * After all segments have had their sizes determined, their absolute offset is
 * calculable, and, therefore, so are their relative offsets. At this point,
 * `write_code` may be called.
 */

class AssemblySegment {
public:

  virtual void write_code(uint8_t** code, const OffsetInterface* offset_if) = 0;
  virtual std::string debug_string() const = 0;

  virtual void determine_size(const OffsetInterface* offset_if) = 0;

  // Only valid to call this after write_code has been called.
  virtual size_t size() const = 0;

  // The maximum size that this segment will take on. It is valid to call this
  // method before write_code.
  virtual size_t max_size() const = 0;
};

// You could argue that this is a hack and I should eliminate it in a
// prior step.
class NoOp : public AssemblySegment {
  NoOp() = default;

public:
  void write_code(uint8_t** code, const OffsetInterface* offset_if) override {
    // Do nothing. 
  }

  void determine_size(const OffsetInterface* offset_if) override {
    // Do nothing.
  }

  std::string debug_string() const override {
    return "    ; No-op section\n";
  }

  size_t size() const override { return 0; }

  size_t max_size() const override {
    return 0;
  }

private:

  static const uint8_t kCode[];
};

// Consumes a single character if there's a match and jumps to the specified
// section. Otherwise, does not consume a character and continues to the next
// section.
class ConsumingMatchNonConsumingNonMatch : public AssemblySegment {
public:

  ConsumingMatchNonConsumingNonMatch(unsigned int index,
                                     char letter, unsigned int jmp_index) noexcept :
    index_(index),
    letter_(letter),
    jmp_index_(jmp_index),
    offset_size_(0),
    relative_offset_(0) {}

  void write_code(uint8_t** code, const OffsetInterface* offset_if) noexcept override;

  void determine_size(const OffsetInterface* offset_if) noexcept override;

  std::string debug_string() const override;
  size_t size() const noexcept override;

  size_t max_size() const noexcept override;

private:
  static const uint8_t kCodeRel8[];

  static const uint8_t kCodeRel16Or32[];

  unsigned int index_;
  char letter_;
  unsigned int jmp_index_;
  size_t offset_size_;
  int32_t relative_offset_;
};

// Consumes a single character. If there's a match, continue to the next
// section, otherwise jump to the given section.
class ConsumingMatchElse : public AssemblySegment {
public:
  void write_code(uint8_t** code, const OffsetInterface* offset_if) override;

  void determine_size(const OffsetInterface* offset_if) override {
    // TODO: Implement.
  }

  std::string debug_string() const override;
  size_t size() const override;
  size_t max_size() const noexcept override;

private:

  // static const uint8_t kCode[] = {
  //   0xb0, 0x00,   // mov LETTER, %al
  //   0xae,         // scas
  //   0x75, 0x00,   // jne .section_X
  //   // 0x0f, 0x85, 0x00, 0x00   // jne rel16
  // };
};

} // end namespace assembly
} // end namespace gnossen
