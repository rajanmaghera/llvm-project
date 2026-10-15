# For a better template to copy, see examples/standalone
add_subdirectory(include)

set(LLVM_LINK_COMPONENTS
  Support
  )

add_toy_chapter(toyc-ch1
  toyc.cpp
  parser/AST.cpp
  mlir/MLIRGen.cpp
  mlir/FunCombine.cpp
  mlir/Dialect.cpp

  DEPENDS
  ToyCh1OpsIncGen
)

include_directories(include/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/)
target_link_libraries(toyc-ch1
  PRIVATE
    MLIRAnalysis
    MLIRFunctionInterfaces
    MLIRControlFlowInterfaces
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms)
