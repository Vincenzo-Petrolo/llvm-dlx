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
  // Handles byval parameters. Tell LLVM the size and alignment
  // requirements of each stack slot
  CCIfByVal<CCPassByVal<4, 4>>,

  // Promote i1/i8/i16 to i32
  CCIfType<[i1, i8, i16], CCPromoteToType<i32>>,

  // Promote f32 to i32 in soft-float mode
  CCIfType<[f32], CCPromoteToType<i32>>,

  // Handle i64 and f64 types using a custom function
  CCIfType<[i64, f64], CCCustom<"DLXHandleI64">>,

  // Assign i32 and promoted types to registers
  CCIfType<[i32], CCAssignToReg<[R10, R11, R12, R13, R14, R15, R16, R17]>>,

  // Assign i32 to stack if no registers are available
  CCIfType<[i32], CCAssignToStack<4, 4>>,

  // Fallback: assign any remaining types to the stack
  CCAssignToStack<4, 4>
]>;

def DLX_CRetConv : CallingConv<[
  // Promote i1/i8/i16 to i32
  CCIfType<[i1, i8, i16], CCPromoteToType<i32>>,

  // Promote f32 to i32 in soft-float mode
  CCIfType<[f32], CCPromoteToType<i32>>,

  // Handle i64 and f64 types using a custom function
  CCIfType<[i64, f64], CCCustom<"DLXHandleI64Ret">>,

  // Return i32 and promoted types in R10
  CCIfType<[i32], CCAssignToReg<[R10]>>,

  // Return i32 on stack if necessary
  CCIfType<[i32], CCAssignToStack<4, 4>>,

  // Fallback: assign any remaining types to the stack
  CCAssignToStack<4, 4>
]>;

def DLX_CalleeSavedRegs : CalleeSavedRegs<( add R2,
  (sequence "R%u", 8, 9),
  (sequence "R%u", 18, 27)
)>;