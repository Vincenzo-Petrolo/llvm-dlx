#include "Target.h"
#include "InputSection.h"
#include "OutputSections.h"
#include "Symbols.h"
#include "SyntheticSections.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;
using namespace llvm::support::endian;
using namespace llvm::ELF;

namespace lld {
namespace elf {

// Minimal DLX target setup for handling DLX relocations.
class DLXTargetInfo : public TargetInfo {
public:
  DLXTargetInfo();
  RelType getDynRel(RelType type) const override;
  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;
  void relocateOne(uint8_t *loc, RelType type, uint64_t val) const override;
};

// Initialize DLX target settings
DLXTargetInfo::DLXTargetInfo() {
  copyRel = R_MIPS_COPY;
  noneRel = R_MIPS_NONE;
  pltRel = R_MIPS_JUMP_SLOT;
  relativeRel = R_MIPS_REL32;
  symbolicRel = R_MIPS_32;
  tlsModuleIndexRel = R_MIPS_TLS_DTPMOD32;
  tlsOffsetRel = R_MIPS_TLS_DTPREL32;
  tlsGotRel = R_MIPS_TLS_TPREL32;
  gotRel = symbolicRel;
}

// Minimal implementation for getDynRel.
RelType DLXTargetInfo::getDynRel(RelType type) const {
  switch (type) {
    case R_MIPS_32:
    case R_MIPS_64:
    case R_MIPS_LO16:
    case R_MIPS_HI16:
    case R_MIPS_PC16:
    case R_MIPS_PC26_S2:
      return type;
    default:
      return R_MIPS_NONE; // Return a safe default
  }
}

// Basic expression handling for DLX relocations.
RelExpr DLXTargetInfo::getRelExpr(RelType type, const Symbol &s,
                                  const uint8_t *loc) const {
  switch (type) {
    case R_MIPS_PC26_S2:
    case R_MIPS_PC16:
      return R_PC; // PC-relative relocations
    case R_MIPS_LO16:
    case R_MIPS_HI16:
      return R_ABS; // Absolute relocations
    case R_MIPS_32:
    case R_MIPS_64:
      return R_ABS; // Absolute data relocations
    default:
      return R_NONE; // Return None for unsupported or unknown types
  }
}

// Handle DLX relocations similarly to MIPS.
void DLXTargetInfo::relocateOne(uint8_t *loc, RelType type, uint64_t val) const {
  switch (type) {
    case R_MIPS_32:
      write32le(loc, val);
      return;
    case R_MIPS_64:
      write64le(loc, val);
      return;
    case R_MIPS_LO16: {
      uint32_t insn = read32le(loc);
      uint32_t lo = val & 0xFFFF;
      insn = (insn & 0xFFFF0000) | lo;
      write32le(loc, insn);
      return;
    }
    case R_MIPS_HI16: {
      uint32_t insn = read32le(loc);
      uint32_t hi = (val + 0x8000) >> 16;
      insn = (insn & 0xFFFF0000) | hi;
      write32le(loc, insn);
      return;
    }
    case R_MIPS_PC26_S2: {
      uint32_t insn = read32le(loc) & 0xFC000000;
      uint32_t imm26 = (val >> 2) & 0x03FFFFFF;
      insn |= imm26;
      write32le(loc, insn);
      return;
    }
    case R_MIPS_PC16: {
      uint32_t insn = read32le(loc) & 0xFFFF0000;
      uint32_t imm16 = (val >> 2) & 0xFFFF;
      insn |= imm16;
      write32le(loc, insn);
      return;
    }
    default:
      llvm_unreachable("Unknown DLX relocation type");
  }
}

// Register the DLX target
TargetInfo *getDLXTargetInfo() {
  static DLXTargetInfo target;
  return &target;
}

} // namespace elf
} // namespace lld
