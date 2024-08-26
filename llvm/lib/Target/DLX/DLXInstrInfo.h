//===-- DLXInstrInfo.h - DLX Instruction Information ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the DLX implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DLX_DLXINSTRINFO_H
#define LLVM_LIB_TARGET_DLX_DLXINSTRINFO_H

#include "DLX.h"
#include "DLXRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "DLXGenInstrInfo.inc"

namespace llvm {

class DLXInstrInfo : public DLXGenInstrInfo {
public:
  explicit DLXInstrInfo(const DLXSubtarget &STI);

protected:
  const DLXSubtarget &Subtarget;
};
}

#endif // end LLVM_LIB_TARGET_DLX_DLXINSTRINFO_H