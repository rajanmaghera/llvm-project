//===- Ops.td - Toy dialect operation definitions ----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines the operations of the Toy dialect.
//
//===----------------------------------------------------------------------===//

#ifndef FUN_OPS
#define FUN_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"
include "mlir/IR/BuiltinAttributeInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/IR/AttrTypeBase.td"

def Fun_Dialect : Dialect {
  let name = "fun";
  let cppNamespace = "::mlir::toy";

  // We set this bit to generate the declarations for the dialect's type parsing
  // and printing hooks.
  let useDefaultTypePrinterParser = 1;

}


class Fun_Op<string mnemonic, list<Trait> traits = [IsolatedFromAbove]> :
    Op<Fun_Dialect, mnemonic, traits>;

//class FunType<string name, string typeMnemonic, list<Trait> traits = []>:
//    TypeDef<Fun_Dialect, name, traits> {
//    let mnemonic = typeMnemonic;
//    }
//

def Fun_RetType:
    DialectType<Fun_Dialect, CPred<"isa<RetType>($_self)">,
                "Return value type">;

// A captureless closure
def ClosureOp : Fun_Op<"closure", [Pure]> {
  let summary = "closure";
  let description = [{
    Closure defines a functional closure that returns a value.
  }];

  let arguments = (ins);
  let results = (outs Fun_RetType:$closure);
  let regions = (region SizedRegion<1>:$body);
  let hasVerifier = 1;
  let assemblyFormat = [{
    $body attr-dict `:` type($closure)
  }];
}

def EvalOp : Fun_Op<"eval", []> {
  let summary = "eval";
  let description = [{
    Eval defines an evaluation of a closure and returns the computation.
  }];
    let hasVerifier = 1;
    let arguments = (ins Fun_RetType:$fn);
    let results   = (outs AnyType:$results);
    let assemblyFormat = [{
        type($fn) $fn attr-dict `:` type($results)
    }];
}

def ReturnOp : Fun_Op<"return", [Terminator, ReturnLike, HasParent<"ClosureOp">]> {
   let summary = "return";
   let description = [{
        Return operator for a closure.
   }];
   let arguments = (ins AnyType:$results);
   let results = (outs);

  let assemblyFormat = "attr-dict $results `:` type($results)";
}


def PrintOp : Fun_Op<"print", []> {
    let summary = "print";
    let description = [{
        Print value
    }];
    let arguments = (ins AnyType:$value);
    let results = (outs);
    let assemblyFormat = [{
        attr-dict $value `:` type($value)
    }];
}

// ConstantOp should be ConstantLike, but the canonicalizer won't let us pass
// since we don't have any rewrites on it
def ConstantOp : Fun_Op<"constant", [Pure]> {
    let summary = "constant";
    let description = [{
        Any constant value
    }];
    let arguments = (ins TypedAttrInterface:$value);
    let results = (outs AnyType:$result);
    let assemblyFormat = [{
        attr-dict $value `:` type($result)
    }];
    let hasVerifier = 1;

}

#endif // FUN_OPS
