package garp

import Chisel._

class TSBufferBlackBox(W: Int=2) extends BlackBox() {
  val io = new Bundle {
    val en = Bool(INPUT)
    val in = Bits(INPUT, W)
    val out = Bits(OUTPUT, W)
  }
  io.en.setName("en")
  io.in.setName("in")
  io.out.setName("out")

  setVerilogParameters("#(.W("+W+"))")
  moduleName="tsbuffer"
}
    
