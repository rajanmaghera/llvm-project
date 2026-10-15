
#ifndef TOY_FUNCOMBINE_H
#define TOY_FUNCOMBINE_H

#include "mlir/Pass/Pass.h"
#include "llvm/Support/Casting.h"
#include "mlir/IR/PatternMatch.h"

namespace toy {

struct ApplyClosurePass
    : public mlir::PassWrapper<ApplyClosurePass, mlir::OperationPass<mlir::ModuleOp>> {
  void runOnOperation() override;
};

inline std::unique_ptr<mlir::Pass> createApplyClosurePass() {
  return std::make_unique<ApplyClosurePass>();
}

}

#endif // TOY_FUNCOMBINE_H
