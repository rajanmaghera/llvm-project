//
// Created by Rajan Maghera on 2025-09-29.
//

//===- FuncOps.h - Func Dialect Operations ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_DIALECT_EFF_IR_OPS_H
#define MLIR_DIALECT_EFF_IR_OPS_H

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

namespace mlir {
class PatternRewriter;
} // namespace mlir

namespace mlir {
namespace eff {
class EffType;
namespace detail {
struct EffTypeStorage;
}
}
}

#include "eff/EffOpsDialect.h.inc"

#define GET_OP_CLASSES
#include "eff/EffOps.h.inc"


namespace llvm {

/// Allow stealing the low bits of FuncOp.
template <>
struct PointerLikeTypeTraits<mlir::eff::FuncOp> {
  static inline void *getAsVoidPointer(mlir::eff::FuncOp val) {
    return const_cast<void *>(val.getAsOpaquePointer());
  }
  static inline mlir::eff::FuncOp getFromVoidPointer(void *p) {
    return mlir::eff::FuncOp::getFromOpaquePointer(p);
  }
  static constexpr int numLowBitsAvailable = 3;
};
} // namespace llvm

namespace mlir {
namespace eff {

class EffType : public Type::TypeBase<EffType, Type, detail::EffTypeStorage> {

public:
  using Base::Base;

  static EffType get(StringRef name, FunctionType funcSig);

  static LogicalResult verifyConstructionInvariants(
    Location loc, StringRef name, FunctionType funcSig
  );

  StringRef getHandlerName();

  FunctionType getHandlerSignature();

  static constexpr StringLiteral name = "eff.effect";

};
}
}

#endif // MLIR_DIALECT_FUNC_IR_OPS_H