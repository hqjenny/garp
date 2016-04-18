package garp

import Chisel._

class VwireBlackBox(val W: Int=2, val L: Int) extends Module() {
  val io = new Bundle {
    val in = Vec(L, Bits(INPUT, width=W))
    val en = Vec(L, Bits(INPUT, width=1))
    val out = Bits(OUTPUT, width=W)
  }

  val VwireBlockBB = Module(new VwireFlatBlackBox(W, L)).io
  VwireBlockBB.in := Bits(0)
  VwireBlockBB.en := Bits(0)

  for(i <- 0 until L){
    VwireBlockBB.in(W * i + 1, W * i) := io.in(i)
    VwireBlockBB.en(i) := io.en(i)
  }
  io.out := VwireBlockBB.out

}

class VwireFlatBlackBox(val W: Int=2, val L: Int=4) extends BlackBox() {
  val io = new Bundle {
    val in = Bits(INPUT, width=W*L)
    val en = Bits(INPUT, width=L)
    val out = Bits(OUTPUT, width=W)
  }

  io.en.setName("en")
  io.in.setName("in")
  io.out.setName("out")
  
  setVerilogParameters("#(.W("+W+"),.L("+L+"))")
  moduleName="vwire"
}

