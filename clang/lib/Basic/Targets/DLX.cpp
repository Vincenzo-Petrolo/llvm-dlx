//===--- DLX.cpp - Implement DLX target feature support ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements DLXTargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "DLX.h"
#include "clang/Basic/MacroBuilder.h"
#include "llvm/ADT/StringSwitch.h"

using namespace clang;
using namespace clang::targets;

const char *const DLXTargetInfo::GCCRegNames[] = {
  // Integer registers
  "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
  "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
  "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
};

const TargetInfo::GCCRegAlias GCCRegAliases[] = {
  {{"zero"}, "r0"}, {{"ra"}, "r1"},   {{"sp"}, "r2"},    {{"gp"}, "r3"},
  {{"tp"}, "r4"},   {{"t0"}, "r5"},   {{"t1"}, "r6"},    {{"t2"}, "r7"},
  {{"s0"}, "r8"},   {{"s1"}, "r9"},   {{"a0"}, "r10"},   {{"a1"}, "r11"},
  {{"a2"}, "r12"},  {{"a3"}, "r13"},  {{"a4"}, "r14"},   {{"a5"}, "r15"},
  {{"a6"}, "r16"},  {{"a7"}, "r17"},  {{"s2"}, "r18"},   {{"s3"}, "r19"},
  {{"s4"}, "r20"},  {{"s5"}, "r21"},  {{"s6"}, "r22"},   {{"s7"}, "r23"},
  {{"s8"}, "r24"},  {{"s9"}, "r25"},  {{"s10"}, "r26"},  {{"s11"}, "r27"},
  {{"t3"}, "r28"},  {{"t4"}, "r29"},  {{"t5"}, "r30"},   {{"t6"}, "r31"}};

ArrayRef<const char *> DLXTargetInfo::getGCCRegNames() const {
  return llvm::makeArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias> DLXTargetInfo::getGCCRegAliases() const {
  return llvm::makeArrayRef(GCCRegAliases);
}

void DLXTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  // Define the __DLX__ macro when building for this target
  Builder.defineMacro("__DLX__");
}