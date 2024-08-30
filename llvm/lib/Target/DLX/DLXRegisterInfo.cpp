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
  : DLXGenRegisterInfo(DLX::R1, /*DwarfFlavour*/0, /*EHFlavor*/0,
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

  markSuperRegs(Reserved, DLX::R0); // zero
  markSuperRegs(Reserved, DLX::R2); // sp
  markSuperRegs(Reserved, DLX::R3); // gp
  markSuperRegs(Reserved, DLX::R4); // tp

  return Reserved;
}

void DLXRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj,
                                           unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const DLXInstrInfo *TII = MF.getSubtarget<DLXSubtarget>().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  unsigned FrameReg;
  int Offset =
      getFrameLowering(MF)->getFrameIndexReference(MF, FrameIndex, FrameReg) +
      MI.getOperand(FIOperandNum + 1).getImm();
  if (!isInt<32>(Offset)) {
    report_fatal_error(
        "Frame offsets outside of the signed 32-bit range not supported");
  }
  MachineBasicBlock &MBB = *MI.getParent();
  bool FrameRegIsKill = false;
  if (!isInt<16>(Offset)) {
    assert(isInt<32>(Offset) && "Int32 expected");
    llvm_unreachable("Int32 offset not implemented");
  }
  MI.getOperand(FIOperandNum)
      .ChangeToRegister(FrameReg, false, false, FrameRegIsKill);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
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
  const TargetFrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? DLX::R8 : DLX::R2;
}

Register DLXRegisterInfo::getRARegister() const { return DLX::R31; }