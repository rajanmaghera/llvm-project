builtin.module {
  func.func @second_main(%arg0 : i64) -> i64 {
    %0 = arith.constant 32 : i64
    %1 = arith.muli %arg0, %0 : i64
    %2 = arith.constant 64 : i64
    %3 = arith.muli %arg0, %2 : i64
    %4 = arith.addi %1, %3 : i64
    func.return %4 : i64
  }
  "llvm.func"() <{sym_name = "atoi", function_type = !llvm.func<i64 (!llvm.ptr<i8>)>, CConv = #llvm.cconv<ccc>, linkage = #llvm.linkage<"internal">, visibility_ = 0 : i64}> ({
  }) : () -> ()
  "llvm.func"() <{sym_name = "printf", function_type = !llvm.func<i64 (!llvm.ptr<i8>, i64)>, CConv = #llvm.cconv<ccc>, linkage = #llvm.linkage<"internal">, visibility_ = 0 : i64}> ({
  }) : () -> ()
  "llvm.mlir.global"() <{global_type = !llvm.array<5 x i8>, sym_name = "fmt_str", linkage = #llvm.linkage<"internal">, addr_space = 0 : i32, constant, value = "%ld\n\u0000"}> ({
  }) : () -> ()
  "llvm.func"() <{sym_name = "main", function_type = !llvm.func<i64 (i64, !llvm.ptr<!llvm.ptr<i8>>)>, CConv = #llvm.cconv<ccc>, linkage = #llvm.linkage<"internal">, visibility_ = 0 : i64}> ({
  ^bb0(%argc : i64, %argv : !llvm.ptr<!llvm.ptr<i8>>):
    %0 = "llvm.getelementptr"(%argv) <{rawConstantIndices = array<i32: 1>}> : (!llvm.ptr<!llvm.ptr<i8>>) -> !llvm.ptr<!llvm.ptr<i8>>
    %1 = "llvm.load"(%0) <{ordering = 0 : i64}> : (!llvm.ptr<!llvm.ptr<i8>>) -> !llvm.ptr<i8>
    %2 = "llvm.call"(%1) <{callee = "atoi", fastmathFlags = #llvm.fastmath<none>, CConv = #llvm.cconv<ccc>, op_bundle_sizes = array<i32>, operandSegmentSizes = array<i32: 1, 0>, TailCallKind = #llvm.tailcallkind<none>}> : (!llvm.ptr<i8>) -> i64
    %3 = "llvm.call"(%2) <{callee = "second_main", fastmathFlags = #llvm.fastmath<none>, CConv = #llvm.cconv<ccc>, op_bundle_sizes = array<i32>, operandSegmentSizes = array<i32: 1, 0>, TailCallKind = #llvm.tailcallkind<none>}> : (i64) -> i64
    %4 = "llvm.mlir.addressof"() <{global_name = @fmt_str}> : () -> !llvm.ptr<!llvm.array<5 x i8>>
    %5 = llvm.bitcast %4 : !llvm.ptr<!llvm.array<5 x i8>> to !llvm.ptr<i8>
    %6 = "llvm.call"(%5, %3) <{callee = "printf", fastmathFlags = #llvm.fastmath<none>, CConv = #llvm.cconv<ccc>, op_bundle_sizes = array<i32>, operandSegmentSizes = array<i32: 2, 0>, TailCallKind = #llvm.tailcallkind<none>}> : (!llvm.ptr<i8>, i64) -> i64
    "llvm.return"(%7) : (i64) -> ()
  }) : () -> ()
}