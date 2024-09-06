//===-- DLXTargetStreamer.cpp - DLX Target Streamer Methods -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides DLX specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "DLXTargetStreamer.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

DLXTargetStreamer::DLXTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

// This part is for ascii assembly output
DLXTargetAsmStreamer::DLXTargetAsmStreamer(MCStreamer &S,
                                               formatted_raw_ostream &OS)
    : DLXTargetStreamer(S), OS(OS) {}
