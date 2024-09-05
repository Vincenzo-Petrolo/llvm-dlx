//===-- DLXISelLowering.cpp - DLX DAG Lowering Implementation -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that DLX uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//
#include "DLXISelLowering.h"
#include "DLXSubtarget.h"
#include "DLXTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/Debug.h"
#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "dlx-isellower"

#include "DLXGenCallingConv.inc"

#define EXPAND(Op) setOperationAction(ISD::Op, MVT::i32, Expand)


DLXTargetLowering::DLXTargetLowering(const TargetMachine &TM,
                                         const DLXSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI)
{
  // Set up the register classes
  addRegisterClass(MVT::i32, &DLX::GPRRegClass);

  // Must, computeRegisterProperties - Once all of the register classes are
  // added, this allows us to compute derived properties we expose.
  computeRegisterProperties(Subtarget.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(DLX::R2);

  // Set scheduling preference. There are a few options: //    - None: No preference
  //    - Source: Follow source order
  //    - RegPressure: Scheduling for lowest register pressure
  //    - Hybrid: Scheduling for both latency and register pressure
  // Source (the option used by XCore) is no good when there are few registers
  // because the compiler will try to keep a lot more things into the register
  // which eventually results in a lot of stack spills for no good reason. So
  // use either RegPressure or Hybrid
  setSchedulingPreference(Sched::RegPressure);

  // Use i32 for setcc operations results (slt, sgt, ...).
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);

  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::BlockAddress,  MVT::i32, Custom);
  setOperationAction(ISD::ConstantPool,  MVT::i32, Custom);
  setOperationAction(ISD::SELECT,        MVT::i32, Custom);
  setOperationAction(ISD::ExternalSymbol,MVT::i32, Custom);

  // Expand to implement using more basic operations
  // TODO, add other operations that are missing from DLX ISA
  EXPAND(ROTL);
  EXPAND(ROTR);
  EXPAND(BR_CC);

  // TODO add check on M-extension support, for now just expand
  EXPAND(MUL);
  EXPAND(MULHS);
  EXPAND(MULHU);
  EXPAND(SDIV);
  EXPAND(UDIV);
  EXPAND(SREM);
  EXPAND(UREM);
  EXPAND(SMUL_LOHI);
  EXPAND(UMUL_LOHI);
  EXPAND(SDIVREM);
  EXPAND(UDIVREM);
  EXPAND(ADDC);
  EXPAND(SUBC);
  EXPAND(ADDE);
  EXPAND(SUBE);
  EXPAND(UADDO);
  EXPAND(USUBO);
  EXPAND(SADDO);
  EXPAND(SSUBO);
  EXPAND(UMULO);
  EXPAND(SMULO);
  EXPAND(SADDSAT);
  EXPAND(UADDSAT);
  EXPAND(SSUBSAT);
  EXPAND(USUBSAT);
  EXPAND(SMULFIX);
  EXPAND(UMULFIX);
  EXPAND(SMULFIXSAT);
  EXPAND(UMULFIXSAT);
  EXPAND(SDIVFIX);
  EXPAND(UDIVFIX);
  EXPAND(SMIN);
  EXPAND(SMAX);
  EXPAND(UMIN);
  EXPAND(UMAX);
  EXPAND(ABS);
  EXPAND(CTLZ);
  EXPAND(CTTZ);
  EXPAND(CTPOP);
  EXPAND(BITREVERSE);
  EXPAND(SELECT_CC);
  EXPAND(SETCCCARRY);
  EXPAND(SHL_PARTS);
  EXPAND(SRA_PARTS);
  EXPAND(SRL_PARTS);
  EXPAND(SIGN_EXTEND);
  EXPAND(ZERO_EXTEND);
  EXPAND(ANY_EXTEND);
  EXPAND(TRUNCATE);

  // Set minimum and preferred function alignment (log2)
  setMinFunctionAlignment(Align(1));
  setPrefFunctionAlignment(Align(1));

  // Set preferred loop alignment (log2)
  setPrefLoopAlignment(Align(1));
}

const char *DLXTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case DLXISD::Ret: return "DLXISD::Ret";
  default:            return NULL;
  }
}

