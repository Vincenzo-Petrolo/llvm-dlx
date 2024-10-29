//===-- DLXFixupKinds.h - DLX Specific Fixup Entries ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_MCTARGETDESC_CPU0FIXUPKINDS_H
#define LLVM_LIB_TARGET_CPU0_MCTARGETDESC_CPU0FIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace DLX {
enum Fixups {
  fixup_DLX_first = FirstTargetFixupKind,
  fixup_DLX_LO16 = FirstTargetFixupKind, // Lower 16 bits
  fixup_DLX_HI16,                        // Higher 16 bits
  fixup_DLX_JAL_PC26, // 26-bit PC-relative address for calls/jumps
  fixup_DLX_BR_PC16,  // 16-bit PC-relative address for branches
  fixup_DLX_J_26,   // 26-bit absolute address for jumps
  fixup_DLX_invalid,
  NumTargetFixupKinds = fixup_DLX_invalid - FirstTargetFixupKind
};
} // namespace DLX
} // namespace llvm

#endif // LLVM_CPU0_CPU0FIXUPKINDS_H