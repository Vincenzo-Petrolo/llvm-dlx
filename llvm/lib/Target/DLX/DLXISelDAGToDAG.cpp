//===-- DLXISelDAGToDAG.cpp - A Dag to Dag Inst Selector for DLX ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the DLX target.
//
//===----------------------------------------------------------------------===//

#include "DLXISelDAGToDAG.h"
#include "DLXSubtarget.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/SelectionDAGISel.h"

using namespace llvm;

#define DEBUG_TYPE "dlx-isel"

bool DLXDAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
  Subtarget = &static_cast<const DLXSubtarget &>(MF.getSubtarget());
  return SelectionDAGISel::runOnMachineFunction(MF);
}

void DLXDAGToDAGISel::Select(SDNode *Node) {
  unsigned Opcode = Node->getOpcode();
  SDLoc DL(Node);
  EVT VT = Node->getValueType(0);

  // If we have a custom node, we already have selected!
  if (Node->isMachineOpcode()) {
    LLVM_DEBUG(errs() << "== "; Node->dump(CurDAG); errs() << "\n");
    Node->setNodeId(-1);
    return;
  }

  // Instruction Selection not handled by the auto-generated tablegen selection
  // should be handled here.
  switch(Opcode) {
    case ISD::FrameIndex: {
      SDValue Imm = CurDAG->getTargetConstant(0, DL, MVT::i32);
      int FI = cast<FrameIndexSDNode>(Node)->getIndex();
      SDValue TFI = CurDAG->getTargetFrameIndex(FI, VT);
      ReplaceNode(Node, CurDAG->getMachineNode(DLX::ADDI, DL, VT, TFI, Imm));
      return;
    }
    default: break;
  }

  // Select the default instruction
  SelectCode(Node);
}

bool DLXDAGToDAGISel::SelectAddrFI(SDValue Addr, SDValue &Base) {
  if (auto FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i32);
    return true;
  }
  return false;
}