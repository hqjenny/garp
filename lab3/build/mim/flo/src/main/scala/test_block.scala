package garp

import Chisel._

class TestBlockModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Bits(INPUT, width=W) 
    val sel = Bits(INPUT, width=2)
    val out = Bits(OUTPUT, width=W) 
  }

  //val tsb = Module(new TSBufferBlackBox(W)).io
  //tsb.in :=  io.in 
  //tsb.en := io.sel(0)
  io.out := io.in & io.sel

  /*io.out := Bits(0)
  for(i <- 0 until 2){
    when (io.sel(i) === Bits(0, width=1)){
      io.out(i) := io.in(0)
    }.otherwise {
      io.out(i) := io.in(1)
    }
  }*/
}

