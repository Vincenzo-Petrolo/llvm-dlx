Compiler-RT
================================

This directory and its subdirectories contain source code for the compiler
support routines.

Compiler-RT is open source software. You may freely distribute it under the
terms of the license agreement found in LICENSE.txt.

================================

DLX32 Configuration
```sh
cmake ../compiler-rt -DLLVM_CMAKE_DIR=../build -DLLVM_CONFIG_PATH=../build/bin/llvm-config -DCOMPILER_RT_BUILD_BUILTINS=ON -DCOMPILER_RT_BUILD_SANITIZERS=OFF -DCOMPILER_RT_BUILD_XRAY=OFF -DCOMPILER_RT_BUILD_LIBFUZZER=OFF -DCOMPILER_RT_DEFAULT_TARGET=dlx32
```