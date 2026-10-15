# For a better template to copy, see examples/standalone
include_directories(include)
add_subdirectory(include)

set(LLVM_LINK_COMPONENTS
  Support
  )

add_toy_chapter(toyc-ch0eff
  effc.cpp
  mlir/MLIRGen.cpp
  mlir/EffOps.cpp
        mlir/LowerToFuncPass.cpp

  DEPENDS
  EffOpsIncGen
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/)
target_link_libraries(toyc-ch0eff
  PRIVATE
    MLIRAnalysis
    MLIRCallInterfaces
    MLIRCastInterfaces
    MLIRFunctionInterfaces
    MLIRIR
    MLIRParser
    MLIRPass
    MLIRRegisterAllDialects
    MLIRRegisterAllExtensions
    MLIRSideEffectInterfaces
    MLIRSupport
    MLIRTransforms
  )
