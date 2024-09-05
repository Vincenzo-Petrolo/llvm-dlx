#include "DLX.h"
#include "DLXRegisterInfo.h"
#include "MCTargetDesc/DLXMCExpr.h"
#include "MCTargetDesc/DLXMCInstrInfo.h"
#include "MCTargetDesc/DLXMCRegisterInfo.h"
#include "TargetInfo/DLXTargetInfo.h"
#include "llvm/MC/MCAsmParser.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;

namespace {


class DLXAsmParser : public MCTargetAsmParser {
#define GET_ASSEMBLER_HEADER
#include "DLXGenAsmMatcher.inc"

  // Constructor
  DLXAsmParser(const MCSubtargetInfo &sti, MCAsmParser &parser, const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, sti), MII(MII), Parser(parser) {
    // Initialize the instruction set
    setAvailableFeatures(ComputeAvailableFeatures(sti.getFeatureBits()));
  }

  // Required function to match and emit an instruction
  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode, OperandVector &Operands, MCStreamer &Out, uint64_t &ErrorInfo, bool MatchingInlineAsm) override;

  // Function to parse operands
  bool ParseOperand(OperandVector &Operands, StringRef Mnemonic);

  // Functions to parse specific operand types
  bool ParseRegister(OperandVector &Operands);
  bool ParseImmediate(OperandVector &Operands);
  
  // Error handling utility
  void Error(SMLoc IDLoc, const Twine &Message);

  // Additional utility functions if needed
  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name, SMLoc NameLoc, OperandVector &Operands) override;

private:
  const MCInstrInfo &MII;
  MCAsmParser &Parser;
};

struct DLXOperand : public MCParsedAsmOperand {

  enum class KindTy {
    Token,
    Register,
    Immediate,
  } Kind;

  struct RegOp {
    Register RegNum;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  SMLoc StartLoc, EndLoc;
  union {
    StringRef Tok;
    RegOp Reg;
    ImmOp Imm;
  };

  DLXOperand(KindTy K) : MCParsedAsmOperand(), Kind(K) {}

public:
  DLXOperand(const DLXOperand &o) : MCParsedAsmOperand() {
    Kind = o.Kind;
    StartLoc = o.StartLoc;
    EndLoc = o.EndLoc;
    switch (Kind) {
    case KindTy::Register:
      Reg = o.Reg;
      break;
    case KindTy::Immediate:
      Imm = o.Imm;
      break;
    case KindTy::Token:
      Tok = o.Tok;
      break;
    }
  }

  bool isToken() const override { return Kind == KindTy::Token; }
  bool isReg() const override { return Kind == KindTy::Register; }
  bool isImm() const override { return Kind == KindTy::Immediate; }
  bool isMem() const override { return false; }

  bool isGPR() const {
    return Kind == KindTy::Register &&
           DLXMCRegisterClasses[DLX::GPRRegClassID].contains(Reg.RegNum);
  }

  static bool evaluateConstantImm(const MCExpr *Expr, int64_t &Imm,
                                  DLXMCExpr::VariantKind &VK) {
    if (auto *RE = dyn_cast<DLXMCExpr>(Expr)) {
      VK = RE->getKind();
      return RE->evaluateAsConstant(Imm);
    }

    if (auto CE = dyn_cast<MCConstantExpr>(Expr)) {
      VK = DLXMCExpr::VK_DLX_None;
      Imm = CE->getValue();
      return true;
    }

    return false;
  }

  bool isBareSymbol() const {
    int64_t Imm;
    DLXMCExpr::VariantKind VK = DLXMCExpr::VK_DLX_None;
    // Must be of 'immediate' type but not a constant.
    if (!isImm() || evaluateConstantImm(getImm(), Imm, VK))
      return false;
    return DLXAsmParser::classifySymbolRef(getImm(), VK, Imm) &&
           VK == DLXMCExpr::VK_DLX_None;
  }

