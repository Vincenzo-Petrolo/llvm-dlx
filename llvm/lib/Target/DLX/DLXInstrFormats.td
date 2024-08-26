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
def InstFormatR4     : InstFormat<2>;
def InstFormatI      : InstFormat<3>;
def InstFormatS      : InstFormat<4>;
def InstFormatB      : InstFormat<5>;
def InstFormatU      : InstFormat<6>;
def InstFormatJ      : InstFormat<7>;
def InstFormatCR     : InstFormat<8>;
def InstFormatCI     : InstFormat<9>;
def InstFormatCSS    : InstFormat<10>;
def InstFormatCIW    : InstFormat<11>;
def InstFormatCL     : InstFormat<12>;
def InstFormatCS     : InstFormat<13>;
def InstFormatCA     : InstFormat<14>;
def InstFormatCB     : InstFormat<15>;
def InstFormatCJ     : InstFormat<16>;
def InstFormatOther  : InstFormat<17>;

// The following opcode names match those given in Table 19.1 in the
// RISC-V User-level ISA specification ("RISC-V base opcode map").
class DLXOpcode<bits<7> val> {
  bits<7> Value = val;
}
def OPC_LOAD      : DLXOpcode<0b0000011>;
def OPC_LOAD_FP   : DLXOpcode<0b0000111>;
def OPC_MISC_MEM  : DLXOpcode<0b0001111>;
def OPC_OP_IMM    : DLXOpcode<0b0010011>;
def OPC_AUIPC     : DLXOpcode<0b0010111>;
def OPC_OP_IMM_32 : DLXOpcode<0b0011011>;
def OPC_STORE     : DLXOpcode<0b0100011>;
def OPC_STORE_FP  : DLXOpcode<0b0100111>;
def OPC_AMO       : DLXOpcode<0b0101111>;
def OPC_OP        : DLXOpcode<0b0110011>;
def OPC_LUI       : DLXOpcode<0b0110111>;
def OPC_OP_32     : DLXOpcode<0b0111011>;
def OPC_MADD      : DLXOpcode<0b1000011>;
def OPC_MSUB      : DLXOpcode<0b1000111>;
def OPC_NMSUB     : DLXOpcode<0b1001011>;
def OPC_NMADD     : DLXOpcode<0b1001111>;
def OPC_OP_FP     : DLXOpcode<0b1010011>;
def OPC_BRANCH    : DLXOpcode<0b1100011>;
def OPC_JALR      : DLXOpcode<0b1100111>;
def OPC_JAL       : DLXOpcode<0b1101111>;
def OPC_SYSTEM    : DLXOpcode<0b1110011>;

class RWInst<dag outs, dag ins, string opcodestr, string argstr,
             list<dag> pattern, InstFormat format>
    : Instruction {
  field bits<32> Inst;
  // SoftFail is a field the disassembler can use to provide a way for
  // instructions to not match without killing the whole decode process. It is
  // mainly used for ARM, but Tablegen expects this field to exist or it fails
  // to build the decode table.
  field bits<32> SoftFail = 0;
  let Size = 4;

  bits<7> Opcode = 0;

  let Inst{6-0} = Opcode;

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
    : RWInst<outs, ins, opcodestr, argstr, pattern, InstFormatPseudo>,
      Sched<[]> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}

// Instruction formats are listed in the order they appear in the RISC-V
// instruction set manual (R, I, S, B, U, J) with sub-formats (e.g. RWInstR4,
// RWInstRAtomic) sorted alphabetically.

class RWInstR<bits<7> funct7, bits<3> funct3, DLXOpcode opcode, dag outs,
              dag ins, string opcodestr, string argstr>
    : RWInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> rs2;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = funct7;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class RWInstI<bits<3> funct3, DLXOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : RWInst<outs, ins, opcodestr, argstr, [], InstFormatI> {
  bits<12> imm12;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-20} = imm12;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}