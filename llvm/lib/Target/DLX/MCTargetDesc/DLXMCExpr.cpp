#include "DLXMCExpr.h"
#include "MCTargetDesc/DLXAsmBackend.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCSymbol.h"

using namespace llvm;

const DLXMCExpr *DLXMCExpr::create(const MCExpr *Expr, VariantKind Kind,
                                       MCContext &Ctx) {
  return new (Ctx) DLXMCExpr(Expr, Kind);
}

void DLXMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  if (Kind == VK_DLX_LO)
    OS << "%lo(";
  else if (Kind == VK_DLX_HI)
    OS << "%hi(";
  else if (Kind == VK_DLX_CALL)
    OS << "%pcrel(";
  
  Expr->print(OS, MAI);
  OS << ")";
}

bool DLXMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                          const MCAsmLayout *Layout,
                                          const MCFixup *Fixup) const {
  if (!getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup))
    return false;

  // Some custom fixup types are not valid with symbol difference expressions
  if (Res.getSymA() && Res.getSymB()) {
    switch (getKind()) {
    default:
      return true;
    case VK_DLX_LO:
    case VK_DLX_HI:
    case VK_DLX_CALL:
      return false;
    }
  }

  return true;
}

void DLXMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}


DLXMCExpr::VariantKind DLXMCExpr::getVariantKindForName(StringRef name) {
  return StringSwitch<DLXMCExpr::VariantKind>(name)
      .Case("lo", VK_DLX_LO)
      .Case("hi", VK_DLX_HI)
      .Case("CALL", VK_DLX_CALL)
      .Default(VK_DLX_None);
}

StringRef DLXMCExpr::getVariantKindName(VariantKind Kind) {
  switch (Kind) {
  default:
    llvm_unreachable("Invalid ELF symbol kind");
  case VK_DLX_LO:
    return "lo";
  case VK_DLX_HI:
    return "hi";
  case VK_DLX_CALL:
    return "call";
  }
}

void DLXMCExpr::fixELFSymbolsInTLSFixups(MCAssembler &Asm) const {
  // No threads local storage (TLS)
  return;
}

