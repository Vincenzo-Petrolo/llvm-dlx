set(LLVM_TARGET_DEFINITIONS DLX.td)

tablegen(LLVM DLXGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM DLXGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM DLXGenCodeEmitter.inc -gen-emitter)
tablegen(LLVM DLXGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM DLXGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM DLXGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM DLXGenCallingConv.inc -gen-callingconv)
tablegen(LLVM DLXGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM DLXGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM DLXGenAsmMatcher.inc -gen-asm-matcher)

# DLXCommonTableGen must be defined
add_public_tablegen_target(DLXCommonTableGen)

# DLXCodeGen should match with LLVMBuild.txt DLXCodeGen
add_llvm_target(DLXCodeGen
  DLXAsmPrinter.cpp
  DLXFrameLowering.cpp
  DLXInstrInfo.cpp
  DLXISelLowering.cpp
  DLXRegisterInfo.cpp
  DLXSubtarget.cpp
  DLXTargetObjectFile.cpp
  DLXISelDAGToDAG.cpp
  DLXTargetMachine.cpp
)

# Should match with "subdirectories =  MCTargetDesc TargetInfo" in LLVMBuild.txt
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(AsmParser)