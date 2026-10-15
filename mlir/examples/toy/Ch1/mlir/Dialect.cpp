//===- Dialect.cpp - Toy IR Dialect registration in MLIR ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the dialect for the Toy IR: custom type parsing and
// operation verification.
//
//===----------------------------------------------------------------------===//

#include "toy/Dialect.h"

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/OperationSupport.h"
#include "mlir/IR/Value.h"
#include "mlir/Interfaces/FunctionImplementation.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include <algorithm>
#include <mlir/IR/DialectImplementation.h>
#include <string>

using namespace mlir;
using namespace mlir::toy;

#include "toy/Dialect.cpp.inc"

RetType RetType::get(mlir::Type elementType) {
  mlir::MLIRContext *ctx = elementType.getContext();
  return Base::get(ctx, elementType);
}

mlir::Type RetType::getElementType() const {
  return getImpl()->elementType;
}

void FunDialect::initialize() {
  addTypes<RetType>();
  addOperations<
#define GET_OP_LIST
#include "toy/Ops.cpp.inc"
      >();
}

mlir::Type FunDialect::parseType(mlir::DialectAsmParser &parser) const {
  // Parse a function def in the format
  //  clos-type ::= `clos` `<` `ret` type `>`
  if (parser.parseKeyword("cls") || parser.parseLess()||parser.parseKeyword("ret"))
    return mlir::Type();

  mlir::Type elementType;
  if (parser.parseType(elementType))
    return nullptr;
  if (parser.parseGreater())
    return mlir::Type();

  return RetType::get(elementType);

}

void FunDialect::printType(mlir::Type type, mlir::DialectAsmPrinter &printer) const {
  RetType retType = llvm::cast<RetType>(type);
  printer << "clos<ret ";
  printer << retType.getElementType();
  printer << ">";
}

llvm::LogicalResult ClosureOp::verify() {
  auto &body = getBody();

  if (body.empty())
    return emitOpError() << "closure body is empty";

  // Body has no block args
  if (body.getNumArguments() != 0)
    return emitOpError() << "closure body must not have block arguments";

  auto ret = llvm::dyn_cast<ReturnOp>(&body.back().back());
  if (!ret)
    return emitOpError() << "closure body must end in fun.return";

  auto funcType = llvm::dyn_cast<RetType>(getClosure().getType());
  auto retVariadic = ret.getResults().getType();

  if (funcType.getElementType() != retVariadic)
    return emitOpError() << "closure body must not have the same type";
  for (Block&block : body) {
  for (Operation &op : block) {
    for (Value operand : op.getOperands()) {
      if (operand.getParentRegion() != &getBody()) {
        return emitOpError() << "closure body operand " << operand << " is defined outside closure, not allowed";
      }
    }
  }
  }

return success();

}


llvm::LogicalResult EvalOp::verify() {

  auto fnType = llvm::dyn_cast<RetType>(getFn().getType());
  auto resType = getResults().getType();
  auto closType = fnType;
    if (closType != resType)
    return emitOpError() << "closure return type " << closType << " is not the same as the eval return type " << resType;

return success();

}

llvm::LogicalResult ConstantOp::verify() {
  auto valueAttr = getValue();
  if (!valueAttr)
    return emitOpError() << "requires value attribute";

  if (valueAttr.getType() != getType())
    return emitOpError() << "requires type " << valueAttr.getType() << " but got " << getType();

return success();

}

#define GET_OP_CLASSES
#include "toy/Ops.cpp.inc"
