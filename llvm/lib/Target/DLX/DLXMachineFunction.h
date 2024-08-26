//=== DLXMachineFunctionInfo.h - Private data used for DLX ----*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DLX specific subclass of MachineFunctionInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DLX_DLXMACHINEFUNCTION_H
#define LLVM_LIB_TARGET_DLX_DLXMACHINEFUNCTION_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// DLXFunctionInfo - This class is derived from MachineFunction private
/// DLX target-specific information for each MachineFunction.
class DLXFunctionInfo : public MachineFunctionInfo {
private:
  MachineFunction &MF;

public:
  DLXFunctionInfo(MachineFunction &MF) : MF(MF) {}
};

} // end of namespace llvm

#endif // end LLVM_LIB_TARGET_DLX_DLXMACHINEFUNCTION_H