#include "DLX.h"
#include "DLXRegisterInfo.h"
#include "MCTargetDesc/DLXMCExpr.h"
#include "TargetInfo/DLXTargetInfo.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;

namespace {


class DLXAsmParser : public MCTargetAsmParser {
#define GET_ASSEMBLER_HEADER
#include "DLXGenAsmMatcher.inc"

public:
  // Constructor
  DLXAsmParser(const MCSubtargetInfo &sti, MCAsmParser &parser, const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, sti, MII), MII(MII), Parser(parser) {
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
  bool ParseMemOpBaseReg(OperandVector &Operands);

  bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) override;
  bool ParseDirective(AsmToken DirectiveID) override;

  OperandMatchResultTy parseCallSymbol(OperandVector &Operands);
  SMLoc getLoc() const { return getParser().getTok().getLoc(); }

  // Error handling utility
  void Error(SMLoc IDLoc, const Twine &Message);

  // Additional utility functions if needed
  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name, SMLoc NameLoc, OperandVector &Operands) override;

public:
enum DLXMatchResultTy {
    Match_Dummy = FIRST_TARGET_MATCH_RESULT_TY,
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "DLXGenAsmMatcher.inc"
#undef GET_OPERAND_DIAGNOSTIC_TYPES
  };
  static bool classifySymbolRef(const MCExpr *Expr,
                                DLXMCExpr::VariantKind &Kind,
                                int64_t &Addend);


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
           (VK == DLXMCExpr::VK_DLX_CALL);
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
                       VK == DLXMCExpr::VK_DLX_LO);
  }

  bool isUImm16() const {
    DLXMCExpr::VariantKind VK = DLXMCExpr::VK_DLX_None;
    int64_t Imm;
    bool IsValid;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    if (!IsConstantImm)
      IsValid = DLXAsmParser::classifySymbolRef(getImm(), VK, Imm);
    else
      IsValid = isUInt<16>(Imm);
    return IsValid && ((IsConstantImm && VK == DLXMCExpr::VK_DLX_None) ||
                       VK == DLXMCExpr::VK_DLX_LO);
  }

  bool isUImm32() const {
    DLXMCExpr::VariantKind VK = DLXMCExpr::VK_DLX_None;
    int64_t Imm;
    bool IsValid;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    if (!IsConstantImm)
      IsValid = DLXAsmParser::classifySymbolRef(getImm(), VK, Imm);
    else
      IsValid = isUInt<32>(Imm);
    return IsValid && ((IsConstantImm && VK == DLXMCExpr::VK_DLX_None) ||
                       VK == DLXMCExpr::VK_DLX_LO);
  }

  bool isSImm32() const {
    DLXMCExpr::VariantKind VK = DLXMCExpr::VK_DLX_None;
    int64_t Imm;
    bool IsValid;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    if (!IsConstantImm)
      IsValid = DLXAsmParser::classifySymbolRef(getImm(), VK, Imm);
    else
      IsValid = isInt<32>(Imm);
    return IsValid && ((IsConstantImm && VK == DLXMCExpr::VK_DLX_None) ||
                       VK == DLXMCExpr::VK_DLX_LO);
  }

  bool isSImm26() const {
    DLXMCExpr::VariantKind VK = DLXMCExpr::VK_DLX_None;
    int64_t Imm;
    bool IsValid;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    if (!IsConstantImm)
      IsValid = DLXAsmParser::classifySymbolRef(getImm(), VK, Imm);
    else
      IsValid = isInt<26>(Imm);
    return IsValid && ((IsConstantImm && VK == DLXMCExpr::VK_DLX_None) ||
                       VK == DLXMCExpr::VK_DLX_CALL);
  }

  /// getStartLoc - Gets location of the first token of this operand
  SMLoc getStartLoc() const override { return StartLoc; }
  /// getEndLoc - Gets location of the last token of this operand
  SMLoc getEndLoc() const override { return EndLoc; }

  unsigned getReg() const override {
    assert(Kind == KindTy::Register && "Invalid type access!");
    return Reg.RegNum.id();
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

#define GET_REGISTER_MATCHER
#define GET_SUBTARGET_FEATURE_NAME
#define GET_MATCHER_IMPLEMENTATION
#define GET_MNEMONIC_SPELL_CHECKER
#include "DLXGenAsmMatcher.inc"

bool DLXAsmParser::classifySymbolRef(const MCExpr *Expr,
                                       DLXMCExpr::VariantKind &Kind,
                                       int64_t &Addend) {
  Kind = DLXMCExpr::VK_DLX_None;
  Addend = 0;

  if (const DLXMCExpr *RE = dyn_cast<DLXMCExpr>(Expr)) {
    Kind = RE->getKind();
    Expr = RE->getSubExpr();
  }

  // It's a simple symbol reference or constant with no addend.
  if (isa<MCConstantExpr>(Expr) || isa<MCSymbolRefExpr>(Expr))
    return true;

  const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(Expr);
  if (!BE)
    return false;

  if (!isa<MCSymbolRefExpr>(BE->getLHS()))
    return false;

  if (BE->getOpcode() != MCBinaryExpr::Add &&
      BE->getOpcode() != MCBinaryExpr::Sub)
    return false;

  // We are able to support the subtraction of two symbol references
  if (BE->getOpcode() == MCBinaryExpr::Sub &&
      isa<MCSymbolRefExpr>(BE->getRHS()))
    return true;

  // See if the addend is a constant, otherwise there's more going
  // on here than we can deal with.
  auto AddendExpr = dyn_cast<MCConstantExpr>(BE->getRHS());
  if (!AddendExpr)
    return false;

  Addend = AddendExpr->getValue();
  if (BE->getOpcode() == MCBinaryExpr::Sub)
    Addend = -Addend;

  // It's some symbol reference + a constant addend
  return Kind != DLXMCExpr::VK_DLX_Invalid;
}


OperandMatchResultTy
DLXAsmParser::parseCallSymbol(OperandVector &Operands) {
  SMLoc S = getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);
  const MCExpr *Res;

  if (getLexer().getKind() != AsmToken::Identifier)
    return MatchOperand_NoMatch;

  // Avoid parsing the register in `call rd, foo` as a call symbol.
  if (getLexer().peekTok().getKind() != AsmToken::EndOfStatement)
    return MatchOperand_NoMatch;

  StringRef Identifier;
  if (getParser().parseIdentifier(Identifier))
    return MatchOperand_ParseFail;

  DLXMCExpr::VariantKind Kind = DLXMCExpr::VK_DLX_CALL;
  if (Identifier.consume_back("@plt"))
    llvm_unreachable("No PLT!!");

  MCSymbol *Sym = getContext().getOrCreateSymbol(Identifier);
  Res = MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, getContext());
  Res = DLXMCExpr::create(Res, Kind, getContext());
  Operands.push_back(DLXOperand::createImm(Res, S, E));
  return MatchOperand_Success;
}

