//== DLXInstrInfo.td - Target Description for DLX Target -*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the DLX implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

class ImmAsmOperand<string prefix, int width, string suffix> : AsmOperandClass {
  let Name = prefix # "Imm" # width # suffix;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
}

class SImmAsmOperand<int width, string suffix = "">
    : ImmAsmOperand<"S", width, suffix> {
}

class UImmAsmOperand<int width, string suffix = "">
    : ImmAsmOperand<"U", width, suffix> {
}

def simm16 : Operand<i32>, ImmLeaf<i32, [{return isInt<16>(Imm);}]> {
  let ParserMatchClass = SImmAsmOperand<16>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<16>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<16>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_SIMM16";
  let OperandNamespace = "DLXOp";
}

def brtarget : Operand<OtherVT> {
  let ParserMatchClass = SImmAsmOperand<16>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<16>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<16>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_BRTARGET";
  let OperandNamespace = "DLXOp";
}

def jmptarget : Operand<OtherVT> {
  let ParserMatchClass = SImmAsmOperand<26>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<26>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<26>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_JMPTARGET";
  let OperandNamespace = "DLXOp";
}

def uimm16 : Operand<i32>, ImmLeaf<i32, [{return isUInt<16>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<16>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<16>";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<16>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_UIMM16";
  let OperandNamespace = "DLXOp";
}

def i32imm_li : Operand<i32>, ImmLeaf<i32, [{return isInt<32>(Imm);}]> {
  let ParserMatchClass = SImmAsmOperand<32>;
}

def u32imm_li : Operand<i32>, ImmLeaf<i32, [{return isUInt<32>(Imm);}]> {
  let ParserMatchClass = UImmAsmOperand<32>;
}

def AddrFI : ComplexPattern<iPTR, 1, "SelectAddrFI", [frameindex], []>;

// A bare symbol used in call/tail only.
def CallSymbol : AsmOperandClass {
  let Name = "CallSymbol";
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidCallSymbol";
  let ParserMethod = "parseCallSymbol";
}
def call_symbol : Operand<i32> {
  let ParserMatchClass = CallSymbol;
}

def SDT_RISCVCall     : SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>;


// Procedure return
def dlx_ret : SDNode<"DLXISD::Ret", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Procedure calling
def SDT_CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>,
                                       SDTCisVT<1, i32>]>;
def SDT_CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>,
                                    SDTCisVT<1, i32>]>;

// These are target-independent nodes, but have target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_CallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_CallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

include "DLXInstrFormats.td"

let Defs = [R2], Uses = [R2] in {
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP   : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              [(callseq_end timm:$amt1, timm:$amt2)]>;
} // end Defs = [R2], Uses = [R2]

/*
  _____     _______               
 |  __ \   |__   __|              
 | |__) |_____| |_   _ _ __   ___ 
 |  _  /______| | | | | '_ \ / _ \
 | | \ \      | | |_| | |_) |  __/
 |_|  \_\     |_|\__, | .__/ \___|
                  __/ | |         
                 |___/|_|         
*/

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class ALU_rr<DLXFunc func11, string opcodestr>
    : DLXInstR<func11, OPC_R, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
              opcodestr, "$rd, $rs1, $rs2">;

/* R-Type definitions */
def SLL  : ALU_rr<FUNC_SLL, "sll">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SRL  : ALU_rr<FUNC_SRL, "srl">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SRA  : ALU_rr<FUNC_SRA, "sra">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def ADD  : ALU_rr<FUNC_ADD, "add">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def ADDU : ALU_rr<FUNC_ADDU,"addu">,Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SUB  : ALU_rr<FUNC_SUB, "sub">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SUBU : ALU_rr<FUNC_SUBU,"subu">,Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def AND  : ALU_rr<FUNC_AND, "and">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def OR   : ALU_rr<FUNC_OR,  "or">,  Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def XOR  : ALU_rr<FUNC_XOR, "xor">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SEQ  : ALU_rr<FUNC_SEQ, "seq">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SNE  : ALU_rr<FUNC_SNE, "sne">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SLT  : ALU_rr<FUNC_SLT, "slt">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SGT  : ALU_rr<FUNC_SGT, "sgt">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SLE  : ALU_rr<FUNC_SLE, "sle">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SGE  : ALU_rr<FUNC_SGE, "sge">, Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SLTU : ALU_rr<FUNC_SLTU,"sltu">,Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SGTU : ALU_rr<FUNC_SGTU,"sgtu">,Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SLEU : ALU_rr<FUNC_SLEU,"sleu">,Sched<[WriteIALU, ReadIALU, ReadIALU]>;
def SGEU : ALU_rr<FUNC_SGEU,"sgeu">,Sched<[WriteIALU, ReadIALU, ReadIALU]>;

