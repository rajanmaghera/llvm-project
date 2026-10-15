builtin.module {
  func.func @myfunc() -> i1 !eff.effect<@io_read : () -> i64> {
    // effect @io_debug writes number to console and returns the same number
    // %0 (i1) gets false if successful, true if error
    // !eff.cont is the continuation type
    %0 = eff.handle !eff.effect<@io_debug : (i64) -> i64> (%k : !eff.cont, %handler0 : i64) -> (i1) {
        // ... write %handler0 to console
        // if successful:
        eff.continue %k %handler0
        // if error:
        %1 = arith.constant true : i1
        eff.discontinue %k %1 : !eff.cont
    } {
        %2 = eff.do @io_read() : () -> i64
        %3 = eff.do @io_debug(%2) : (i64) -> i64
        // ...
        %4 = arith.constant false : i1
        eff.yield %4 : i1
    }
    func.return %0 : i1
  }
}