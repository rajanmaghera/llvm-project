//
// Created by Rajan Maghera on 2025-09-29.
//

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/ValueRange.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/TypeID.h"
#include "eff/EffOps.h"
#include "eff/Passes.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/CodeGen/StackMaps.h"
#include "llvm/Support/Casting.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <mlir/Dialect/EmitC/IR/EmitC.h.inc>
#include <utility>

using namespace mlir;

struct FuncOpLowering : public OpConversionPattern<eff::FuncOp> {
  using OpConversionPattern<eff::FuncOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(eff::FuncOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {

    auto func = mlir::func::FuncOp::create(rewriter, op.getLoc(), op.getName(),
                                           op.getFunctionType());
    rewriter.inlineRegionBefore(op.getRegion(), func.getBody(), func.end());
    rewriter.eraseOp(op);
    return success();
  }
};

struct CallOpLowering : public OpConversionPattern<eff::CallOp> {
  using OpConversionPattern<eff::CallOp>::OpConversionPattern;
  LogicalResult
  matchAndRewrite(eff::CallOp op, OpAdaptor adaptor,
    ConversionPatternRewriter &rewriter
    ) const final {
    auto call = mlir::func::CallOp::create(rewriter, op.getLoc(), op.getCallee(), op.getResultTypes(), op.getOperands());
    rewriter.replaceOp(op, call);
    return success();
  }
};


struct ConstantOpLowering : public OpConversionPattern<eff::ConstantOp> {
  using OpConversionPattern<eff::ConstantOp>::OpConversionPattern;
  LogicalResult
  matchAndRewrite(eff::ConstantOp op, OpAdaptor adaptor,
    ConversionPatternRewriter &rewriter
    ) const final {
    auto constant = mlir::func::ConstantOp::create(rewriter, op.getLoc(), op->getResultTypes(), op.getValue());
    rewriter.replaceOp(op, constant);
    return success();
  }
};

struct ReturnOpLowering : public OpConversionPattern<eff::ReturnOp> {
  using OpConversionPattern<eff::ReturnOp>::OpConversionPattern;
  LogicalResult
  matchAndRewrite(eff::ReturnOp op, OpAdaptor adaptor,
    ConversionPatternRewriter &rewriter
    ) const final {
    auto ret = mlir::func::ReturnOp::create(rewriter, op.getLoc(), op.getOperands());
    rewriter.replaceOp(op, ret);
    return success();
  }
};


namespace {
struct EffToFuncLoweringPass : public PassWrapper<EffToFuncLoweringPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(EffToFuncLoweringPass)
  StringRef getArgument() const override { return "eff-to-func"; }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<func::FuncDialect>();
  }
  void runOnOperation() final;
};
}

void EffToFuncLoweringPass::runOnOperation() {
  ConversionTarget target(getContext());
  target.addLegalDialect<affine::AffineDialect, BuiltinDialect,
                         arith::ArithDialect, func::FuncDialect,
                         memref::MemRefDialect>();
  target.addIllegalDialect<eff::EffDialect>();

  RewritePatternSet patterns(&getContext());
  patterns.add<CallOpLowering, ConstantOpLowering, ReturnOpLowering, FuncOpLowering>(&getContext());


  if (failed(
          applyPartialConversion(getOperation(), target, std::move(patterns))))
    signalPassFailure();
}

std::unique_ptr<Pass> mlir::eff::createEffToFuncLoweringPass() {
  return std::make_unique<EffToFuncLoweringPass>();
}