package garp

import Chisel._

class GWireModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val G_in = Vec.fill(23){Bits(INPUT, width=W)}
    val en = Bits(INPUT, width=23)
    val G_out = Bits(OUTPUT, width=W)
  }

  io.G_out := Bits(0)
  for(i <- 0 until 23) {
    when(io.en(i).toBool){
      io.G_out := io.G_in(i) 
    }
  }
}

// I is the index of the row
class ArrayRowModule (val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4, val I: Int) extends Module {
  val io = new Bundle {
    // 23 x V wires
    val V_wire_in = Vec.fill(23*V){Bits(INPUT, width=W)}
    val V_wire_out = Vec.fill(23*V){Bits(OUTPUT, width=W)}
    val V_wire_en = Vec.fill(23){Bits(OUTPUT, width=V)}

    val G_wire_above =  Vec.fill(G){Bits(INPUT, width=W)}
    val H_wire_above =  Vec.fill(33){Bits(INPUT, width=W)}

    // Assume 1 mem bus is allowed
    val mem_bus_in = Vec.fill(24){Bits(INPUT, width=W)}

    // H output of the block immediately above 
    val H_out_above =  Vec.fill(23){Bits(INPUT, width=W)}
    val H_out =  Vec.fill(23){Bits(OUTPUT, width=W)}    

    // 4 G Wires per row
    val G_wire_below =  Vec.fill(G){Bits(OUTPUT, width=W)}
    // 33 H Wires per row, 11 rows, 2 is not driven by anything  
    val H_wire_below =  Vec.fill(33){Bits(OUTPUT, width=W)}

    // Control port
    val row_en = Bits(INPUT, width=1)


    // Port for testing 
    /*val config = Vec.fill(24){Bits(INPUT, width=64)}
    val Z_in = Vec.fill(23){Bits(INPUT, width=W)}
    val D_in = Vec.fill(23){Bits(INPUT, width=W)}
    val Z_out = Vec.fill(23){Bits(OUTPUT, width=W)}
    val D_out = Vec.fill(23){Bits(OUTPUT, width=W)}
    val test = Bool(INPUT)*/

    // Assume 1 mem bus is allowed
    val mem_bus_out = Vec.fill(24){Bits(OUTPUT, width=W)}
  }

  val config = Vec.fill(24){Bits(width=64)}
  val CM = Vec.fill(24){Module(new ConfigurationModule()).io}
  for(i <-0 until 24){
    CM(i).en := io.row_en
    CM(i).in := io.mem_bus_in(i) 
    config(i) := CM(i).out
  }

  //printf("index%d\n", UInt(I));
  // 1 Control Blocks per row
  val CB = Module(new ControlBlockModule()).io
  // 23 Logic Blocks per row
  val LB = Vec.fill(23){Module(new LogicBlockModule()).io}
  // Enable driver to a specific V wire, each block has 16 wires 
  val V_wire_en = Vec.fill(23){Bits(width=V)}
  io.V_wire_en := V_wire_en
  val G_wire_below_en = Vec.fill(G){Bits(width=23)}

  // 4 G Wires per row
  val G_wire_below =  Vec.fill(G){Bits(width=W)}
  // 33 H Wires per row, 11 rows, 2 is not driven by anything  
  val H_wire_below =  Vec.fill(33){Bits(width=W)}
  io.H_wire_below := H_wire_below
  io.G_wire_below := G_wire_below
  // Initialize H_wire_below
  for (i <- 0 until 33){
    H_wire_below(i) := Bits(0)
  }

  //H_wire_below := Bits(0,width=W*33)
  
  // V Wires
  for (i <- 0 until 23) {
    for (j <- 0 until V) {
      LB(i).V_wire_in(j) := io.V_wire_in(i * V + j)
      io.V_wire_out(i * V + j) := LB(i).V_wire_out
    }
  }

  // Making the assumption for now as only H9-1 have driver
  for (i <- 0 until 9) {
    //CB.H_wire_above_in(i) := io.H_wire_above(9,1) 
    CB.H_wire_above_in(i) := io.H_wire_above(i+1) 
    CB.H_wire_below_in(i) := H_wire_below(i+1) 
  } 
  
  for (i <- 0 until 23) {
    LB(i).G_wire_above_in := io.G_wire_above
    LB(i).G_wire_below_in := G_wire_below
    for(j <- 0 until 11) {
      //LB(i).H_wire_above_in := io.H_wire_above(32-i,22-i)    
      LB(i).H_wire_above_in(j) := io.H_wire_above(22-i+j)    
      LB(i).H_wire_below_in(j) := H_wire_below(22-i+j)
    }
  }

