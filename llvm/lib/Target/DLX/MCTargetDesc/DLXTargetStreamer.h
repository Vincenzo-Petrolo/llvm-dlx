//===-- DLXTargetStreamer.h - DLX Target Streamer ----------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DLX_DLXTARGETSTREAMER_H
#define LLVM_LIB_TARGET_DLX_DLXTARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {

class DLXTargetStreamer : public MCTargetStreamer {
public:
  DLXTargetStreamer(MCStreamer &S);

};

// This part is for ascii assembly output
class DLXTargetAsmStreamer : public DLXTargetStreamer {
  formatted_raw_ostream &OS;

public:
  DLXTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);

};

}
#endif
