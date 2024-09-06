//===-- DLXELFStreamer.cpp - DLX ELF Target Streamer Methods ----------===//
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

#include "DLXELFStreamer.h"
#include "MCTargetDesc/DLXAsmBackend.h"
#include "MCTargetDesc/DLXMCTargetDesc.h"
#include "DLXBaseInfo.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCSubtargetInfo.h"

using namespace llvm;

// This part is for ELF object output.
DLXTargetELFStreamer::DLXTargetELFStreamer(MCStreamer &S,
                                               const MCSubtargetInfo &STI)
    : DLXTargetStreamer(S) {
  MCAssembler &MCA = getStreamer().getAssembler();
  const FeatureBitset &Features = STI.getFeatureBits();
  auto &MAB = static_cast<DLXAsmBackend &>(MCA.getBackend());
}

MCELFStreamer &DLXTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