/* Patterns for Instruction Selection */
// Available pattern operators are found in TargetSelectionDAG.td
class PatGprGpr<SDPatternOperator OpNode, DLXInst Inst>
    : Pat<(OpNode GPR:$rs1, GPR:$rs2), (Inst GPR:$rs1, GPR:$rs2)>;

def : PatGprGpr<shl, SLL>;
def : PatGprGpr<srl, SRL>;
def : PatGprGpr<sra, SRA>;
def : PatGprGpr<add, ADD>;
def : PatGprGpr<add, ADDU>; //? not clear how to pattern uadd
def : PatGprGpr<sub, SUB>;
def : PatGprGpr<sub, SUBU>;
def : PatGprGpr<and, AND>;
def : PatGprGpr<or,  OR>;
def : PatGprGpr<xor, XOR>;
def : PatGprGpr<seteq, SEQ>;
def : PatGprGpr<setne, SNE>;
def : PatGprGpr<setlt, SLT>;
def : PatGprGpr<setgt, SGT>;
def : PatGprGpr<setle, SLE>;
def : PatGprGpr<setge, SGE>;
def : PatGprGpr<setult,SLTU>;
def : PatGprGpr<setugt,SGTU>;
def : PatGprGpr<setule,SLEU>;
def : PatGprGpr<setuge,SGEU>;


/*  
  _____   _______               
 |_   _| |__   __|              
   | |______| |_   _ _ __   ___ 
   | |______| | | | | '_ \ / _ \
  _| |_     | | |_| | |_) |  __/
 |_____|    |_|\__, | .__/ \___|
                __/ | |         
               |___/|_|         
*/

/* Arithmetic and logic I-Type class */
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class ALU_ri<DLXOpcode opcode, string opcodestr>
    : DLXInstI<opcode, (outs GPR:$rd), (ins GPR:$rs1, simm16:$imm16), opcodestr, "$rd, $rs1, $imm16">,
      Sched<[WriteIALU, ReadIALU]>;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class ALU_i<DLXOpcode opcode, string opcodestr>
    : DLXInstI<opcode, (outs GPR:$rd), (ins simm16:$imm16), opcodestr, "$rd, $imm16">,
      Sched<[WriteIALU, ReadIALU]>;


// Arithmetics
// -----------
def ADDI  : ALU_ri<OPC_ADDI, "addi">;
def ADDUI : ALU_ri<OPC_ADDUI,"addui">;
def SUBI  : ALU_ri<OPC_SUBI, "subi">;
def SUBUI : ALU_ri<OPC_SUBUI,"subui">;
def ANDI  : ALU_ri<OPC_ANDI, "andi">;
def ORI   : ALU_ri<OPC_ORI,  "ori">;
def XORI  : ALU_ri<OPC_XORI, "xori">;
def SLLI  : ALU_ri<OPC_SLLI, "slli">;
def NOP   : ALU_ri<OPC_NOP,  "nop">;
def SRLI  : ALU_ri<OPC_SRLI, "srli">;
def SRAI  : ALU_ri<OPC_SRAI, "srai">;
def SEQI  : ALU_ri<OPC_SEQI, "seqi">;
def SNEI  : ALU_ri<OPC_SNEI, "snei">;
def SLTI  : ALU_ri<OPC_SLTI, "slti">;
def SGTI  : ALU_ri<OPC_SGTI, "sgti">;
def SLEI  : ALU_ri<OPC_SLEI, "slei">;
def SGEI  : ALU_ri<OPC_SGEI, "sgei">;
def SLTUI : ALU_ri<OPC_SLTUI,"sltui">;
def SGTUI : ALU_ri<OPC_SGTUI,"sgtui">;
def SLEUI : ALU_ri<OPC_SLEUI,"sleui">;
def SGEUI : ALU_ri<OPC_SGEUI,"sgeui">;

def LHI   : ALU_i<OPC_LHI,  "lhi">;

/* Patterns I-Type Arithmetics */
// Available pattern operators are found in TargetSelectionDAG.td
class PatGprSimm16<SDPatternOperator OpNode, DLXInstI Inst>
    : Pat<(OpNode GPR:$rs1, simm16:$imm16), (Inst GPR:$rs1, simm16:$imm16)>;
