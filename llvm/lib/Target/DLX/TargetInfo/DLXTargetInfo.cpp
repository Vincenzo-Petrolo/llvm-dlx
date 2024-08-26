//===-- DLXTargetInfo.cpp - DLX Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/DLXTargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheDLXTarget() {
  static Target TheDLXTarget;
  return TheDLXTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeDLXTargetInfo() {
  RegisterTarget<Triple::dlx32> X(getTheDLXTarget(), "dlx32",
                                  "32-bit DLX", "DLX");
}