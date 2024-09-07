//===---- DLXAsmBackend.h - DLX Asm Backend  ------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the DLXAsmBackend class.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_LIB_TARGET_DLX_MCTARGETDESC_DLXASMBACKEND_H
#define LLVM_LIB_TARGET_DLX_MCTARGETDESC_DLXASMBACKEND_H

#include "MCTargetDesc/DLXFixupKinds.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCAsmBackend.h"

namespace llvm {

class MCAssembler;
struct MCFixupKindInfo;
class Target;
class MCObjectWriter;
class MCTargetOptions;

class DLXAsmBackend : public MCAsmBackend {
  Triple TheTriple;
  const MCSubtargetInfo &STI;
  uint8_t OSABI;
  bool ForceRelocs = false;
  const MCTargetOptions &TargetOptions;
  //todo DLXABI::ABI TargetABI = RISCVABI::ABI_Unknown; todo

public:
  DLXAsmBackend(const MCSubtargetInfo &STI, uint8_t OSABI,
                  const MCTargetOptions &Options)
      : MCAsmBackend(support::little), STI(STI), OSABI(OSABI),
        TargetOptions(Options) {
    //todo TargetABI = DLXABI::computeTargetABI(STI.getTargetTriple(), STI.getFeatureBits(), Options.getABIName()); todo
    //todo DLXFeatures::validate(STI.getTargetTriple(), STI.getFeatureBits());
  }
  ~DLXAsmBackend() override {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;

  unsigned getNumFixupKinds() const override {
    return DLX::NumTargetFixupKinds;
  }

  bool mayNeedRelaxation(const MCInst &Inst,
                         const MCSubtargetInfo &STI) const override {
    return false;
  }

   bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                             const MCRelaxableFragment *DF,
                             const MCAsmLayout &Layout) const override {
    return false;
  }

  
  void relaxInstruction(const MCInst &Inst, const MCSubtargetInfo &STI,
                                MCInst &Res) const override;


  bool writeNopData(raw_ostream &OS, uint64_t Count) const override;
}; // class DLXAsmBackend

} // namespace

#endif
