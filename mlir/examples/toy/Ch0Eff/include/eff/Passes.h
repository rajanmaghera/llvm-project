//===- Passes.h - Toy Passes Definition -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file exposes the entry points to create compiler passes for Toy.
//
//===----------------------------------------------------------------------===//

#ifndef EFF_PASSES_H
#define EFF_PASSES_H

#include <memory>

namespace mlir {
class Pass;

namespace eff {

std::unique_ptr<mlir::Pass> createEffToFuncLoweringPass();


} // namespace toy
} // namespace mlir

#endif // TOY_PASSES_H