  bool isCallSymbol() const {
    int64_t Imm;
    DLXMCExpr::VariantKind VK = DLXMCExpr::VK_DLX_None;
    // Must be of 'immediate' type but not a constant.
    if (!isImm() || evaluateConstantImm(getImm(), Imm, VK))
      return false;
    return DLXAsmParser::classifySymbolRef(getImm(), VK, Imm) &&
           (VK == DLXMCExpr::VK_DLX_CALL ||
            VK == DLXMCExpr::VK_DLX_CALL_PLT);
  }

  bool isSImm16() const {
    DLXMCExpr::VariantKind VK = DLXMCExpr::VK_DLX_None;
    int64_t Imm;
    bool IsValid;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    if (!IsConstantImm)
      IsValid = DLXAsmParser::classifySymbolRef(getImm(), VK, Imm);
    else
      IsValid = isInt<16>(Imm);
    return IsValid && ((IsConstantImm && VK == DLXMCExpr::VK_DLX_None) ||
                       VK == DLXMCExpr::VK_DLX_LO ||
                       VK == DLXMCExpr::VK_DLX_PCREL_LO ||
                       VK == DLXMCExpr::VK_DLX_TPREL_LO);
  }

  /// getStartLoc - Gets location of the first token of this operand
  SMLoc getStartLoc() const override { return StartLoc; }
  /// getEndLoc - Gets location of the last token of this operand
  SMLoc getEndLoc() const override { return EndLoc; }

  unsigned getReg() const override {
    assert(Kind == KindTy::Register && "Invalid type access!");
    return Reg.RegNum.id();
  }

  StringRef getSysReg() const {
    assert(Kind == KindTy::SystemRegister && "Invalid access!");
    return StringRef(SysReg.Data, SysReg.Length);
  }

  const MCExpr *getImm() const {
    assert(Kind == KindTy::Immediate && "Invalid type access!");
    return Imm.Val;
  }

  StringRef getToken() const {
    assert(Kind == KindTy::Token && "Invalid type access!");
    return Tok;
  }

  void print(raw_ostream &OS) const override {
    switch (Kind) {
    case KindTy::Immediate:
      OS << *getImm();
      break;
    case KindTy::Register:
      OS << "<register x";
      OS << getReg() << ">";
      break;
    case KindTy::Token:
      OS << "'" << getToken() << "'";
      break;
    case KindTy::SystemRegister:
      OS << "<sysreg: " << getSysReg() << '>';
      break;
    }
  }

  static std::unique_ptr<DLXOperand> createToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<DLXOperand>(KindTy::Token);
    Op->Tok = Str;
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static std::unique_ptr<DLXOperand> createReg(unsigned RegNo, SMLoc S, SMLoc E) {
    auto Op = std::make_unique<DLXOperand>(KindTy::Register);
    Op->Reg.RegNum = RegNo;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<DLXOperand> createImm(const MCExpr *Val, SMLoc S, SMLoc E) {
    auto Op = std::make_unique<DLXOperand>(KindTy::Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    assert(Expr && "Expr shouldn't be null!");
    int64_t Imm = 0;
    DLXMCExpr::VariantKind VK = DLXMCExpr::VK_DLX_None;
    bool IsConstant = evaluateConstantImm(Expr, Imm, VK);

    if (IsConstant)
      Inst.addOperand(MCOperand::createImm(Imm));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  // Used by the TableGen Code
  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

};
} // end anonymous namespace.

// Implementation of MatchAndEmitInstruction
bool DLXAsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode, OperandVector &Operands, MCStreamer &Out, uint64_t &ErrorInfo, bool MatchingInlineAsm) {
  // Utilize TableGen generated code to match instructions
  MCInst Inst;
  unsigned Error;
  MatchResultTy MatchResult = MatchInstructionImpl(Operands, Inst, Error, MatchingInlineAsm);

  switch (MatchResult) {
    case Match_Success:
      // Emit the matched instruction to the output streamer
      Out.EmitInstruction(Inst, getSTI());
      return false;
    case Match_MissingFeature:
      Error(IDLoc, "instruction use requires a feature not currently enabled");
      return true;
    case Match_InvalidOperand:
      Error(IDLoc, "invalid operand for instruction");
      return true;
    case Match_MnemonicFail:
      Error(IDLoc, "unknown instruction mnemonic");
      return true;
  }

  // If we reach here, something went wrong
  llvm_unreachable("unexpected match result");
  return true;
}

// Implementation of ParseOperand
bool DLXAsmParser::ParseOperand(OperandVector &Operands, StringRef Mnemonic) {
  // Attempt to parse token as a register.
  if (ParseRegister(Operands) == MatchOperand_Success)
    return false;

  // Attempt to parse token as an immediate
  if (parseImmediate(Operands) == MatchOperand_Success) {
    // Parse memory base register if present
    if (getLexer().is(AsmToken::LParen))
      return parseMemOpBaseReg(Operands) != MatchOperand_Success;
    return false;
  }

  // Finally we have exhausted all options and must declare defeat.
  Error(getLoc(), "unknown operand");
  return true;
}

// Implementation for specific operand types
bool DLXAsmParser::ParseRegister(OperandVector &Operands) {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() -1);

