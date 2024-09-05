add_llvm_library(LLVMDLXASMParser
  DLXAsmParser.cpp
  )

add_dependencies(LLVMDLXAsmParser DLXCommonTableGen)