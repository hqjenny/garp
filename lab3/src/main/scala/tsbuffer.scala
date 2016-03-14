package garp

import Chisel._

class tsbufferBlackBox() extends BlackBox() {
  val io = new Bundle {
    val en = Bool(INPUT)
    val in = Bits(INPUT, 1)
    val out = Bits(OUTPUT, 1)
  }
  io.en.setName("en")
  io.in.setName("in")
  io.out.setName("out")

  moduleName="tsbuffer"
}
    
