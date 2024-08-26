//===-- DLXInstrInfo.cpp - DLX Instruction Information ----------------===//
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

#include "DLXInstrInfo.h"

#include "DLXTargetMachine.h"
#include "DLXMachineFunction.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "dlx-instrinfo"

#define GET_INSTRINFO_CTOR_DTOR
#include "DLXGenInstrInfo.inc"

DLXInstrInfo::DLXInstrInfo(const DLXSubtarget &STI)
    : DLXGenInstrInfo(DLX::ADJCALLSTACKDOWN, DLX::ADJCALLSTACKUP),
      Subtarget(STI)
{
}