void DLXTargetLowering::ReplaceNodeResults(SDNode *N,
                                             SmallVectorImpl<SDValue> &Results,
                                             SelectionDAG &DAG) const {
  switch (N->getOpcode()) {
  default:
    llvm_unreachable("Don't know how to custom expand this!");
  }
}

//===----------------------------------------------------------------------===//
//@            Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//

// The BeyondRISC calling convention parameter registers.
static const MCPhysReg GPRArgRegs[] = {
  DLX::R0, DLX::R1, DLX::R2, DLX::R3
};

/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue DLXTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {

  assert((CallingConv::C == CallConv || CallingConv::Fast == CallConv) &&
         "Unsupported CallingConv in FORMAL_ARGUMENTS Lowering");

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, DLX_CCallingConv);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    EVT ValVT = VA.getValVT();
    SDValue ArgValue;

    if (VA.isRegLoc()) {
      // Argument is passed in a register
      EVT RegVT = VA.getLocVT();
      unsigned Reg = MF.addLiveIn(VA.getLocReg(), &DLX::GPRRegClass);
      ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, RegVT);

      // Handle any needed conversions.
      if (VA.getLocInfo() == CCValAssign::SExt) {
        ArgValue = DAG.getNode(ISD::AssertSext, dl, RegVT, ArgValue,
                               DAG.getValueType(ValVT));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, ValVT, ArgValue);
      } else if (VA.getLocInfo() == CCValAssign::ZExt) {
        ArgValue = DAG.getNode(ISD::AssertZext, dl, RegVT, ArgValue,
                               DAG.getValueType(ValVT));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, ValVT, ArgValue);
      } else if (VA.getLocInfo() == CCValAssign::AExt) {
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, ValVT, ArgValue);
      }

      InVals.push_back(ArgValue);

    } else if (VA.isMemLoc()) {
      // Argument is passed on the stack
      int FI = MFI.CreateFixedObject(ValVT.getSizeInBits() / 8, VA.getLocMemOffset(),
                                     true);
      SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
      ArgValue = DAG.getLoad(ValVT, dl, Chain, FIN,
                             MachinePointerInfo::getFixedStack(MF, FI));

      InVals.push_back(ArgValue);
    } else {
      llvm_unreachable("Unknown argument location");
    }
  }

  return Chain;
}


//===----------------------------------------------------------------------===//
//@              Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

bool DLXTargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                MachineFunction &MF, bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                LLVMContext &Context) const
{
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);
  return CCInfo.CheckReturn(Outs, DLX_CRetConv);
}

/// LowerMemOpCallTo - Store the argument to the stack.
SDValue DLXTargetLowering::LowerMemOpCallTo(SDValue Chain,
                                              SDValue Arg, const SDLoc &dl,
                                              SelectionDAG &DAG,
                                              const CCValAssign &VA,
                                              ISD::ArgFlagsTy Flags) const {
  llvm_unreachable("Cannot store arguments to stack");
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue
DLXTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                     CallingConv::ID CallConv, bool isVarArg,
                                     const SmallVectorImpl<ISD::InputArg> &Ins,
                                     const SDLoc &dl, SelectionDAG &DAG,
                                     SmallVectorImpl<SDValue> &InVals,
                                     bool isThisReturn, SDValue ThisVal) const {
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeCallResult(Ins, DLX_CRetConv);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign VA = RVLocs[i];

    // Pass 'this' value directly from the argument to return value, to avoid
    // reg unit interference
    if (i == 0 && isThisReturn) {
      assert(!VA.needsCustom() && VA.getLocVT() == MVT::i32 &&
             "unexpected return calling convention register assignment");
      InVals.push_back(ThisVal);
      continue;
    }

    SDValue Val;
    if (VA.needsCustom()) {
        llvm_unreachable("Vector and floating point values not supported yet");
    } else {
      Val = DAG.getCopyFromReg(Chain, dl, VA.getLocReg(), VA.getLocVT(),
                               InFlag);
      Chain = Val.getValue(1);
      InFlag = Val.getValue(2);
    }

    switch (VA.getLocInfo()) {
    default: llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full: break;
    case CCValAssign::BCvt:
      Val = DAG.getNode(ISD::BITCAST, dl, VA.getValVT(), Val);
      break;
    }

    InVals.push_back(Val);
  }

  return Chain;
}

