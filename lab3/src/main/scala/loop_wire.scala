package garp

import Chisel._

class LoopWireBlackBox(val W: Int=2) extends BlackBox() {
  val io = new Bundle {
    val in = Bits(INPUT, W)
    val out = Bits(OUTPUT, W)
  }
  io.in.setName("in")
  io.out.setName("out")
  io.out := Bits(0)

  moduleName="loop_wire"
}
  
