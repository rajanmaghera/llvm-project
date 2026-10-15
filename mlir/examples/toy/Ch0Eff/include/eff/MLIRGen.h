//
// Created by Rajan Maghera on 2025-09-29.
//

#ifndef EFF_MLIRGEN_H
#define EFF_MLIRGEN_H

#include <memory>

namespace mlir {
class MLIRContext;
template <typename OpTy>
class OwningOpRef;
class ModuleOp;
} // namespace mlir

namespace eff {
class ModuleAST;

mlir::OwningOpRef<mlir::ModuleOp> mlirGen(mlir::MLIRContext &context);
} // namespace eff


#endif //MLIRGEN_H
