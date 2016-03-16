package garp

import Chisel._

// W bit width, H is at most 12 
class LogicBlockInModule(val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4) extends Module {

  val io = new Bundle { 
    // 16 2-bit input
    val V_wire_in = Vec.fill(V){Bits(INPUT, width=W)}
    val H_wire_above_in = Vec.fill(H){Bits(INPUT, width=W)}
    val H_wire_below_in = Vec.fill(H){Bits(INPUT, width=W)}
    val G_wire_above_in = Vec.fill(G){Bits(INPUT, width=W)}
    val G_wire_below_in = Vec.fill(G){Bits(INPUT, width=W)}

    val config_X_in = Vec.fill(4){Bits(INPUT, width=6)}
    val Z_reg = Bits(INPUT, width=W)
    val D_reg = Bits(INPUT, width=W)

    val X_in = Vec.fill(4){Bits(OUTPUT, width=W)}
  }

  // Upper Muxes
  for(i <- 0 until 4) {
    io.X_in(i) := Bits(0)
    
    // Take in 00, 10, Z_reg and D_reg
    when(io.config_X_in(i)(5,4) === Bits(0, width=2)) {
      switch(io.config_X_in(i)(1,0)){
        is(Bits(0, width=2)){
          io.X_in(i) := Bits(0) 
        }
        is(Bits(1, width=2)){
          io.X_in(i) := Bits(2) 
        }
        is(Bits(2, width=2)){
          io.X_in(i) := io.Z_reg
        }
        is(Bits(3, width=2)){
          io.X_in(i) := io.D_reg
        }
      }
      // V wires
    }.elsewhen(io.config_X_in(i)(5,4) === Bits(1, width=2)) {
      switch(io.config_X_in(i)(log2Up(V)-1,0)){
        for (j <- 0 until V){
          // For j=0, select V-1 for V_wire_in 
          is(Bits(j, width=log2Up(V))){
            io.X_in(i) := io.V_wire_in(V-j-1)
          }
        }
      }
      // H/G wires above
    }.elsewhen(io.config_X_in(i)(5,4) === Bits(2, width=2)) {
      // G wires
      when(io.config_X_in(i)(3,2) === Bits(3, width=2)){
        switch(io.config_X_in(i)(log2Up(G)-1,0)){
          for(j <- 0 until G){
            is(Bits(j, width=log2Up(G))){
              io.X_in(i) := io.G_wire_above_in(G-j-1)
            }
          }
        }
      }.otherwise{
        // H wires
        switch(io.config_X_in(i)(log2Up(H)-1,0)){
          for(j <- 0 until H){
            is(Bits(j, width=log2Up(H))){
              io.X_in(i) := io.H_wire_above_in(H-j-1)
            }
          }
        }
      }
    }
    // H/G wires below
  }.otherwise{
    // G wires
    when(io.config_X_in(i)(3,2) === Bits(3, width=2)){
      switch(io.config_X_in(i)(log2Up(G)-1,0)){
        for(j <- 0 until G){
          is(Bits(j, width=log2Up(G))){
            io.X_in(i) := io.G_wire_below_in(G-j-1)
          }
        }
      }
    }.otherwise{
      // H wires
      switch(io.config_X_in(i)(log2Up(H)-1,0)){
        for(j <- 0 until H){
          is(Bits(j, width=log2Up(H))){
            io.X_in(i) := io.H_wire_below_in(H-j-1)
          }
        }
      }
    }
  }
} 
