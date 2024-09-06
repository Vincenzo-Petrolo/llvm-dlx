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

bool DLXMCExpr::evaluateAsConstant(int64_t &Res) const {
  MCValue Value;

  if (Kind == VK_DLX_CALL)
    return false;

  if (!getSubExpr()->evaluateAsRelocatable(Value, nullptr, nullptr))
    return false;

  if (!Value.isAbsolute())
    return false;

  Res = evaluateAsInt64(Value.getConstant());
  return true;
}

int64_t DLXMCExpr::evaluateAsInt64(int64_t Value) const {
  switch (Kind) {
  default:
    llvm_unreachable("Invalid kind");
  case VK_DLX_LO:
    return SignExtend64<12>(Value);
  case VK_DLX_HI:
    // Add 1 if bit 11 is 1, to compensate for low 16 bits being negative.
    return ((Value + 0x800) >> 16) & 0xffffff;
  }
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

