//===--- DLX.cpp - DLX ToolChain Implementations ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "DLX.h"
#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

DLXToolChain::DLXToolChain(const Driver &D, const llvm::Triple &Triple,
                               const ArgList &Args)
    : ToolChain(D, Triple, Args) {
  // ProgramPaths are found via 'PATH' environment variable.
}

bool DLXToolChain::isPICDefault() const { return true; }

bool DLXToolChain::isPIEDefault() const { return false; }

bool DLXToolChain::isPICDefaultForced() const { return true; }

bool DLXToolChain::SupportsProfiling() const { return false; }

bool DLXToolChain::hasBlocksRuntime() const { return false; }