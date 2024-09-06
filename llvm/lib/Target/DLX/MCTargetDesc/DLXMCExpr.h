#include "llvm/MC/MCExpr.h"

namespace llvm {

class StringRef;
class MCOperand;

class DLXMCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_DLX_None,
    VK_DLX_LO,
    VK_DLX_HI,
    VK_DLX_CALL,
    VK_DLX_Invalid
  };
private:
  const MCExpr *Expr;
  const VariantKind Kind;

  int64_t evaluateAsInt64(int64_t Value) const;
  explicit DLXMCExpr(const MCExpr *Expr, VariantKind Kind)
      : Expr(Expr), Kind(Kind) {}

public:
  static const DLXMCExpr *create(const MCExpr *Expr, VariantKind Kind,
                                   MCContext &Ctx);
  VariantKind getKind() const { return Kind; }

  const MCExpr *getSubExpr() const { return Expr; }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;

  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;

  void visitUsedExpr(MCStreamer &Streamer) const override;

  MCFragment *findAssociatedFragment() const override {
      return getSubExpr()->findAssociatedFragment();
  }

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  bool evaluateAsConstant(int64_t &Res) const;

  static bool classof(const DLXMCExpr *) { return true; }

  static VariantKind getVariantKindForName(StringRef name);
  static StringRef getVariantKindName(VariantKind Kind);
};
}