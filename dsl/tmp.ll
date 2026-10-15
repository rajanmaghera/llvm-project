; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

@fmt_str = internal constant [4 x i8] c"%ld\0A"

define i64 @second_main(i64 %0) {
  %2 = mul i64 %0, 32
  %3 = mul i64 %0, 64
  %4 = add i64 %2, %3
  ret i64 %4
}

declare i64 @atoi(ptr)

declare void @printf(ptr, ...)

define i64 @main(i64 %0, ptr %1) {
  %3 = getelementptr ptr, ptr %1, i32 1
  %4 = load ptr, ptr %3, align 8
  %5 = call i64 @atoi(ptr %4)
  %6 = call i64 @second_main(i64 %5)
  call void (ptr, ...) @printf(ptr @fmt_str, i64 %6)
  ret i64 0
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
