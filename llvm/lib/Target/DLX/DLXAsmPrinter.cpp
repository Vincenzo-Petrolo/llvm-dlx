//===-- DLXAsmPrinter.cpp - DLX LLVM Assembly Printer -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format CPU0 assembly language.
//
//===----------------------------------------------------------------------===//

#include "DLXInstrInfo.h"
#include "DLXTargetMachine.h"
#include "MCTargetDesc/DLXMCExpr.h"
#include "MCTargetDesc/DLXInstPrinter.h"
#include "TargetInfo/DLXTargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "dlx-asm-printer"

namespace llvm {
class DLXAsmPrinter : public AsmPrinter {
public:
  explicit DLXAsmPrinter(TargetMachine &TM,
                           std::unique_ptr<MCStreamer> Streamer)
    : AsmPrinter(TM, std::move(Streamer)) {}

  virtual StringRef getPassName() const override {
    return "DLX Assembly Printer";
  }

  void EmitInstruction(const MachineInstr *MI) override;

  // This function must be present as it is internally used by the
  // auto-generated function emitPseudoExpansionLowering to expand pseudo
  // instruction
  void EmitToStreamer(MCStreamer &S, const MCInst &Inst);
  // Auto-generated function in DLXGenMCPseudoLowering.inc
  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                   const MachineInstr *MI);

private:
  void LowerInstruction(const MachineInstr *MI, MCInst &OutMI) const;
  bool lowerOperand(const MachineOperand& MO, MCOperand &MCOp) const;
  MCOperand LowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;
};
}

// Simple pseudo-instructions have their lowering (with expansion to real
// instructions) auto-generated.
#include "DLXGenMCPseudoLowering.inc"
void DLXAsmPrinter::EmitToStreamer(MCStreamer &S, const MCInst &Inst) {
  AsmPrinter::EmitToStreamer(*OutStreamer, Inst);
}

void DLXAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  // Do any auto-generated pseudo lowerings.
  if (emitPseudoExpansionLowering(*OutStreamer, MI)) {
    outs() << "Emitted pseudo expansion lowering\n";
    return;
  }

  MCInst TmpInst;
  LowerInstruction(MI, TmpInst);
  outs() << "DLXAsmPrinter::EmitInstruction: Emitting: ";
  TmpInst.dump_pretty(outs());
  outs() << "\n";

  EmitToStreamer(*OutStreamer, TmpInst);
}

void DLXAsmPrinter::LowerInstruction(const MachineInstr *MI,
                                       MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    lowerOperand(MO, MCOp);
    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}

bool DLXAsmPrinter::lowerOperand(const MachineOperand& MO, MCOperand &MCOp) const {
  outs() << "Lowering operand: ";
  MO.dump();
  outs() << "\n";
  outs() << "Operand type: " << int(MO.getType()) << "\n";
  switch (MO.getType()) {
    case MachineOperand::MO_Register:
      // Ignore all implicit register operands.
      if (MO.isImplicit()) {
        return false;
        break;
      }
      MCOp = MCOperand::createReg(MO.getReg());
      break;

    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;

    case MachineOperand::MO_MachineBasicBlock:
      MCOp = LowerSymbolOperand(MO, MO.getMBB()->getSymbol());
      break;

    case MachineOperand::MO_GlobalAddress:
      MCOp = LowerSymbolOperand(MO, getSymbol(MO.getGlobal()));
      break;

    case MachineOperand::MO_BlockAddress:
      MCOp = LowerSymbolOperand(MO, GetBlockAddressSymbol(MO.getBlockAddress()));
      break;

    case MachineOperand::MO_ExternalSymbol:
      MCOp = LowerSymbolOperand(MO, GetExternalSymbolSymbol(MO.getSymbolName()));
      break;

    case MachineOperand::MO_ConstantPoolIndex:
      MCOp = LowerSymbolOperand(MO, GetCPISymbol(MO.getIndex()));
      break;

    case MachineOperand::MO_RegisterMask:
      return false;
      break;

    default:
      report_fatal_error("unknown operand type");
  }

  return true;
}

MCOperand DLXAsmPrinter::LowerSymbolOperand(const MachineOperand &MO,
                                              MCSymbol *Sym) const {
  MCContext &Ctx = OutContext;
  DLXMCExpr::VariantKind Kind;

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case DLXII::MO_None:
    Kind = DLXMCExpr::VK_DLX_None;
    break;
  case DLXII::MO_CALL:
    Kind = DLXMCExpr::VK_DLX_CALL;
    break;
  case DLXII::MO_LO:
    Kind = DLXMCExpr::VK_DLX_LO;
    break;
  case DLXII::MO_HI:
    Kind = DLXMCExpr::VK_DLX_HI;
    break;
  }

  const MCExpr *ME =
      MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, Ctx);

  if (!MO.isJTI() && !MO.isMBB() && MO.getOffset())
    ME = MCBinaryExpr::createAdd(
        ME, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);

  if (Kind != DLXMCExpr::VK_DLX_None)
    ME = DLXMCExpr::create(ME, Kind, Ctx);

  return MCOperand::createExpr(ME);
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeDLXAsmPrinter() {
  RegisterAsmPrinter<DLXAsmPrinter> X(getTheDLXTarget());
}