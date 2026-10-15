# A basic DSL that lowers to MLIR code

from abc import ABC, abstractmethod

from xdsl.dialects.builtin import ModuleOp, IntegerAttr, NoneAttr, MemRefType, i8, i64, StringAttr
from xdsl.dialects.arith import AddiOp, ConstantOp, MuliOp
from xdsl.dialects.func import FuncOp, ReturnOp, CallOp
from xdsl.ir import Attribute, SSAValue
from xdsl.rewriter import InsertPoint, BlockInsertPoint
from xdsl.builder import Builder
from xdsl.printer import Printer

from xdsl.dialects import llvm, memref


class Context:
    module: ModuleOp
    builder: Builder
    main_func: FuncOp

    def __init__(self, arg_count: int):
        self.module = ModuleOp([])
        self.builder = Builder(InsertPoint.at_end(self.module.body.block))
        arg_types = [i64] * arg_count
        self.main_func = FuncOp("second_main", (arg_types, [i64]))
        for idx, arg in enumerate(self.main_func.args):
            arg.name_hint = f"arg{idx}"
        self.builder.insert(self.main_func)
        self.builder.insertion_point = InsertPoint.at_end(
            self.main_func.body.block)

    def build_module(self, result: SSAValue) -> ModuleOp:
        self.builder.insert(ReturnOp(result))

        # This is not the main func, the new main function
        # will format the inputs and return the output
        # This will mainly be done in the LLVM dialect to make
        # lowering easier
        self.builder.insertion_point = InsertPoint.at_end(
            self.module.body.block)

        # Declare external functions
        ptr_type = llvm.LLVMPointerType(i8, NoneAttr())

        self.builder.insert(llvm.FuncOp(
            "atoi",
            llvm.LLVMFunctionType(
                [llvm.LLVMPointerType.opaque()], i64
            ),
            linkage=llvm.LinkageAttr("external"),
        ))

        self.builder.insert(llvm.FuncOp(
            "printf",
            llvm.LLVMFunctionType(
                [llvm.LLVMPointerType.opaque()], is_variadic=True
            ),
            linkage=llvm.LinkageAttr("external"),
        ))

        # constant printf string
        fmt_string = llvm.GlobalOp(llvm.LLVMArrayType(
            llvm.IntAttr(4), i8), "fmt_str", "internal", constant=True, value=StringAttr("%ld\n"))
        self.builder.insert(fmt_string)

        # Main function begins here
        m = llvm.FuncOp("main", llvm.LLVMFunctionType(
            [i64, llvm.LLVMPointerType.opaque()], i64), linkage=llvm.LinkageAttr("external"))
        self.builder.insert(m)
        blk = self.builder.create_block(BlockInsertPoint.at_start(m.body))
        # insert args into blk
        argc = blk.insert_arg(i64, 0)
        argc.name_hint = "argc"
        argv = blk.insert_arg(llvm.LLVMPointerType.opaque(), 1)
        argv.name_hint = "argv"

        # TODO assertions
        # Read args from input strings
        arg_vals = []
        # for each argument in our main, emit code to read it and push the read value to arg_vals
        for i in range(len(self.main_func.args)):
            # this iteration reads `argv[i+1]`
            # get pointer to argument string
            arg_ptr = self.builder.insert(llvm.GEPOp(
                argv, [i + 1], pointee_type=llvm.LLVMPointerType.opaque()))
            argstr = self.builder.insert(llvm.LoadOp(
                arg_ptr, result_type=llvm.LLVMPointerType.opaque()))
            arg_vals.append(self.builder.insert(
                llvm.CallOp("atoi", argstr, return_type=i64)))

        # Call second_main with the read arguments
        call = self.builder.insert(CallOp(
            "second_main", arg_vals, return_types=[i64]
        ))

        # Print the result
        fmt_ptr = self.builder.insert(llvm.AddressOfOp(
            fmt_string.sym_name, result_type=llvm.LLVMPointerType.opaque()))
        self.builder.insert(llvm.CallOp(
            "printf", fmt_ptr, call, variadic_args=1))

        # return 0
        zer = self.builder.insert(llvm.ConstantOp(IntegerAttr(0, i64), i64))

        self.builder.insert(llvm.ReturnOp.build(
            operands=[zer]))

        # self.module.verify()
        return self.module


class Expr(ABC):
    def build_module(self, ctx: Context):
        return ctx.build_module(self.lower(ctx))

    @abstractmethod
    def lower(self, ctx: Context):
        pass

    @abstractmethod
    def get_mlir_type(self) -> Attribute:
        pass

    @abstractmethod
    def display(self) -> str:
        pass

    def __add__(self, o):
        return Add(self, o)

    def __mul__(self, o):
        return Mul(self, o)


# TODO ensure no cycles?
# TODO each unique constructor creates single value, then is referenced


class ConstInt(Expr):
    num: int

    def __init__(self, num: int):
        self.num = num

    def lower(self, ctx: Context):
        return ctx.builder.insert(ConstantOp(IntegerAttr(self.num, self.get_mlir_type())))

    def get_mlir_type(self) -> Attribute:
        return i64

    def display(self) -> str:
        return f"Int({self.num})"


class ConstArg(Expr):
    idx: int

    def __init__(self, idx: int):
        self.idx = idx

    def lower(self, ctx: Context):
        return ctx.main_func.args[self.idx]

    def get_mlir_type(self) -> Attribute:
        return i64

    def display(self) -> str:
        return f"Arg[{self.idx}]"


class Mul(Expr):
    lhs: Expr
    rhs: Expr

    def __init__(self, lhs: Expr, rhs: Expr):
        self.lhs = lhs
        self.rhs = rhs
        assert self.lhs.get_mlir_type() == self.rhs.get_mlir_type()

    def lower(self, ctx: Context):
        lhs_lower = self.lhs.lower(ctx)
        rhs_lower = self.rhs.lower(ctx)
        return ctx.builder.insert(MuliOp(lhs_lower, rhs_lower, self.get_mlir_type()))

    def get_mlir_type(self) -> Attribute:
        return self.lhs.get_mlir_type()

    def display(self) -> str:
        return f"Mul({self.lhs.display()} + {self.rhs.display()})"


class Add(Expr):
    lhs: Expr
    rhs: Expr

    def __init__(self, lhs: Expr, rhs: Expr):
        self.lhs = lhs
        self.rhs = rhs
        assert self.lhs.get_mlir_type() == self.rhs.get_mlir_type()

    def lower(self, ctx: Context):
        lhs_lower = self.lhs.lower(ctx)
        rhs_lower = self.rhs.lower(ctx)
        return ctx.builder.insert(AddiOp(lhs_lower, rhs_lower, self.get_mlir_type()))

    def get_mlir_type(self) -> Attribute:
        return self.lhs.get_mlir_type()

    def display(self) -> str:
        return f"Add({self.lhs.display()} + {self.rhs.display()})"


def my_func(arg0, arg1):
    return (arg0 + arg1) * ConstInt(2)


def main():
    ctx = Context(2)
    my_expr = my_func(ConstArg(0), ConstArg(1))
    mod = my_expr.build_module(ctx)
    printer = Printer(print_generic_format=False)
    printer.print_op(mod)


if __name__ == "__main__":
    main()
