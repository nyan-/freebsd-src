//===--- SPIRV.cpp - SPIR-V Tool Implementations ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "SPIRV.h"
#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/InputInfo.h"
#include "clang/Driver/Options.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace llvm::opt;

void SPIRV::constructTranslateCommand(Compilation &C, const Tool &T,
                                      const JobAction &JA,
                                      const InputInfo &Output,
                                      const InputInfo &Input,
                                      const llvm::opt::ArgStringList &Args) {
  llvm::opt::ArgStringList CmdArgs(Args);
  CmdArgs.push_back(Input.getFilename());

  if (Input.getType() == types::TY_PP_Asm)
    CmdArgs.push_back("-to-binary");
  if (Output.getType() == types::TY_PP_Asm)
    CmdArgs.push_back("--spirv-tools-dis");

  CmdArgs.append({"-o", Output.getFilename()});

  const char *Exec =
      C.getArgs().MakeArgString(T.getToolChain().GetProgramPath("llvm-spirv"));
  C.addCommand(std::make_unique<Command>(JA, T, ResponseFileSupport::None(),
                                         Exec, CmdArgs, Input, Output));
}

void SPIRV::Translator::ConstructJob(Compilation &C, const JobAction &JA,
                                     const InputInfo &Output,
                                     const InputInfoList &Inputs,
                                     const ArgList &Args,
                                     const char *LinkingOutput) const {
  claimNoWarnArgs(Args);
  if (Inputs.size() != 1)
    llvm_unreachable("Invalid number of input files.");
  constructTranslateCommand(C, *this, JA, Output, Inputs[0], {});
}

clang::driver::Tool *SPIRVToolChain::getTranslator() const {
  if (!Translator)
    Translator = std::make_unique<SPIRV::Translator>(*this);
  return Translator.get();
}

clang::driver::Tool *SPIRVToolChain::SelectTool(const JobAction &JA) const {
  Action::ActionClass AC = JA.getKind();
  return SPIRVToolChain::getTool(AC);
}

clang::driver::Tool *SPIRVToolChain::getTool(Action::ActionClass AC) const {
  switch (AC) {
  default:
    break;
  case Action::BackendJobClass:
  case Action::AssembleJobClass:
    return SPIRVToolChain::getTranslator();
  }
  return ToolChain::getTool(AC);
}
