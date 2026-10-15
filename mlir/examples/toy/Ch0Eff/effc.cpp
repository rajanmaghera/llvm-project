//
// Created by Rajan Maghera on 2025-09-29.
//


//===- toyc.cpp - The Toy Compiler ----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the entry point for the Toy compiler.
//
//===----------------------------------------------------------------------===//

#include "eff/EffOps.h"
#include "eff/Passes.h"
#include "include/eff/MLIRGen.h"
#include "mlir/Dialect/Func/Extensions/AllExtensions.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Diagnostics.h"

#include "mlir/Dialect/Affine/Passes.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "mlir/InitAllDialects.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/Passes.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include "mlir/Dialect/Arith/IR/Arith.h"
#include <string>
#include <system_error>
#include <utility>

using namespace eff;
namespace cl = llvm::cl;

static cl::opt<bool> enableOpt("opt", cl::desc("Enable optimizations"));

static int dumpEffMLIR() {
  mlir::DialectRegistry registry;

  mlir::MLIRContext context(registry);
  context.getOrLoadDialect<mlir::eff::EffDialect>();
  context.getOrLoadDialect<mlir::arith::ArithDialect>();

  mlir::OwningOpRef<mlir::ModuleOp> module;
  llvm::SourceMgr sourceMgr;
  mlir::SourceMgrDiagnosticHandler sourceMgrHandler(sourceMgr, &context);
  module = mlirGen(context);
  if (!module)
    return 4;

  module->dump();

  mlir::PassManager pm(module.get()->getName());
  if (mlir::failed(mlir::applyPassManagerCLOptions(pm)))
    return 4;
  // pm.addPass(mlir::eff::createEffToFuncLoweringPass());

  if (enableOpt) {
    pm.addPass(mlir::createInlinerPass());

    // Now that there is only one function, we can infer the shapes of each of
    // the operations.
    mlir::OpPassManager &optPM = pm.nest<mlir::eff::FuncOp>();
    optPM.addPass(mlir::createCanonicalizerPass());
    optPM.addPass(mlir::createCSEPass());
    optPM.addPass(mlir::affine::createLoopFusionPass());
    optPM.addPass(mlir::affine::createAffineScalarReplacementPass());
  }
  if (mlir::failed(pm.run(*module)))
    return 4;

  module->dump();
  return 0;
}


int main(int argc, char **argv) {
  // Register any command line options.
  mlir::registerAsmPrinterCLOptions();
  mlir::registerMLIRContextCLOptions();
  mlir::registerPassManagerCLOptions();

  cl::ParseCommandLineOptions(argc, argv, "effect lang compiler\n");

  dumpEffMLIR();

  return 0;
}
