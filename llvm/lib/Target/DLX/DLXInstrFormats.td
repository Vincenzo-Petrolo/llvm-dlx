//=== DLXInstrFormats.td - DLX Instruction Formats -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//
//  These instruction format definitions are structured to match the
//  description in the RISC-V User-Level ISA specification as closely as
//  possible. For instance, the specification describes instructions with the
//  MSB (31st bit) on the left and the LSB (0th bit) on the right. This is
//  reflected in the order of parameters to each instruction class.
//
//  One area of divergence is in the description of immediates. The
//  specification describes immediate encoding in terms of bit-slicing
//  operations on the logical value represented. The immediate argument to
//  these instruction formats instead represents the bit sequence that will be
//  inserted into the instruction. e.g. although JAL's immediate is logically
//  a 21-bit value (where the LSB is always zero), we describe it as an imm20
//  to match how it is encoded.
//
//===----------------------------------------------------------------------===//

// Format specifies the encoding used by the instruction. This is used by
// DLXMCCodeEmitter to determine which form of fixup to use. These
// definitions must be kept in-sync with DLXBaseInfo.h.
class InstFormat<bits<5> val> {
  bits<5> Value = val;
}
def InstFormatPseudo : InstFormat<0>;
def InstFormatR      : InstFormat<1>;
def InstFormatI      : InstFormat<2>;
def InstFormatJ      : InstFormat<3>;
def InstFormatJR     : InstFormat<4>;
def InstFormatB      : InstFormat<5>;
def InstFormatOther  : InstFormat<6>;


class DLXOpcode<bits<6> val> {
  bits<6> Value = val;
}
def OPC_R         : DLXOpcode<0x00>;
def OPC_J         : DLXOpcode<0x02>;
def OPC_JAL       : DLXOpcode<0x03>;
def OPC_BEQZ      : DLXOpcode<0x04>;
def OPC_BNEZ      : DLXOpcode<0x05>;
def OPC_ADDI      : DLXOpcode<0x08>;
def OPC_ADDUI     : DLXOpcode<0x09>;
def OPC_SUBI      : DLXOpcode<0x0A>;
def OPC_SUBUI     : DLXOpcode<0x0B>;
def OPC_ANDI      : DLXOpcode<0x0C>;
def OPC_ORI       : DLXOpcode<0x0D>;
def OPC_XORI      : DLXOpcode<0x0E>;
def OPC_LHI       : DLXOpcode<0x0F>;
def OPC_JR        : DLXOpcode<0x12>;
def OPC_JALR      : DLXOpcode<0x13>;
def OPC_SLLI      : DLXOpcode<0x14>;
def OPC_NOP       : DLXOpcode<0x15>;
def OPC_SRLI      : DLXOpcode<0x16>;
def OPC_SRAI      : DLXOpcode<0x17>;
def OPC_SEQI      : DLXOpcode<0x18>;
def OPC_SNEI      : DLXOpcode<0x19>;
def OPC_SLTI      : DLXOpcode<0x1A>;
def OPC_SGTI      : DLXOpcode<0x1B>;
def OPC_SLEI      : DLXOpcode<0x1C>;
def OPC_SGEI      : DLXOpcode<0x1D>;
def OPC_LB        : DLXOpcode<0x20>;
def OPC_LH        : DLXOpcode<0x21>;
def OPC_LW        : DLXOpcode<0x23>;
def OPC_LBU       : DLXOpcode<0x24>;
def OPC_LHU       : DLXOpcode<0x25>;
def OPC_SB        : DLXOpcode<0x28>;
def OPC_SH        : DLXOpcode<0x29>;
def OPC_SW        : DLXOpcode<0x2B>;
def OPC_SLTUI     : DLXOpcode<0x3A>;
def OPC_SGTUI     : DLXOpcode<0x3B>;
def OPC_SLEUI     : DLXOpcode<0x3C>;
def OPC_SGEUI     : DLXOpcode<0x3D>;

class DLXFunc<bits<11> val> {
  bits<11> Value = val;
}
def FUNC_SLL     : DLXFunc<0x04>;
def FUNC_SRL     : DLXFunc<0x06>;
def FUNC_SRA     : DLXFunc<0x07>;
def FUNC_ADD     : DLXFunc<0x20>;
def FUNC_ADDU    : DLXFunc<0x21>;
def FUNC_SUB     : DLXFunc<0x22>;
def FUNC_SUBU    : DLXFunc<0x23>;
def FUNC_AND     : DLXFunc<0x24>;
def FUNC_OR      : DLXFunc<0x25>;
def FUNC_XOR     : DLXFunc<0x26>;
def FUNC_SEQ     : DLXFunc<0x28>;
def FUNC_SNE     : DLXFunc<0x29>;
def FUNC_SLT     : DLXFunc<0x2A>;
def FUNC_SGT     : DLXFunc<0x2B>;
def FUNC_SLE     : DLXFunc<0x2C>;
def FUNC_SGE     : DLXFunc<0x2D>;
def FUNC_SLTU    : DLXFunc<0x3A>;
def FUNC_SGTU    : DLXFunc<0x3B>;
def FUNC_SLEU    : DLXFunc<0x3C>;
def FUNC_SGEU    : DLXFunc<0x3D>;

class DLXInst<dag outs, dag ins, string opcodestr, string argstr,
             list<dag> pattern, InstFormat format>
    : Instruction {
  field bits<32> Inst;
  // SoftFail is a field the disassembler can use to provide a way for
  // instructions to not match without killing the whole decode process. It is
  // mainly used for ARM, but Tablegen expects this field to exist or it fails
  // to build the decode table.
  field bits<32> SoftFail = 0;
  let Size = 4;

  bits<6> Opcode = 0;
  bits<11> Func = 0;

  let Inst{5-0} = Opcode;

  let Namespace = "DLX";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = opcodestr # "\t" # argstr;
  let Pattern = pattern;

  let TSFlags{4-0} = format.Value;
}

// Pseudo instructions
class Pseudo<dag outs, dag ins, list<dag> pattern, string opcodestr = "",
             string argstr = "">
    : DLXInst<outs, ins, opcodestr, argstr, pattern, InstFormatPseudo>,
      Sched<[]> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}

class DLXInstR<DLXFunc func, DLXOpcode opcode, dag outs,
              dag ins, string opcodestr, string argstr>
    : DLXInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> rs2;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-21} = func.Value;
  let Inst{20-16} = rd;
  let Inst{15-11} = rs2;
  let Inst{10-6} = rs1;

  let Func    = func.Value;
  let Opcode  = opcode.Value;
}

class DLXInstI<DLXOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : DLXInst<outs, ins, opcodestr, argstr, [], InstFormatI> {
  bits<16> imm16;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-16} = imm16;
  let Inst{15-11} = rd;
  let Inst{10-6} = rs1;
  let Opcode = opcode.Value;
}

class DLXInstB<DLXOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : DLXInst<outs, ins, opcodestr, argstr, [], InstFormatB> {
  bits<16> imm16;
  bits<5> rs1;

  let Inst{31-16} = imm16;
  let Inst{10-6} = rs1;
  let Opcode = opcode.Value;
}

class DLXInstJ<DLXOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : DLXInst<outs, ins, opcodestr, argstr, [], InstFormatJ> {
  bits<26> imm26;

  let Inst{31-6} = imm26;
  let Opcode = opcode.Value;
}

class DLXInstJR<DLXOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : DLXInst<outs, ins, opcodestr, argstr, [], InstFormatJR> {
  bits<5> rs1;

  let Inst{10-6} = rs1;
  let Opcode = opcode.Value;
}