#include "DLXMCExpr.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCSymbol.h"

using namespace llvm;
using namespace llvm::DLX;

const DLXMCExpr *DLXMCExpr::create(const MCExpr *Expr, VariantKind Kind,
                                   MCContext &Ctx) {
  return new (Ctx) DLXMCExpr(Expr, Kind);
}

void DLXMCExpr::PrintImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
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
  if (!Expr->evaluateAsRelocatable(Res, Layout, Fixup))
    return false;

  switch (Kind) {
  case VK_DLX_LO:
    Res = MCValue::get(Res.getSymA(), Res.getSymB(), Res.getConstant() & 0xFFFF);
    break;
  case VK_DLX_HI:
    Res = MCValue::get(Res.getSymA(), Res.getSymB(), (Res.getConstant() >> 16) & 0xFFFF);
    break;
  case VK_DLX_CALL:
    // For PC-relative, we usually want to compute a relocation entry, 
    // so just pass the expression as is, letting the linker compute
    // the final value.
    break;
  default:
    break;
  }

  return true;
}