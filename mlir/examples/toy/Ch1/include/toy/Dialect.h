//===- Dialect.h - Dialect definition for the Toy IR ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the IR Dialect for the Toy language.
// See docs/Tutorials/Toy/Ch-2.md for more information.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_TUTORIAL_TOY_DIALECT_H_
#define MLIR_TUTORIAL_TOY_DIALECT_H_

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"

/// Include the auto-generated header file containing the declaration of the toy
/// dialect.
#include "toy/Dialect.h.inc"

namespace mlir::toy {
struct RetTypeStorage : public mlir::TypeStorage {
  using KeyTy = mlir::Type;
  RetTypeStorage(mlir::Type elementType) : elementType(elementType) {}
  bool operator==(const KeyTy &key) const {
    return elementType == key;
  }
  static RetTypeStorage *construct(mlir::TypeStorageAllocator &allocator, const KeyTy &key) {
    return new (allocator.allocate<RetTypeStorage>()) RetTypeStorage(key);
  }

  mlir::Type elementType;

};

class RetType : public mlir::Type::TypeBase<RetType, mlir::Type, struct RetTypeStorage> {
public:
  using Base::Base;
  static RetType get(mlir::Type elementType);
  mlir::Type getElementType() const;
  static constexpr StringLiteral name = "fun.clos";
};

}



/// Include the auto-generated header file containing the declarations of the
/// toy operations.
#define GET_OP_CLASSES
#include "toy/Ops.h.inc"

#endif // MLIR_TUTORIAL_TOY_DIALECT_H_
