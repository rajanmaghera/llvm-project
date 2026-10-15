
set(LLVM_TARGET_DEFINITIONS EffOps.td)
mlir_tablegen(EffOps.h.inc -gen-op-decls)
mlir_tablegen(EffOps.cpp.inc -gen-op-defs)
mlir_tablegen(EffOpsDialect.h.inc -gen-dialect-decls)
mlir_tablegen(EffOpsDialect.cpp.inc -gen-dialect-defs)
add_public_tablegen_target(EffOpsIncGen)

