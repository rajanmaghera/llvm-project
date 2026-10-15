//
// Created by Rajan Maghera on 2025-09-29.
//

#include "eff/MLIRGen.h"

#include "mlir/IR/Block.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/Value.h"
#include "eff/EffOps.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include <cassert>
#include <cstdint>
#include <functional>

#include "llvm/BinaryFormat/COFF.h"

#include <numeric>
#include <optional>
#include <vector>

using namespace mlir::eff;
using namespace eff;

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
  MLIRGenImpl(mlir::MLIRContext &context) : builder(&context)  {}

  mlir::ModuleOp mlirGen() {
    theModule = mlir::ModuleOp::create(builder.getUnknownLoc());

    // TODO: emit IR here
    mlirGenHardcoded();
    mlirGenHardcodedSecond();

    if (failed(mlir::verify(theModule))) {
      theModule.emitError("module verification error");
      return nullptr;
    }

    return theModule;
  }

private:
  mlir::ModuleOp theModule;
  mlir::OpBuilder builder;
  llvm::ScopedHashTable<StringRef, mlir::Value> symbolTable;

  mlir::eff::FuncOp mlirGenHardcodedSecond() {
    builder.setInsertionPointToEnd(theModule.getBody());
    llvm::SmallVector<mlir::Type, 4> argTypes;
    argTypes.reserve(1);
    argTypes.push_back(
      builder.getI64Type()
    );
    auto funcType = builder.getFunctionType(argTypes, builder.getI64Type());
    auto function = mlir::eff::FuncOp::create(builder, builder.getUnknownLoc(), "thedup", funcType);
    mlir::Block&entryBlock = *function.addEntryBlock();
    builder.setInsertionPointToStart(&entryBlock);

    auto ioReadEffType = EffType::get("io.modify", builder.getFunctionType({builder.getI64Type()}, {builder.getI64Type()}));
    auto val = mlir::eff::CallOp::create(builder, builder.getUnknownLoc(), "thefunc", {builder.getI64Type()}, {entryBlock.getArgument(0), entryBlock.getArgument(0)}, {ioReadEffType});

    auto handler = HandleOp::create(builder, builder.getUnknownLoc(), ioReadEffType);

    auto ret = mlir::eff::ReturnOp::create(builder, builder.getUnknownLoc(),{val.getResult(0)});
    return function;

  }

  mlir::eff::FuncOp mlirGenHardcoded() {
    // Generate some hardcoded function code
    builder.setInsertionPointToEnd(theModule.getBody());

    // New function that takes two integers and returns an integer
    auto funcType = builder.getFunctionType({builder.getI64Type(), builder.getI64Type()}, builder.getI64Type());
    auto eff =  mlir::eff::EffType::get("io.modify", builder.getFunctionType({builder.getI64Type()}, {builder.getI64Type()}));
    auto function = mlir::eff::FuncOp::create(builder, builder.getUnknownLoc(), "thefunc", funcType, eff);
    mlir::Block &entryBlock = *function.addEntryBlock();
    builder.setInsertionPointToStart(&entryBlock);
    auto val = mlir::arith::AddIOp::create(builder, builder.getUnknownLoc(), entryBlock.getArgument(0), entryBlock.getArgument(1));

    // add to IO read value
    auto inp = mlir::eff::DoEffectOp::create(builder, builder.getUnknownLoc(), "io.modify", {builder.getI64Type()}, {});
    auto val2 = mlir::arith::AddIOp::create(builder, builder.getUnknownLoc(), val.getResult(), inp.getResult(0));

    // // return nothing for now
    ReturnOp::create(builder, builder.getUnknownLoc(), {val2.getResult()});
    return function;

  }
};

}

namespace eff {

// The public API for codegen.
mlir::OwningOpRef<mlir::ModuleOp> mlirGen(mlir::MLIRContext &context) {
  return MLIRGenImpl(context).mlirGen();
}

} // namespace eff