class PatGprUimm16<SDPatternOperator OpNode, DLXInstI Inst>
    : Pat<(OpNode GPR:$rs1, uimm16:$imm16), (Inst GPR:$rs1, uimm16:$imm16)>;

def : PatGprSimm16<add, ADDI>;
def : PatGprUimm16<add, ADDUI>;
def : PatGprSimm16<sub, SUBI>;
def : PatGprUimm16<sub, SUBUI>;
def : PatGprSimm16<and, ANDI>;
def : PatGprSimm16<or,  ORI>;
def : PatGprSimm16<xor, XORI>;
def : PatGprSimm16<shl, SLLI>;
def : PatGprSimm16<srl, SRLI>;
def : PatGprSimm16<sra, SRAI>;

// SETCC Patterns
def : PatGprSimm16<seteq, SEQI>;
def : PatGprSimm16<setne, SNEI>;
def : PatGprSimm16<setlt, SLTI>;
def : PatGprSimm16<setgt, SGTI>;
def : PatGprSimm16<setle, SLEI>;
def : PatGprSimm16<setge, SGEI>;
def : PatGprUimm16<setult,SLTUI>;
def : PatGprUimm16<setugt,SGTUI>;
def : PatGprUimm16<setule,SLEUI>;
def : PatGprUimm16<setuge,SGEUI>;

// Load a constant into a register
def : Pat<(i32 i32imm_li:$imm16), (ADDI (LHI uimm16:$imm16), uimm16:$imm16)>;

// Load and store
// --------------
let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class Load_ri<DLXOpcode opcode, string opcodestr>
    : DLXInstI<opcode, (outs GPR:$rd), (ins GPR:$rs1, simm16:$imm16), opcodestr, "$rd, ${imm16}(${rs1})">;

def LB  : Load_ri<OPC_LB,  "lb">, Sched<[WriteLDB, ReadMemBase]>;
def LH  : Load_ri<OPC_LH,  "lh">, Sched<[WriteLDH, ReadMemBase]>;
def LW  : Load_ri<OPC_LW,  "lw">, Sched<[WriteLDW, ReadMemBase]>;
def LBU : Load_ri<OPC_LBU, "lbu">, Sched<[WriteLDB, ReadMemBase]>;
def LHU : Load_ri<OPC_LHU, "lhu">, Sched<[WriteLDH, ReadMemBase]>;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
class Store_rri<DLXOpcode opcode, string opcodestr>
    : DLXInstI<opcode, (outs), (ins GPR:$rs2, GPR:$rs1, simm16:$imm16), opcodestr, "${imm16}(${rs1}), $rs2">;

def SB  : Store_rri<OPC_SB,  "sb">, Sched<[WriteSTB, ReadStoreData, ReadMemBase]>;
def SH  : Store_rri<OPC_SH,  "sh">, Sched<[WriteSTH, ReadStoreData, ReadMemBase]>;
def SW  : Store_rri<OPC_SW,  "sw">, Sched<[WriteSTW, ReadStoreData, ReadMemBase]>;

multiclass LdPat<PatFrag LoadOp, DLXInstI Inst> {
  def : Pat<(LoadOp GPR:$rs1), (Inst GPR:$rs1, 0)>;
  def : Pat<(LoadOp AddrFI:$rs1), (Inst AddrFI:$rs1, 0)>;
  def : Pat<(LoadOp (add GPR:$rs1, simm16:$imm16)),
            (Inst GPR:$rs1, simm16:$imm16)>;
  def : Pat<(LoadOp (add AddrFI:$rs1, simm16:$imm16)),
            (Inst AddrFI:$rs1, simm16:$imm16)>;
}

defm : LdPat<sextloadi8, LB>;
defm : LdPat<extloadi8, LB>;
defm : LdPat<sextloadi16, LH>;
defm : LdPat<extloadi16, LH>;
defm : LdPat<load, LW>;
defm : LdPat<zextloadi8, LBU>;
defm : LdPat<zextloadi16, LHU>;

/// Stores

multiclass StPat<PatFrag StoreOp, DLXInstI Inst> {
  def : Pat<(StoreOp GPR:$rs2, GPR:$rs1), (Inst GPR:$rs2, GPR:$rs1, 0)>;
  def : Pat<(StoreOp GPR:$rs2, AddrFI:$rs1), (Inst GPR:$rs2, AddrFI:$rs1, 0)>;
  def : Pat<(StoreOp GPR:$rs2, (add GPR:$rs1, simm16:$imm16)),
            (Inst GPR:$rs2, GPR:$rs1, simm16:$imm16)>;
  def : Pat<(StoreOp GPR:$rs2, (add AddrFI:$rs1, simm16:$imm16)),
            (Inst GPR:$rs2, AddrFI:$rs1, simm16:$imm16)>;
}

