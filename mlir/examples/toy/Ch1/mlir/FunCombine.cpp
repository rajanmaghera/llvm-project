//
// Created by Rajan Maghera on 2025-09-17.
//

#include "toy/FunCombine.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/OperationSupport.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/Value.h"
#include "mlir/Interfaces/FunctionImplementation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "toy/Dialect.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include <algorithm>
#include <mlir/IR/DialectImplementation.h>
#include <string>

#include "mlir/IR/IRMapping.h"

struct EvalOpLowering : public mlir::OpRewritePattern<mlir::toy::EvalOp> {
  // using OpRewritePattern<mydialect::EvalOp>::OpRewritePattern;

  EvalOpLowering(mlir::MLIRContext *context) : OpRewritePattern<mlir::toy::EvalOp>(context) {}

  mlir::LogicalResult matchAndRewrite(
      mlir::toy::EvalOp evalOp,
      mlir::PatternRewriter &rewriter) const override {

    // Get the closure being evaluated.
    auto closureOp = evalOp.getFn().getDefiningOp<mlir::toy::ClosureOp>();
    if (!closureOp)
      return rewriter.notifyMatchFailure(evalOp, "expected a closure");

    // Get the body of the closure.
    mlir::Region &body = closureOp.getBody();
    if (body.empty())
      return rewriter.notifyMatchFailure(evalOp, "closure has no body");

    // Clone the closure block into the same region as the eval block
    // mlir::IRMapping map;
    auto clonedOp = closureOp.clone();
    auto ret = clonedOp.getBody().back().getTerminator();
    auto val = ret->getOperand(0);

    // Inline the block before the evalop
    rewriter.inlineBlockBefore(&clonedOp.getBody().front(), evalOp);

    // Erase the eval op and any leftover rets
    rewriter.replaceOp(evalOp, val);
    rewriter.eraseOp(ret);

    return mlir::success();
  }
};

  void toy::ApplyClosurePass::runOnOperation() {
    mlir::RewritePatternSet patterns(&getContext());
    patterns.add<EvalOpLowering>(&getContext());
    mlir::GreedyRewriteConfig config;
    if (mlir::failed(mlir::applyPatternsAndFoldGreedily(getOperation(), std::move(patterns), config))) {
      signalPassFailure();
    }
  }

