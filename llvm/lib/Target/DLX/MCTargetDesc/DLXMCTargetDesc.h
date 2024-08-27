//===-- DLXMCTargetDesc.h - DLX Target Descriptions ---------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_DLX_MCTARGETDESC_DLXMCTARGETDESC_H
#define LLVM_LIB_TARGET_DLX_MCTARGETDESC_DLXMCTARGETDESC_H

#include "DLXBaseInfo.h"

// Defines symbolic names for DLX registers. This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "DLXGenRegisterInfo.inc"

// Defines symbolic names for the DLX instructions.
#define GET_INSTRINFO_ENUM
#include "DLXGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "DLXGenSubtargetInfo.inc"

#endif // end LLVM_LIB_TARGET_DLX_MCTARGETDESC_DLXMCTARGETDESC_H