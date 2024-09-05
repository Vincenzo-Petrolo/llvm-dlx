//===-- DLXMCCodeEmitter.cpp - Convert DLX code to machine code -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the DLXMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/DLXFixupKinds.h"
#include "MCTargetDesc/DLXMCExpr.h"
#include "MCTargetDesc/DLXMCTargetDesc.h"
#include "Utils/DLXBaseInfo.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

static inline unsigned getDLXRegisterNumbering(unsigned Reg) {
  switch(Reg) {
    case DLX::R0  : return 0;
    case DLX::R1  : return 1;
    case DLX::R2  : return 2;
    case DLX::R3  : return 3;
    case DLX::R4  : return 4;
    case DLX::R5  : return 5;
    case DLX::R6  : return 6;
    case DLX::R7  : return 7;
    case DLX::R8  : return 8;
    case DLX::R9  : return 9;
    case DLX::R10 : return 10;
    case DLX::R11 : return 11;
    case DLX::R12 : return 12;
    case DLX::R13 : return 13;
    case DLX::R14 : return 14;
    case DLX::R15 : return 15;
    case DLX::R16 : return 16;
    case DLX::R17 : return 17;
    case DLX::R18 : return 18;
    case DLX::R19 : return 19;
    case DLX::R20 : return 20;
    case DLX::R21 : return 21;
    case DLX::R22 : return 22;
    case DLX::R23 : return 23;
    case DLX::R24 : return 24;
    case DLX::R25 : return 25;
    case DLX::R26 : return 26;
    case DLX::R27 : return 27;
    case DLX::R28 : return 28;
    case DLX::R29 : return 29;
    case DLX::R30 : return 30;
    case DLX::R31 : return 31;
    default: llvm_unreachable("Unknown register number");
  }
}

namespace {
class DLXMCCodeEmitter : public MCCodeEmitter {
  DLXMCCodeEmitter(const DLXMCCodeEmitter &) = delete;
  void operator=(const DLXMCCodeEmitter &) = delete;
  MCContext &Ctx;
  MCInstrInfo const &MCII;

public:
  DLXMCCodeEmitter(MCContext &ctx, MCInstrInfo const &MCII)
      : Ctx(ctx), MCII(MCII) {}

  ~DLXMCCodeEmitter() override {}

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  /// TableGen'erated function for getting the binary encoding for an
  /// instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// Return binary encoding of operand. If the machine operand requires
  /// relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImmOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const;
};
} // end anonymous namespace

void DLXMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  // Get byte count of instruction.
  unsigned Size = Desc.getSize();

  switch (Size) {
  default:
    llvm_unreachable("Unhandled encodeInstruction length!");
  case 4: {
    uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write(OS, Bits, support::little);
    break;
  }
  }
}

// Emit one byte through output stream (from MCBlazeMCCodeEmitter)
void EmitByte(unsigned char C, unsigned &CurByte, raw_ostream &OS) const {
  OS << (char)C;
  ++CurByte;
}

// Emit a series of bytes (from MCBlazeMCCodeEmitter)
void EmitConstant(uint64_t Val, unsigned Size, unsigned &CurByte,
                  raw_ostream &OS) const {
  assert(Size <= 8 && "size too big in emit constant");

  for (unsigned i = 0; i != Size; ++i) {
    EmitByte(Val & 255, CurByte, OS);
    Val >>= 8;
  }
}

unsigned
DLXMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {

  if (MO.isReg())
  return getDLXRegisterNumbering(MO.getReg());

  if (MO.isImm())
  return static_cast<unsigned>(MO.getImm());

  // MO must be an expression
  assert(MO.isExpr());

  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();

  if (Kind == MCExpr::Binary) {
      Expr = static_cast<const MCBinaryExpr*>(Expr)->getLHS();
      Kind = Expr->getKind();
  }

  assert (Kind == MCExpr::SymbolRef);


  DLX::Fixups FixupKind = DLX::Fixups(0);

  switch(cast<MCSymbolRefExpr>(Expr)->getKind()) {
      default: llvm_unreachable("Unknown fixup kind!");
      break;
      case DLX::VK_DLX_LO:
          FixupKind = DLX::fixup_DLX_LO16;
          break;
      case DLX::VK_DLX_HI:
          FixupKind = DLX::fixup_DLX_HI16;
          break;
      case DLX::VK_DLX_CALL:
          FixupKind = DLX::fixup_DLX_JAL_PC26;
          break;
  }

  // Push fixup (all info is contained within)
  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));
  return 0;
}

void DLXMCCodeEmitter::
EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups) const {
  // Keep track of the current byte being emitted
  unsigned CurByte = 0;
  // Get instruction encoding and emit it
  ++MCNumEmitted;       // Keep track of the number of emitted insns.
  unsigned Value = getBinaryCodeForInstr(MI);
  EmitConstant(Value, 4, CurByte, OS);
}

#include "DLXGenMCCodeEmitter.inc"