SDValue 
DLXTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &dl = CLI.DL;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;
  
  // Analyze the operands of the call, assigning locations to each operand
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs, *DAG.getContext());

  // Assuming DLX has a custom calling convention, DLXCC
  CCInfo.AnalyzeCallOperands(CLI.Outs, DLX_CCallingConv);

  // Prepare the call sequence
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Assign locations to arguments and pass them
  for (unsigned i = 0; i != CLI.Outs.size(); ++i) {
    SDValue Arg = CLI.OutVals[i];
    CCValAssign &VA = ArgLocs[i];
    
    if (VA.isRegLoc()) {
      Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), Arg, SDValue());
      Ops.push_back(DAG.getRegister(VA.getLocReg(), Arg.getValueType()));
    } else {
      // Memory location
      SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset(), dl);
      SDValue StackPtr = DAG.getCopyFromReg(Chain, dl, DLX::R2, PtrOff.getValueType());
      SDValue Addr = DAG.getNode(ISD::ADD, dl, PtrOff.getValueType(), StackPtr, PtrOff);
      Chain = DAG.getStore(Chain, dl, Arg, Addr, MachinePointerInfo());
    }
  }

  // Create the actual call node
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(DLXISD::Call, dl, NodeTys, Ops);

  // Handle return values
  if (!CLI.Ins.empty()) {
    SmallVector<CCValAssign, 16> RVLocs;
    CCState RVInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs, *DAG.getContext());
    RVInfo.AnalyzeCallResult(CLI.Ins, DLX_CRetConv);

    for (unsigned i = 0; i != RVLocs.size(); ++i) {
      SDValue RV = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(), RVLocs[i].getValVT());
      InVals.push_back(RV);
    }
  }

  return Chain;
}

/// HandleByVal - Every parameter *after* a byval parameter is passed
/// on the stack.  Remember the next parameter register to allocate,
/// and then confiscate the rest of the parameter registers to insure
/// this.
void DLXTargetLowering::HandleByVal(CCState *State, unsigned &Size,
                                      unsigned Align) const {
  // Byval (as with any stack) slots are always at least 4 byte aligned.
  Align = std::max(Align, 4U);

  unsigned Reg = State->AllocateReg(GPRArgRegs);
  if (!Reg)
    return;

  unsigned AlignInRegs = Align / 4;
  unsigned Waste = (DLX::R4 - Reg) % AlignInRegs;
  for (unsigned i = 0; i < Waste; ++i)
    Reg = State->AllocateReg(GPRArgRegs);

  if (!Reg)
    return;

  unsigned Excess = 4 * (DLX::R4 - Reg);

  // Special case when NSAA != SP and parameter size greater than size of
  // all remained GPR regs. In that case we can't split parameter, we must
  // send it to stack. We also must set NCRN to X4, so waste all
  // remained registers.
  const unsigned NSAAOffset = State->getNextStackOffset();
  if (NSAAOffset != 0 && Size > Excess) {
    while (State->AllocateReg(GPRArgRegs))
      ;
    return;
  }

  // First register for byval parameter is the first register that wasn't
  // allocated before this method call, so it would be "reg".
  // If parameter is small enough to be saved in range [reg, r4), then
  // the end (first after last) register would be reg + param-size-in-regs,
  // else parameter would be splitted between registers and stack,
  // end register would be r4 in this case.
  unsigned ByValRegBegin = Reg;
  unsigned ByValRegEnd = std::min<unsigned>(Reg + Size / 4, DLX::R4);
  State->addInRegsParamInfo(ByValRegBegin, ByValRegEnd);
  // Note, first register is allocated in the beginning of function already,
  // allocate remained amount of registers we need.
  for (unsigned i = Reg + 1; i != ByValRegEnd; ++i)
    State->AllocateReg(GPRArgRegs);
  // A byval parameter that is split between registers and memory needs its
  // size truncated here.
  // In the case where the entire structure fits in registers, we set the
  // size in memory to zero.
  Size = std::max<int>(Size - Excess, 0);
}

