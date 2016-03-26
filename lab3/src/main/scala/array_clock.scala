package garp

import Chisel._

// Reduces two bit signals
class ArrayClockCounterModule extends Module {
  val io = new Bundle {
    val reset = Bool(INPUT)
    val set = Bool(INPUT)
    val in = UInt(INPUT, width=32)
    val out = UInt(OUTPUT, width=32)
  }

// mfga
// gabump

// Bit 31 is the sticky bit 
// once set, it remains set until the entire counter is forcibly zeroed either by the array or by the processor (gast op).
  val counter_reg  = Reg(init = UInt(0,32))
  when(io.reset){
    counter_reg := UInt(0,32) 
  }.elsewhen(io.set){
    counter_reg := io.in
  }.otherwise{
    counter_reg := Cat(counter_reg(31), counter_reg(30,0) - UInt(1, 31))
  }

  io.out := counter_reg

}
