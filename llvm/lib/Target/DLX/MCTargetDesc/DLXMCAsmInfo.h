//===-- DLXMCAsmInfo.h - DLX Asm Info ------------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the DLXMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DLX_MCTARGETDESC_DLXMCASMINFO_H
#define LLVM_LIB_TARGET_DLX_MCTARGETDESC_DLXMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
  class Triple;

class DLXMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit DLXMCAsmInfo(const Triple &TheTriple);
};

} // namespace llvm

#endif // end LLVM_LIB_TARGET_DLX_MCTARGETDESC_DLXMCASMINFO_H