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
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

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
  case DLX::fixup_DLX_J_26:
    Value >>= 2; // Convert to word-aligned address for 26-bit offset
    Mask = 0x03FFFFFF;
    Value &= Mask; // Mask to get the 26-bit field
    break;
  
  case DLX::fixup_DLX_BR_PC16:
    Value >>= 2; // Convert to word-aligned address for 16-bit offset
    Mask = 0xFFFF;
    Value &= Mask; // Mask to get the 16-bit field
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
    const static MCFixupKindInfo Infos[DLX::NumTargetFixupKinds] = {
        // Name, Offset, Bits, Flags
        { "fixup_DLX_LO16", 0, 16, 0 }, // Lower 16 bits
        { "fixup_DLX_HI16", 0, 16, 0 }, // Higher 16 bits
        { "fixup_DLX_JAL_PC26", 0, 26, MCFixupKindInfo::FKF_IsPCRel }, // 26-bit PC-relative
        { "fixup_DLX_BR_PC16", 0, 16, MCFixupKindInfo::FKF_IsPCRel }, // 16-bit PC-relative
        { "fixup_DLX_J_26", 0, 26, 0 } // 26-bit Absolute jump
    };

    if (Kind - DLX::fixup_DLX_first >= DLX::NumTargetFixupKinds)
      llvm_unreachable("Unknown fixup kind!");

    llvm::errs() << "FK Kind: " << Kind << "\n";

    return Infos[Kind - DLX::fixup_DLX_first];
}

// MCAsmBackend
MCAsmBackend *llvm::createDLXAsmBackend(const Target &T,
                                          const MCSubtargetInfo &STI,
                                          const MCRegisterInfo &MRI,
                                          const MCTargetOptions &Options) {
  const Triple &TT = STI.getTargetTriple();
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
  return new DLXAsmBackend(STI, OSABI, Options);
}

std::unique_ptr<MCObjectTargetWriter>
DLXAsmBackend::createObjectTargetWriter() const {
  return createDLXELFObjectWriter(OSABI, false);
}

void DLXAsmBackend::relaxInstruction(const MCInst &Inst,
                                       const MCSubtargetInfo &STI,
                                       MCInst &Res) const {
  llvm_unreachable("RelaxInstruction() unimplemented");
}

bool DLXAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count) const {
  unsigned MinNopLen = 4;

  if ((Count % MinNopLen) != 0)
    return false;

  // The canonical nop on DLX is addi x0, x0, 0.
  for (; Count >= 4; Count -= 4)
    OS.write("\x13\0\0\0", 4);

  return true;
}