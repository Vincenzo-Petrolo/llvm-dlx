//===-- DLXRegisterInfo.cpp - DLX Register Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the DLX implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "DLXRegisterInfo.h"
#include "DLXSubtarget.h"
#include "llvm/Support/Debug.h"

#define GET_REGINFO_TARGET_DESC
#include "DLXGenRegisterInfo.inc"

#define DEBUG_TYPE "dlx-reginfo"

using namespace llvm;

DLXRegisterInfo::DLXRegisterInfo(const DLXSubtarget &ST)
  : DLXGenRegisterInfo(DLX::X1, /*DwarfFlavour*/0, /*EHFlavor*/0,
                         /*PC*/0), Subtarget(ST) {}

const MCPhysReg *
DLXRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return DLX_CalleeSavedRegs_SaveList;
}

const TargetRegisterClass *DLXRegisterInfo::intRegClass(unsigned Size) const {
  return &DLX::GPRRegClass;
}

const uint32_t *
DLXRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                        CallingConv::ID) const {
  return DLX_CalleeSavedRegs_RegMask;
}

BitVector DLXRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  markSuperRegs(Reserved, DLX::X0); // zero
  markSuperRegs(Reserved, DLX::X2); // sp
  markSuperRegs(Reserved, DLX::X3); // gp
  markSuperRegs(Reserved, DLX::X4); // tp

  return Reserved;
}

void DLXRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj,
                                           unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  llvm_unreachable("Unsupported eliminateFrameIndex");
}

bool
DLXRegisterInfo::requiresRegisterScavenging(const MachineFunction &MF) const {
  return true;
}

bool
DLXRegisterInfo::requiresFrameIndexScavenging(
                                            const MachineFunction &MF) const {
  return true;
}

bool
DLXRegisterInfo::requiresFrameIndexReplacementScavenging(
                                            const MachineFunction &MF) const {
  return true;
}

bool
DLXRegisterInfo::trackLivenessAfterRegAlloc(const MachineFunction &MF) const {
  return true;
}

Register DLXRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  llvm_unreachable("Unsupported getFrameRegister");
}