defm : StPat<truncstorei8, SB>;
defm : StPat<truncstorei16, SH>;
defm : StPat<store, SW>;

// Branches
// --------
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class BranchCC_ri<DLXOpcode Opcode, string opcodestr>
    : DLXInstI<Opcode, (outs),
              (ins GPR:$rs1, brtarget:$imm16),
              opcodestr, "$rs1, $imm16">,
      Sched<[WriteJmp]> {
  let isBranch = 1;
  let isTerminator = 1;
}
def BEQZ : BranchCC_ri<OPC_BEQZ, "beqz">;
def BNEZ : BranchCC_ri<OPC_BNEZ, "bnez">;

def : Pat<(brcond (i32 (seteq GPR:$rs1, 0)), bb:$imm16), (BEQZ GPR:$rs1, brtarget:$imm16)>;
def : Pat<(brcond (i32 (setne GPR:$rs1, 0)), bb:$imm16), (BNEZ GPR:$rs1, brtarget:$imm16)>;

class BRccPat<PatFrag CondCode, DLXInstR Inst>
    : Pat<(brcond (CondCode GPR:$rs1, GPR:$rs2), bb:$imm16),
            (BNEZ (Inst GPR:$rs1, GPR:$rs2), brtarget:$imm16)>;
  
def : BRccPat<setlt, SLT>;
def : BRccPat<setgt, SGT>;
def : BRccPat<setle, SLE>;
def : BRccPat<setge, SGE>;
def : BRccPat<setult,SLTU>;
def : BRccPat<setugt,SGTU>;
def : BRccPat<setule,SLEU>;
def : BRccPat<setuge,SGEU>;

// An extra pattern is needed for a brcond without a setcc (i.e. where the
// condition was calculated elsewhere).
def : Pat<(brcond GPR:$cond, bb:$imm16), (BNEZ GPR:$cond, brtarget:$imm16)>;

/* 
       _     _______               
      | |   |__   __|              
      | |______| |_   _ _ __   ___ 
  _   | |______| | | | | '_ \ / _ \
 | |__| |      | | |_| | |_) |  __/
  \____/       |_|\__, | .__/ \___|
                   __/ | |         
                  |___/|_|         

*/

def JR  : DLXInstJ<OPC_JALR, (outs ), (ins GPR:$rs1), "jr", "$rs1">, 
    Sched<[ReadJalr]>;

// This is for calling functions
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
let isCall = 1 in
def JALR : DLXInstJ<OPC_JALR, (outs GPR:$r31), (ins GPR:$rs1), "jalr", "$rs1">, 
    Sched<[ReadJalr]>;
}

// Return from function
let isReturn = 1, isBarrier = 1, isTerminator = 1 in
def PseudoRET : Pseudo<(outs), (ins), [(dlx_ret)]>,
                PseudoInstExpansion<(JR R31)>;
def : InstAlias<"ret",                   (JR      R31), 4>;

/* J-Type definitions */
def J    : DLXInstJ<OPC_J, (outs), (ins jmptarget:$imm26), "j", "$imm26">, 
    Sched<[]>;
def JAL  : DLXInstJ<OPC_JAL, (outs), (ins call_symbol:$imm26), "jal", "$imm26">, 
    Sched<[ReadJalr]>;

let isBarrier = 1, isBranch = 1, isTerminator = 1 in
def PseudoBR : Pseudo<(outs), (ins jmptarget:$imm26), [(br bb:$imm26)]>,
               PseudoInstExpansion<(J jmptarget:$imm26)>;

def dlx_call      : SDNode<"DLXISD::Call", SDT_RISCVCall,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

let isCall = 1, Defs = [R31], isCodeGenOnly = 0 in
def PseudoCALL : Pseudo<(outs), (ins call_symbol:$imm26), []>,
                 PseudoInstExpansion<(JAL call_symbol:$imm26)>;

def : Pat<(dlx_call tglobaladdr:$imm26), (PseudoCALL tglobaladdr:$imm26)>;
def : Pat<(dlx_call texternalsym:$imm26), (PseudoCALL texternalsym:$imm26)>;