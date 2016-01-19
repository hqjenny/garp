package sha3

import Chisel._

class PiModule(val W: Int = 64) extends Module {
  val io = new Bundle { 
    val state_i = Vec.fill(5*5){Bits(INPUT,width = W)}
    val state_o = Vec.fill(5*5){Bits(OUTPUT,width = W)}
  }

  //YOUR IMPLEMENTATION HERE
}
