package sha3

import Chisel._

class ChiModule(val W: Int = 64) extends Module {
  val io = new Bundle { 
    val state_i = Vec.fill(5*5){Bits(INPUT,W)}
    val state_o = Vec.fill(5*5){Bits(OUTPUT,W)}
  }
  //YOUR IMPLEMENTATION HERE
}