SDValue
DLXTargetLowering::LowerReturn(SDValue Chain,
                                 CallingConv::ID CallConv, bool isVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 const SDLoc &dl, SelectionDAG &DAG) const {
  // CCValAssign - represent the assignment of the return value to a location.
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slots.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  // Analyze outgoing return values.
  CCInfo.AnalyzeReturn(Outs, DLX_CRetConv);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps;
  RetOps.push_back(Chain); // Operand #0 = Chain (updated below)

  // Copy the result values into the output registers.
  for (unsigned i = 0, realRVLocIdx = 0;
       i != RVLocs.size();
       ++i, ++realRVLocIdx) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue Arg = OutVals[realRVLocIdx];
    bool ReturnF16 = false;

    switch (VA.getLocInfo()) {
    default: llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full: break;
    case CCValAssign::BCvt:
      if (!ReturnF16)
        Arg = DAG.getNode(ISD::BITCAST, dl, VA.getLocVT(), Arg);
      break;
    }

    if (VA.needsCustom()) {
      llvm_unreachable("Custom val assignment not supported by "
                       "RETURN Lowering");
    } else {
      Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), Arg, Flag);
    }

    // Guarantee that all emitted copies are stuck together, avoiding something
    // bad.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(),
                                     ReturnF16 ? MVT::f16 : VA.getLocVT()));
  }

  // Update chain and glue.
  RetOps[0] = Chain;
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(DLXISD::Ret, dl, MVT::Other, RetOps);
}

//===----------------------------------------------------------------------===//
//  Lower helper functions
//===----------------------------------------------------------------------===//

SDValue DLXTargetLowering::getGlobalAddressWrapper(SDValue GA,
                                                     const GlobalValue *GV,
                                                     SelectionDAG &DAG) const {
  llvm_unreachable("Unhandled global variable");
}

//===----------------------------------------------------------------------===//
//  Misc Lower Operation implementation
//===----------------------------------------------------------------------===//

SDValue DLXTargetLowering::LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const {
    GlobalAddressSDNode *GSDN = cast<GlobalAddressSDNode>(Op);
    const GlobalValue *GV = GSDN->getGlobal();
    SDLoc DL(Op);
    
    // Retrieve the offset, which is typically 0 unless specified
    int64_t Offset = GSDN->getOffset();

    // Step 1: Create TargetGlobalAddress nodes for the high and low parts
    SDValue HighPart = DAG.getTargetGlobalAddress(GV, DL, MVT::i32, Offset, DLXII::MO_HI);
    SDValue LowPart = DAG.getTargetGlobalAddress(GV, DL, MVT::i32, Offset, DLXII::MO_LO);

    // Step 2: Load the high part into a register
    SDValue LHI = DAG.getNode(DLXISD::LHI, DL, MVT::i32, HighPart);

    // Step 3: Combine with the low part using ORI
    SDValue ORI = DAG.getNode(DLXISD::ORI, DL, MVT::i32, LHI, LowPart);

    // Step 4: Return the final result that combines high and low parts
    return ORI;
}


SDValue DLXTargetLowering::
LowerConstantPool(SDValue Op, SelectionDAG &DAG) const {
  llvm_unreachable("Unsupported constant pool");
}

SDValue DLXTargetLowering::
LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const {
  llvm_unreachable("Unsupported block address");
}

SDValue 
DLXTargetLowering::lowerSELECT(SDValue Op, SelectionDAG &DAG) const {
  SDValue CondV = Op.getOperand(0);
  SDValue TrueV = Op.getOperand(1);
  SDValue FalseV = Op.getOperand(2);
  SDLoc DL(Op);

  // Assuming CondV is an i1 (1-bit integer), extend it to match the bit-width of TrueV/FalseV.
  EVT ValueType = TrueV.getValueType();
  SDValue CondVExtended = DAG.getNode(ISD::SIGN_EXTEND, DL, ValueType, CondV);

  // Calculate the true mask: ~0 if CondV is true, 0 if CondV is false.
  SDValue TrueMask = CondVExtended;

  // Calculate the false mask: ~TrueMask
  SDValue FalseMask = DAG.getNode(ISD::XOR, DL, ValueType, TrueMask, DAG.getConstant(-1, DL, ValueType));

  // Now perform the bitwise operations to select the correct value.
  SDValue TruePart = DAG.getNode(ISD::AND, DL, ValueType, TrueV, TrueMask);
  SDValue FalsePart = DAG.getNode(ISD::AND, DL, ValueType, FalseV, FalseMask);

  // Combine the true and false parts.
  SDValue Result = DAG.getNode(ISD::OR, DL, ValueType, TruePart, FalsePart);

  return Result;
}

