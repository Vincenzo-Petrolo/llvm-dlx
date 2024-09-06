//===-- DLXELFObjectWriter.cpp - DLX ELF Writer -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/DLXFixupKinds.h"
#include "MCTargetDesc/DLXMCExpr.h"
#include "MCTargetDesc/DLXMCTargetDesc.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class DLXELFObjectWriter : public MCELFObjectTargetWriter {
public:
  DLXELFObjectWriter(uint8_t OSABI, bool Is64Bit);

  ~DLXELFObjectWriter() override;

  // Return true if the given relocation must be with a symbol rather than
  // section plus offset.
  bool needsRelocateWithSymbol(const MCSymbol &Sym,
                               unsigned Type) const override {
    // TODO: this is very conservative, update once RISC-V psABI requirements
    //       are clarified.
    return true;
  }

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};
}

DLXELFObjectWriter::DLXELFObjectWriter(uint8_t OSABI, bool Is64Bit)
    : MCELFObjectTargetWriter(/*Is64Bit=*/ false, OSABI, ELF::EM_DLX,
                              /*HasRelocationAddend*/ true) {}

DLXELFObjectWriter::~DLXELFObjectWriter() {}

unsigned DLXELFObjectWriter::getRelocType(MCContext &Ctx,
                                            const MCValue &Target,
                                            const MCFixup &Fixup,
                                            bool IsPCRel) const {
  const MCExpr *Expr = Fixup.getValue();
  // Determine the type of the relocation
  unsigned Kind = Fixup.getTargetKind();
  if (IsPCRel) {
    switch (Kind) {
    default:
      Ctx.reportError(Fixup.getLoc(), "Unsupported relocation type");
      return ELF::R_DLX_NONE;
    case DLX::fixup_DLX_JAL_PC26:
      return ELF::R_DLX_JAL_PC26;
    }
  }

  switch (Kind) {
  default:
    Ctx.reportError(Fixup.getLoc(), "Unsupported relocation type");
    return ELF::R_DLX_NONE;
  case FK_Data_1:
    Ctx.reportError(Fixup.getLoc(), "1-byte data relocations not supported");
    return ELF::R_DLX_NONE;
  case FK_Data_2:
    Ctx.reportError(Fixup.getLoc(), "2-byte data relocations not supported");
    return ELF::R_DLX_NONE;
  case FK_Data_4:
    return ELF::R_DLX_32;
  case FK_Data_8:
    return ELF::R_DLX_64;
  }
}

std::unique_ptr<MCObjectTargetWriter>
createDLXELFObjectWriter(uint8_t OSABI, bool Is64Bit) {
  return std::make_unique<DLXELFObjectWriter>(OSABI, Is64Bit);
}
