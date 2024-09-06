//===-- DLXELFStreamer.h - DLX ELF Target Streamer ---------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DLX_DLXELFSTREAMER_H
#define LLVM_LIB_TARGET_DLX_DLXELFSTREAMER_H

#include "DLXTargetStreamer.h"
#include "llvm/MC/MCELFStreamer.h"

namespace llvm {

class DLXTargetELFStreamer : public DLXTargetStreamer {
public:
  MCELFStreamer &getStreamer();
  DLXTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

};
}
#endif