SDValue 
DLXTargetLowering::lowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const {
  // This nodes represent llvm.frameaddress on the DAG.
  // It takes one operand, the index of the frame address to return.
  // An index of zero corresponds to the current function's frame address.
  // An index of one to the parent's frame address, and so on.
  // Depths > 0 not supported yet!

  if (Op.getConstantOperandVal(0) > 0)
    return SDValue();

  MachineFunction &MF = DAG.getMachineFunction();
  const TargetRegisterInfo *RegInfo = Subtarget.getRegisterInfo();
  return DAG.getCopyFromReg(DAG.getEntryNode(), SDLoc(Op),
                            RegInfo->getFrameRegister(MF), MVT::i32);
}

SDValue 
DLXTargetLowering::lowerRETURNADDR(SDValue Op, SelectionDAG &DAG) const {
  const DLXRegisterInfo &RI = *Subtarget.getRegisterInfo();
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MFI.setReturnAddressIsTaken(true);
  MVT XLenVT = MVT::i32;
  int XLenInBytes = 32 / 8;

  if (verifyReturnAddressArgumentIsConstant(Op, DAG))
    return SDValue();

  EVT VT = Op.getValueType();
  SDLoc DL(Op);
  unsigned Depth = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();

  if (Depth > 0) // Depth > 0 not supported yet!
    return SDValue();

  // Return the value of the return address register
  Register Reg = RI.getRARegister();
  return DAG.getCopyFromReg(DAG.getEntryNode(), DL, Reg, XLenVT);
}

SDValue 
DLXTargetLowering::LowerFrameIndex(SDValue Op, SelectionDAG &DAG) const {
  FrameIndexSDNode *FINode = cast<FrameIndexSDNode>(Op);
  int FrameIndex = FINode->getIndex();

  SDValue SPReg = DAG.getRegister(DLX::R2, MVT::i32); // r2 is the SP

  // Compute the effective address using r2 (SP) and the frame index offset
  SDValue TFI = DAG.getTargetFrameIndex(FrameIndex, MVT::i32);
  SDValue Addr = DAG.getNode(ISD::ADD, SDLoc(Op), MVT::i32, SPReg, TFI);

  return Addr;
}

SDValue 
DLXTargetLowering::LowerExternalSymbol(SDValue Op, SelectionDAG &DAG) const {
  ExternalSymbolSDNode *ES = cast<ExternalSymbolSDNode>(Op);
  const char *Symbol = ES->getSymbol();

  SDLoc DL(Op);
  SDValue Chain = DAG.getEntryNode();
  
  // Get the target-specific symbol node
  SDValue Callee = DAG.getTargetExternalSymbol(Symbol, getPointerTy(DAG.getDataLayout()));

  // Determine if the callee is a symbol (direct call) or a register (indirect call)
  if (isa<ExternalSymbolSDNode>(Callee)) {
      // Direct call to an external symbol
      // JAL: Direct Jump And Link to an external symbol
      Callee = DAG.getTargetExternalSymbol(Symbol, getPointerTy(DAG.getDataLayout()), DLXII::MO_CALL);
      SDValue Ops[] = { Callee, Chain };  // Callee is a symbol
      return DAG.getNode(DLXISD::Call, DL, DAG.getVTList(MVT::Other, MVT::Glue), Ops);
  } else {
      // Indirect call via register (e.g., function pointer)
      // JALR: Jump And Link Register
      SDValue Reg = Callee;  // Callee is a register here
      SDValue Ops[] = { Reg, Chain };  // Register comes before Chain
      return DAG.getNode(DLXISD::Call, DL, DAG.getVTList(MVT::Other, MVT::Glue), Ops);
  }
}

SDValue
DLXTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:        return LowerGlobalAddress(Op, DAG);
  case ISD::BlockAddress:         return LowerBlockAddress(Op, DAG);
  case ISD::ConstantPool:         return LowerConstantPool(Op, DAG);
  case ISD::RETURNADDR:           return lowerRETURNADDR(Op, DAG);
  case ISD::SELECT:               return lowerSELECT(Op, DAG);
  case ISD::FRAMEADDR:            return lowerFRAMEADDR(Op, DAG);
  case ISD::ExternalSymbol:       return LowerExternalSymbol(Op, DAG);
  default: llvm_unreachable("unimplemented operand");
  }
}