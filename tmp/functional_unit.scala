package garp

import Chisel._

class CrossbarModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Bits(INPUT, width=W) 
    val sel = Bits(INPUT, width=2)
    val out = Bits(OUTPUT, width=W) 
  }
  for(i <- 0 until 2){
    when (io.sel(i) === Bits(0, width=1)){
      io.out(i) := io.in(0)
    }.otherwise {
      io.out(i) := io.in(1)
    }
  }
}

class ShiftInvertModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val shift_in = Bits(INPUT, width=1) 
    val in = Bits(INPUT, width=W) 
    val sel = Bits(INPUT, width=2)
    val out = Bits(OUTPUT, width=W) 
  }
  
  switch(io.sel){
    is(Bits(0, width=2)){
      io.out(0):= io.in(0)
      io.out(1):= io.in(1)
    }
    is(Bits(1, width=2)){
      io.out(0):= ~io.in(0)
      io.out(1):= ~io.in(1)
    }
    is(Bits(2, width=2)){
      io.out(0):= io.shift_in
      io.out(1):= io.in(0)
    }
    is(Bits(3, width=2)){
      io.out(0):= ~io.shift_in
      io.out(1):= ~io.in(0)
    }
  }
}

// 
class CarrySaveAdderModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val shift_carry_in = Bits(INPUT, width=1)
    val in = Vec.fill(3){Bits(INPUT, width=W)}
    val sum = Vec.fill(2){Bits(INPUT, width=W)}
    val carry = Vec.fill(2){Bits(INPUT, width=W)}
    val shift_carry_out = Bits(OUTPUT, width=1)
  }

    val carry = Vec.fill(2){Bits(width=W)}
  for(i <- 0 until 2){
    io.sum(i):= io.in(2)^io.in(1)^io.in(0)
    carry := (io.in(2)&io.in(1)) | (io.in(1)&io.in(0)) | 
(io.in(2)&io.in(0)) 
 

  }
}


class FunctionalUnitModule(val W: Int=2) extends Module {

  val io = new Bundle { 
    val X_in = Vec.fill(4){Bits(INPUT, width=W)}
    val config = Bits(INPUT, width=64)
    val Z = Bits(INPUT, width=W)
  }




}
