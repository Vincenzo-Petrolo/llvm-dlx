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
    fixup_DLX_LO16 =      FirstTargetFixupKind + 5, // Lower 16 bits
    fixup_DLX_first =     fixup_DLX_LO16,
    fixup_DLX_HI16 =      FirstTargetFixupKind + 4,      // Higher 16 bits
    fixup_DLX_JAL_PC26 =  FirstTargetFixupKind + 3,                   // 26-bit PC-relative address for calls/jumps
    fixup_DLX_BR_PC16 =   FirstTargetFixupKind + 9,                    // 16-bit PC-relative address for branches
    NumTargetFixupKinds = 4
  };
} // namespace DLX
} // namespace llvm

#endif // LLVM_CPU0_CPU0FIXUPKINDS_H