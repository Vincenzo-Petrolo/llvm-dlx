//=== DLX.td - Describe the DLX Target Machine ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Top tablegen file that pulls in code from the LLVM framework along with
// code specific to this target.
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Calling Conv, Instruction Descriptions, Register information, etc
//===----------------------------------------------------------------------===//

include "DLXSchedule.td"
include "DLXRegisterInfo.td"
include "DLXCallingConv.td"
include "DLXInstrInfo.td"

def DLXInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// DLX processors supported
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic",  NoSchedModel, []>;

//===----------------------------------------------------------------------===//
// DLX Target declaration
//===----------------------------------------------------------------------===//

def DLX : Target {
  let InstructionSet = DLXInstrInfo;
}