  switch(getLexer().getKind()) {
    default: return 0;
    case AsmToken::Identifier:
      RegNo = MatchRegisterName(getLexer().getTok().getIdentifier());
      if (RegNo == 0)
        return 0;
      getLexer().Lex();
      Operands.push_back(DLXOperand::createReg(RegNo, S, E));
  }
  return 0;
}

bool DLXAsmParser::ParseImmediate(OperandVector &Operands) {
  SMLoc S = getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);
  const MCExpr *Res;

  switch (getLexer().getKind()) {
  default:
    return MatchOperand_NoMatch;
  case AsmToken::LParen:
  case AsmToken::Dot:
  case AsmToken::Minus:
  case AsmToken::Plus:
  case AsmToken::Exclaim:
  case AsmToken::Tilde:
  case AsmToken::Integer:
  case AsmToken::String:
  case AsmToken::Identifier:
  case AsmToken::Percent:
    if (getParser().parseExpression(Res))
      return MatchOperand_ParseFail;
    break;
  }

  Operands.push_back(DLXOperand::createImm(Res, S, E));
  return MatchOperand_Success;
}

OperandMatchResultTy
DLXsmParser::parseMemOpBaseReg(OperandVector &Operands) {
  if (getLexer().isNot(AsmToken::LParen)) {
    Error(getLoc(), "expected '('");
    return MatchOperand_ParseFail;
  }

  getParser().Lex(); // Eat '('

  if (ParseRegister(Operands) != MatchOperand_Success) {
    Error(getLoc(), "expected register");
    return MatchOperand_ParseFail;
  }

  if (getLexer().isNot(AsmToken::RParen)) {
    Error(getLoc(), "expected ')'");
    return MatchOperand_ParseFail;
  }

  getParser().Lex(); // Eat ')'

  return MatchOperand_Success;
}

// Error handling
void DLXAsmParser::Error(SMLoc IDLoc, const Twine &Message) {
  Parser.printError(IDLoc, Message);
}

// Main entry point for parsing an instruction
bool DLXAsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name, SMLoc NameLoc, OperandVector &Operands) {
  // First operand is token for instruction
  Operands.push_back(DLXOperand::createToken(Name, NameLoc));

  // If there are no more operands, then finish
  if (getLexer().is(AsmToken::EndOfStatement))
    return false;

  // Parse first operand
  if (parseOperand(Operands, Name))
    return true;

  // Parse until end of statement, consuming commas between operands
  unsigned OperandIdx = 1;
  while (getLexer().is(AsmToken::Comma)) {
    // Consume comma token
    getLexer().Lex();

    // Parse next operand
    if (parseOperand(Operands, Name))
      return true;

    ++OperandIdx;
  }

  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLexer().getLoc();
    getParser().eatToEndOfStatement();
    return Error(Loc, "unexpected token");
  }

  getParser().Lex(); // Consume the EndOfStatement.
  return false;
}

// Register the AsmParser with the target
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeDLXAsmParser() {
  RegisterMCAsmParser<DLXAsmParser> X(getTheDLXTarget());
}

