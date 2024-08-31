//===---- DLXISelDAGToDAG.h - A Dag to Dag Inst Selector for DLX ------===//
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

#ifndef LLVM_LIB_TARGET_DLX_DLXISELDAGTODAG_H
#define LLVM_LIB_TARGET_DLX_DLXISELDAGTODAG_H

#include "DLXSubtarget.h"
#include "DLXTargetMachine.h"
#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {
class DLXDAGToDAGISel : public SelectionDAGISel {
public:
  explicit DLXDAGToDAGISel(DLXTargetMachine &TM, CodeGenOpt::Level OL)
      : SelectionDAGISel(TM, OL), Subtarget(nullptr) {}

  // Pass Name
  StringRef getPassName() const override {
    return "CPU0 DAG->DAG Pattern Instruction Selection";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void Select(SDNode *Node) override;
  bool SelectAddrFI(SDValue Addr, SDValue &Base);

#include "DLXGenDAGISel.inc"

private:
  const DLXSubtarget *Subtarget;
};
}

#endif // end LLVM_LIB_TARGET_DLX_DLXISELDAGTODAG_H