package garp

import Chisel._

// I is the index of the row
class ArrayRowModule (val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4, val I: Int) extends Module {
  val io = new Bundle {


  }
  // 1 Control Blocks per row
  val CB = Module(new ControlBlockModule()).io
  // 23 Logic Blocks per row
  val LB = Vec.fill(23){Module(new LogicBlockModule()).io}
  // 4 G Wires per row
  val G_wire_below =  Vec.fill(4){Bits(width=W)}
  // 31 H Wires per row, 11 rows  
  val H_wire_below =  Vec.fill(31){Bits(width=W)}

  // Enable driver to a specific V wire, each block has 16 wires 
  val V_wire_en = Vec.fill(23){Bits(width=V)}
  val G_wire_below_en = Vec.fill(23){Bits(width=G)}

  // Like the G wires, each H wire can be driven by a logic block from above the wire and can be read by any block above or below the wire.
  val H_wire_below_en = Vec.fill(23){Bits(width=H)}

  // JENNY not sure why the index is reversed
  for (i <- 0 until 23) {
    when (LB(i).config_G_out(2).toBool) {
      switch(LB(i).config_G_out(1,0)) {
        for(j <- 0 until 4) {
          is(Bits(j, width=2)) {
            // j: 0 -> 3 en: 3 -> 0
            G_wire_below_en(i)(3-j)
          }
        }
      }
    }

    when (LB(i).config_V_out(4).toBool) {
      switch(LB(i).config_V_out(3,0)) {
        for(j <- 0 until 16) {
          is(Bits(j, width=2)) {
            // j: 0 -> 3 en: 3 -> 0
            V_wire_en(i)(15-j)
          }
        }
      }
    }
  }
  
  // H wires
  switch(CB.Hdir){
    // Driven from right end (shift left)
    is(Bits(0, width=2)){

    }
    // Driven from center
    is(Bits(1, width=2)){

    }
    // Driven from left end (shift right)
    is(Bits(2, width=2)){

    }
  }
}