  // mem_bus_in
  for (i <- 0 until 23) {
    LB(i).mem_bus_in := io.mem_bus_in(i)
    io.mem_bus_out(i) := LB(i).mem_bus_out
  }
  LB(0).shift_X_in := Bits(0)
  LB(0).shift_carry_in := Bits(0)
  LB(0).carry_in := Bits(0)
  // shift_X, shift_carry, carry
  for (i <- 1 until 23) {
    LB(i).shift_X_in := LB(i-1).shift_X_out
    LB(i).shift_carry_in := LB(i-1).shift_carry_out
    LB(i).carry_in := LB(i-1).carry_out
  }

  // TEST config just for testing
  /*CB.config := io.config(0)
  for (i <- 0 until 23) {
    // 22 - 0 : 1 - 23
    LB(22-i).config := io.config(i+1)
  }

  // TEST
  for (i <- 0 until 23) {
    LB(i).Z_in := io.Z_in(i)
    LB(i).D_in := io.D_in(i)
    LB(i).test := io.test
    io.Z_out(i) := LB(i).Z_out
    io.D_out(i) := LB(i).D_out
  }*/

  for (i <- 0 until 23) {
    LB(i).mem_D_or_Z := CB.mem_D_or_Z
    LB(i).store_en := CB.store_transfer_access
  }
  // Like the G wires, each H wire can be driven by a logic block from above the wire and can be read by any block above or below the wire.
  // JENNY not sure why the index is reversed
  // G wires
  val G_wire_outs = Vec.fill(23){Bits(INPUT, width=W)}
  for (i <- 0 until 23) {
    G_wire_outs(i) := LB(i).G_wire_out
  }

  for(i <- 0 until G) {
    G_wire_below_en := Bits(0)
  }
  for (i <- 0 until 23) {
    when (LB(i).config_G_out(2).toBool) {
      switch(LB(i).config_G_out(1,0)) {
        for(j <- 0 until G) {
          is(Bits(j, width=2)) {
            // j: 0 -> 3 en: 3 -> 0
            G_wire_below_en(3-j)(i) := Bits(x=1, width=1) 
          }
        }
      }
    }
  }

  val GWireBulk = Vec.fill(G){Module(new GWireModule()).io}
  val GWireBB = Vec.fill(G){Module(new LoopWireBlackBox()).io}

  for (i <- 0 until G) {
    GWireBulk(i).G_in := G_wire_outs
    GWireBulk(i).en := G_wire_below_en(i)
    // Blackbox G wire connection
    GWireBB(i).in := GWireBulk(i).G_out 
    G_wire_below(i) := GWireBB(i).out 
  }

  // V wires
  for(i <- 0 until 23){
    V_wire_en(i) := Bits(0)
  }

  for (i <- 0 until 23) {
    when (LB(i).config_V_out(4).toBool) {
      switch(LB(i).config_V_out(3,0)) {
        for(j <- 0 until V) {
          is(Bits(j, width=4)) {
            // j: 0 -> 3 en: 3 -> 0
            //V_wire_en(15-j)(i) := Bits(x=1, width=1)
            V_wire_en(i)(15-j) := Bits(x=1, width=1)
          }
        }
      }
    }
  }
  
  val HWireBB = Vec.fill(33){Module(new LoopWireBlackBox()).io}
  

  for (i <- 0 until 33){
    HWireBB(i).in := Bits(0, width=2)
  }
  // H wires
  switch(CB.Hdir){
    // Driven from right end (shift left)
    is(Bits(0, width=2)){
      for (i <- 0 until 23) {
        //HWireBB(i + 1).in := LB(i).H_wire_out 
        HWireBB(23 - i).in := LB(i).H_wire_out 
      }
    }
    // Driven from center
    is(Bits(1, width=2)){
       for (i <- 0 until 23) {
        //HWireBB(i + 5).in := LB(i).H_wire_out 
        HWireBB(27 - i).in := LB(i).H_wire_out 
      }
    }
    // Driven from left end (shift right)
    is(Bits(2, width=2)){
      for (i <- 0 until 23) {
        //HWireBB(i + 9).in := LB(i).H_wire_out 
        HWireBB(31 - i).in := LB(i).H_wire_out 
      }
    }
  }

  for (i <- 0 until 33){
    H_wire_below(i) := HWireBB(i).out
  }

  // Output H wire
  for (i <- 0 until 23) {
    io.H_out(i) := LB(i).H_wire_out    
    LB(i).H_out_above := io.H_out_above(i)
  }

}

