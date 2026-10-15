//===- MLIRGen.cpp - MLIR Generation from a Toy AST -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a simple IR generation targeting MLIR from a Module AST
// for the Toy language.
//
//===----------------------------------------------------------------------===//

#include "toy/MLIRGen.h"
#include "mlir/IR/Block.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/Value.h"
#include "toy/AST.h"
#include "toy/Dialect.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "toy/Lexer.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include <cassert>
#include <cstdint>
#include <functional>
#include <numeric>
#include <optional>
#include <vector>

using namespace mlir::toy;
using namespace toy;

using llvm::ArrayRef;
using llvm::cast;
using llvm::dyn_cast;
using llvm::isa;
using llvm::ScopedHashTableScope;
using llvm::SmallVector;
using llvm::StringRef;
using llvm::Twine;

namespace {

class MLIRGenImpl {
public:
  MLIRGenImpl(mlir::MLIRContext &context) : builder(&context) {}
  mlir::ModuleOp mlirGen() {
    theModule = mlir::ModuleOp::create(builder.getUnknownLoc());
    std::vector<mlir::Value> args;

    // NOTE: this is hardcoded MLIR code to test functionality
    builder.setInsertionPointToStart(theModule.getBody());

    // auto v1 = mlir::toy::ConstantOp::create(builder, builder.getUnknownLoc(), builder.getI32Type(), builder.getI32IntegerAttr(34));
    auto funcret = mlir::toy::RetType::get(builder.getI32Type());
    auto c = mlir::toy::ClosureOp::create(builder, builder.getUnknownLoc(), funcret);
    mlir::Region &region = c.getBody();

    // add a block to the body
    mlir::Block *block = new mlir::Block();
    region.push_back(block);

    // create a builder for the new block
    mlir::OpBuilder bodyBuilder(block, block->begin());
    // insert ops into body
    auto v1 = bodyBuilder.create<mlir::toy::ConstantOp>(builder.getUnknownLoc(), builder.getI32Type(), builder.getI32IntegerAttr(34));
    auto ret = bodyBuilder.create<mlir::toy::ReturnOp>(builder.getUnknownLoc(), v1);

    auto apply = builder.create<mlir::toy::EvalOp>(builder.getUnknownLoc(), funcret, c);
    auto print = builder.create<mlir::toy::PrintOp>(builder.getUnknownLoc(), apply);
    auto apply2 = builder.create<mlir::toy::EvalOp>(builder.getUnknownLoc(), funcret, c);
    auto print2= builder.create<mlir::toy::PrintOp>(builder.getUnknownLoc(), apply2);


    if (failed(mlir::verify(theModule))) {
      theModule.emitError("module verification error");
      return nullptr;
    }
    return theModule;
  }

private:
  mlir::ModuleOp theModule;
  mlir::OpBuilder builder;
};

} // namespace

namespace toy {

// The public API for codegen.
mlir::OwningOpRef<mlir::ModuleOp> mlirGen(mlir::MLIRContext &context) {
  return MLIRGenImpl(context).mlirGen();
}

} // namespace toy