// Implementation of MatchAndEmitInstruction
bool DLXAsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode, OperandVector &Operands, MCStreamer &Out, uint64_t &ErrorInfo, bool MatchingInlineAsm) {
  // Utilize TableGen generated code to match instructions
  MCInst Inst;
  long unsigned err_info;
  auto MatchResult = MatchInstructionImpl(Operands, Inst, err_info, MatchingInlineAsm);

  outs() << "DLXAsmParser::MatchAndEmitInstruction: " << Opcode << " " << MatchResult << "\n";

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
  if (ParseImmediate(Operands) == MatchOperand_Success) {
    // Parse memory base register if present
    if (getLexer().is(AsmToken::LParen))
      return ParseMemOpBaseReg(Operands) != MatchOperand_Success;
    return false;
  }

  // Finally we have exhausted all options and must declare defeat.
  Error(getLoc(), "unknown operand");
  return true;
}

bool
DLXAsmParser::ParseMemOpBaseReg(OperandVector &Operands) {
  if (getLexer().isNot(AsmToken::LParen)) {
    Error(getLoc(), "expected '('");
    return MatchOperand_ParseFail;
  }

  getParser().Lex(); // Eat '('
  Operands.push_back(DLXOperand::createToken("(", getLoc()));

  if (ParseRegister(Operands) != MatchOperand_Success) {
    Error(getLoc(), "expected register");
    return MatchOperand_ParseFail;
  }

  if (getLexer().isNot(AsmToken::RParen)) {
    Error(getLoc(), "expected ')'");
    return MatchOperand_ParseFail;
  }

  getParser().Lex(); // Eat ')'
  Operands.push_back(DLXOperand::createToken(")", getLoc()));

  return MatchOperand_Success;
}

// Attempts to match Name as a register (either using the default name or
// alternative ABI names), setting RegNo to the matching register. Upon
// failure, returns true and sets RegNo to 0. If IsRV32E then registers
// x16-x31 will be rejected.
static bool matchRegisterNameHelper(Register &RegNo,
                                    StringRef Name) {
  RegNo = MatchRegisterName(Name);

  if (RegNo == DLX::NoRegister)
    RegNo = MatchRegisterAltName(Name);

  return RegNo == DLX::NoRegister;
}

// Implementation for specific operand types
bool
DLXAsmParser::ParseRegister(OperandVector &Operands) {
  SMLoc FirstS = getLoc();
  AsmToken LParen;

  switch (getLexer().getKind()) {
  default:
    return MatchOperand_NoMatch;
  case AsmToken::Identifier:
    StringRef Name = getLexer().getTok().getIdentifier();
    Register RegNo;
    matchRegisterNameHelper(RegNo, Name);

    if (RegNo == DLX::NoRegister) {
      return MatchOperand_NoMatch;
    }
    SMLoc S = getLoc();
    SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);
    getLexer().Lex();
    Operands.push_back(DLXOperand::createReg(RegNo, S, E));
  }

  return MatchOperand_Success;
}


bool DLXAsmParser::ParseRegister(unsigned &RegNo, SMLoc &StartLoc,
                                  SMLoc &EndLoc) {
  const AsmToken &Tok = getParser().getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  RegNo = 0;
  StringRef Name = getLexer().getTok().getIdentifier();

  if (matchRegisterNameHelper((Register&)RegNo, Name))
    Error(StartLoc, "invalid register name");

  getParser().Lex(); // Eat identifier token.
  return false;
}

bool DLXAsmParser::ParseDirective(AsmToken DirectiveID) {
  DirectiveID.dump(llvm::errs());
  llvm_unreachable("No directives supported yet!");

  return true;
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
  if (ParseOperand(Operands, Name))
    return true;

  // Parse until end of statement, consuming commas between operands
  unsigned OperandIdx = 1;
  while (getLexer().is(AsmToken::Comma)) {
    // Consume comma token
    getLexer().Lex();

    // Parse next operand
    if (ParseOperand(Operands, Name))
      return true;

    ++OperandIdx;
  }

  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLexer().getLoc();
    getParser().eatToEndOfStatement();
    Error(Loc, "unexpected token");
    llvm_unreachable("Error");
    return false;
  }

  getParser().Lex(); // Consume the EndOfStatement.
  return false;
}

// Register the AsmParser with the target
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeDLXAsmParser() {
  RegisterMCAsmParser<DLXAsmParser> X(getTheDLXTarget());
}

