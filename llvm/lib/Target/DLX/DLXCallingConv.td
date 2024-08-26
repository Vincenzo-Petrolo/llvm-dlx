//=== DLXCallingConv.td - Calling Conventions for DLX --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for DLX architecture.
//===----------------------------------------------------------------------===//

def DLX_CCallingConv : CallingConv<[
  // Handles byval parameters. Tell LLVM what the size and alignment
  // requirements of each stack slot are
  CCIfByVal<CCPassByVal<4, 4>>,

  // Promote i8/i16 to i32
  CCIfType<[i1, i8, i16], CCPromoteToType<i32>>,

  // Up to 8 integer arguments get passed in registers
  CCIfType<[i32], CCAssignToReg<[R10, R11, R12, R13, R14, R15, R16, R17]>>,

  // Integer values get stored in stack slots that are 4 bytes in size and
  // 4-byte aligned
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def DLX_CRetConv : CallingConv<[
  CCIfType<[i1, i8, i16], CCPromoteToType<i32>>,

  // Return value in two registers
  CCIfType<[i32], CCAssignToReg<[R10, R11]>>,

  // Integer values get stored in stack slots that are 4 bytes in size and
  // 4-byte aligned
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def DLX_CalleeSavedRegs : CalleeSavedRegs<( add R1,
  (sequence "R%u", 3, 9),
  (sequence "R%u", 10, 11),
  (sequence "R%u", 12, 17),
  (sequence "R%u", 18, 27),
  (sequence "R%u", 28, 31)
)>;