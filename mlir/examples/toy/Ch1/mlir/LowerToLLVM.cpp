//
// Created by Rajan Maghera on 2025-09-23.
//

#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/TypeID.h"
#include "toy/Dialect.h"

#include "mlir/Conversion/AffineToStandard/AffineToStandard.h"
#include "mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVMPass.h"
#include "mlir/Conversion/LLVMCommon/ConversionTarget.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Conversion/MemRefToLLVM/MemRefToLLVM.h"
#include "mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/Casting.h"
#include <memory>
#include <utility>

namespace {
struct FunToLLVMLoweringPass :
public mlir::PassWrapper<FunToLLVMLoweringPass, mlir::OperationPass<mlir::ModuleOp>> {

  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(FunToLLVMLoweringPass)
  mlir::StringRef getArgument() const override { return "toy-to-llvm"; }

  void getDependentDialects(mlir::DialectRegistry &registry) const override {
    registry.insert<mlir::LLVM::LLVMDialect, mlir::scf::SCFDialect>();
  }
  void runOnOperation() final;
};
}

struct ConstantOpLowering : public mlir::OpConversionPattern<mlir::toy::ConstantOp> {
  using mlir::OpConversionPattern<mlir::toy::ConstantOp>::OpConversionPattern;

  llvm::LogicalResult
  matchAndRewrite(mlir::toy::ConstantOp op, OpAdaptor adaptor, mlir::ConversionPatternRewriter &rewriter) const final {
    auto loc = op->getLoc();
    rewriter.replaceOp(op,
    mlir::LLVM::ConstantOp::create(rewriter, loc, op.getValue()));
    return mlir::success();
  }
};

void FunToLLVMLoweringPass::runOnOperation() {
  mlir::ConversionTarget target(getContext());
  target.addLegalOp<mlir::ModuleOp>();
  target.addIllegalDialect<mlir::toy::FunDialect>();
  mlir::RewritePatternSet patterns(&getContext());
  patterns.add<ConstantOpLowering>(&getContext());

  // We want to completely lower to LLVM, so we use a `FullConversion`. This
  // ensures that only legal operations will remain after the conversion.
  auto module = getOperation();
  if (failed(applyFullConversion(module, target, std::move(patterns))))
    signalPassFailure();
}

