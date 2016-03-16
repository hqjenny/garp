package garp

import Chisel._

class ConfigAddrDecoder() extends Module {
  val io = new Bundle {
    val addr = Bits(INPUT, width=5) 
    val en = Bits(OUTPUT, width=32) 
  }
  io.en := Bits(0) 
  for(i <- 0 until 32){
    when(io.addr === Bits(i)){
      io.en(i) := Bits(1, width=1) 
    }
  }
}


class ConfigurationModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Bits(INPUT, width=W) 
    val en = Bits(INPUT, width=1)
    val out = Bits(OUTPUT, width=64) 
  }
 
  val config_reg  = Reg(init = Bits(0,64))

  when(io.en.toBool){
    config_reg := config_reg >> 2
    config_reg(63) := io.in(1)
    config_reg(62) := io.in(0)
  }
  io.out := config_reg
}
