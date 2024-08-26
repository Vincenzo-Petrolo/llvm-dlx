
//===-- DLXMCTargetDesc.cpp - DLX Target Descriptions -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides DLX specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "DLXMCTargetDesc.h"
#include "DLXInstPrinter.h"
#include "DLXMCAsmInfo.h"
#include "TargetInfo/DLXTargetInfo.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "DLXGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "DLXGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "DLXGenRegisterInfo.inc"

static MCInstrInfo *createDLXMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitDLXMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createDLXMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  return X;
}

static MCSubtargetInfo *
createDLXMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "generic";
  return createDLXMCSubtargetInfoImpl(TT, CPUName, FS);
}

static MCInstPrinter *createDLXMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new DLXInstPrinter(MAI, MII, MRI);
}

static MCAsmInfo *createDLXMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new DLXMCAsmInfo(TT);

  unsigned WP = MRI.getDwarfRegNum(DLX::X2, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, WP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

extern "C" void LLVMInitializeDLXTargetMC() {
  for (Target *T : {&getTheDLXTarget()}) {
    // Register the MC asm info.
    TargetRegistry::RegisterMCAsmInfo(*T, createDLXMCAsmInfo);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createDLXMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createDLXMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createDLXMCSubtargetInfo);

    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(*T, createDLXMCInstPrinter);
  }
}