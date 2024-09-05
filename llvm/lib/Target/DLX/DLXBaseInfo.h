//===-- DLXBaseInfo.h - Top level definitions for DLX MC ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone enum definitions for the DLX target
// useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_DLX_MCTARGETDESC_DLXBASEINFO_H
#define LLVM_LIB_TARGET_DLX_MCTARGETDESC_DLXBASEINFO_H

#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/SubtargetFeature.h"

namespace llvm {

// DLXII - This namespace holds all of the target specific flags that
// instruction info tracks. All definitions must match DLXInstrFormats.td.
namespace DLXII {
enum {
  InstFormatPseudo = 0,
  InstFormatR = 1,
  InstFormatI = 2,
  InstFormatJ = 3,
  InstFormatOther = 4,

  InstFormatMask = 31
};

// DLX Specific Machine Operand Flags
enum {
  MO_None = 0,
  MO_CALL = 1,
  MO_LO = 2,
  MO_HI = 3,
};

} // namespace DLXII

namespace DLXOp {
enum OperandType : unsigned {
  OPERAND_FIRST_DLX_IMM = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_SIMM16 = OPERAND_FIRST_DLX_IMM,
  OPERAND_BRTARGET,
  OPERAND_JMPTARGET,
  OPERAND_UIMM16,
  OPERAND_LAST_DLX_IMM = OPERAND_UIMM16,
};
} // namespace DLXOp

} // namespace llvm

#endif