//===-- DLXAsmBackend.cpp - DLX Asm Backend  ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DLXAsmBackend class.
//
//===----------------------------------------------------------------------===//
//

#include "MCTargetDesc/DLXFixupKinds.h"
#include "MCTargetDesc/DLXAsmBackend.h"

#include "MCTargetDesc/DLXMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

std::unique_ptr<MCObjectTargetWriter>
DLXAsmBackend::createObjectTargetWriter() const {
  return createDLXELFObjectWriter(TheTriple);
}

void DLXAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                               const MCValue &Target, MutableArrayRef<char> Data,
                               uint64_t Value, bool IsResolved,
                               const MCSubtargetInfo *STI) const {
  MCFixupKind Kind = Fixup.getKind();
  unsigned Offset = Fixup.getOffset();
  unsigned NumBytes = 4; // Assuming 32-bit instructions

  // Determine the mask and shift based on the fixup kind
  uint64_t Mask = 0;

  switch (Kind) {
  case DLX::fixup_DLX_LO16:
    Mask = 0xFFFF;
    Value &= Mask; // Mask to get the lower 16 bits
    break;

  case DLX::fixup_DLX_HI16:
    Mask = 0xFFFF;
    Value = (Value >> 16) & Mask; // Shift right to get the higher 16 bits
    break;

  case DLX::fixup_DLX_JAL_PC26:
    Value >>= 2; // Convert to word-aligned address for 26-bit offset
    Mask = 0x03FFFFFF;
    Value &= Mask; // Mask to get the 26-bit field
    break;

  default:
    llvm_unreachable("Unknown fixup kind!");
  }

  // Apply the fixup by inserting the value into the instruction
  for (unsigned i = 0; i < NumBytes; ++i) {
    Data[Offset + i] = uint8_t((Value >> (i * 8)) & 0xff);
  }
}

const MCFixupKindInfo &DLXAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
    const static MCFixupKindInfo Infos[DLX::LastTargetFixupKind - DLX::FirstTargetFixupKind] = {
        // Name, Offset, Bits, Flags
        { "fixup_DLX_LO16", 0, 16, 0 }, // Lower 16 bits
        { "fixup_DLX_HI16", 0, 16, 0 }, // Higher 16 bits
        { "fixup_DLX_JAL_PC26", 0, 26, MCFixupKindInfo::FKF_IsPCRel } // 26-bit PC-relative
    };

    if (Kind >= FirstTargetFixupKind && Kind < LastTargetFixupKind) {
        return Infos[Kind - FirstTargetFixupKind];
    }

    // Default case for standard fixups
    return MCAsmBackend::getFixupKindInfo(Kind);
}

// MCAsmBackend
MCAsmBackend *llvm::createDLXAsmBackend(const Target &T,
                                         const MCSubtargetInfo &STI,
                                         const MCRegisterInfo &MRI,
                                         const MCTargetOptions &Options) {
  return new DLXAsmBackend(T, STI.getTargetTriple());
}
