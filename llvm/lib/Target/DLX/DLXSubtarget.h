//===-- DLXSubtarget.h - Define Subtarget for the DLX -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DLX specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DLX_DLXSUBTARGET_H
#define LLVM_LIB_TARGET_DLX_DLXSUBTARGET_H

#include "DLXFrameLowering.h"
#include "DLXISelLowering.h"
#include "DLXInstrInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/MC/MCInstrItineraries.h"

#define GET_SUBTARGETINFO_HEADER
#include "DLXGenSubtargetInfo.inc"

namespace llvm {
class DLXSubtarget : public DLXGenSubtargetInfo {
protected:
  SelectionDAGTargetInfo TSInfo;
  DLXInstrInfo InstrInfo;
  DLXFrameLowering FrameLowering;
  DLXTargetLowering TLInfo;
  DLXRegisterInfo RegInfo;

public:
  /// This constructor initializes the data members to match that
  /// of the specified triple.
  DLXSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                const TargetMachine &TM);

  /// ParseSubtargetFeatures - Parses features string setting specified
  /// subtarget options.  Definition of function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);

  DLXSubtarget &initializeSubtargetDependencies(const Triple &TT,
                                                  StringRef CPU, StringRef FS,
                                                  const TargetMachine &TM);

  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }

  const DLXInstrInfo *getInstrInfo() const override {
    return &InstrInfo;
  }

  const TargetFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }

  const DLXRegisterInfo *getRegisterInfo() const override {
    return &RegInfo;
  }

  const DLXTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }

  /// getMaxInlineSizeThreshold - Returns the maximum memset / memcpy size
  /// that still makes it profitable to inline the call.
  unsigned getMaxInlineSizeThreshold() const {
    return 64;
  }
};
}

#endif // end LLVM_LIB_TARGET_DLX_DLXSUBTARGET_H