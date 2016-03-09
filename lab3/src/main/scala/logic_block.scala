package garp

import Chisel._

// W bit width, H is at most 12 
class LogicBlockModule(val W: Int=2, val V: Int=16, val H: Int=10, val G: Int=4) extends Module {

  val io = new Bundle { 
    // 16 2-bit input
    val V_wire_in = Vec.fill(V){Bits(INPUT, width=W)}
    val H_wire_above_in = Vec.fill(H){Bits(INPUT, width=W)}
    val H_wire_below_in = Vec.fill(H){Bits(INPUT, width=W)}
    val G_wire_above_in = Vec.fill(G){Bits(INPUT, width=W)}
    val G_wire_below_in = Vec.fill(G){Bits(INPUT, width=W)}
    val mem_bus_in = Bits(INPUT, width=W)

    // Shift_A_in, Shift_B_in, Shift_C_in
    val shift_X_in = Vec.fill(3){Bits(INPUT, width=1)}

    // Hout Above
    val H_out_above = Bits(INPUT, width=W)

    val shift_carry_in = Bits(INPUT, width=1)
    val carry_in = Bits(INPUT, width=1)

    // Shift_A_out, Shift_B_out, Shift_C_out
    val shift_X_out = Vec.fill(3){Bits(OUTPUT, width=1)}
    val shift_carry_out = Bits(OUTPUT, width=1)
    val carry_out = Bits(OUTPUT, width=1)

    // Control 
    // From control block config C' and D', whether access memory and Z or D
    val store_en = Bits(INPUT, width=1)
    val mem_D_or_Z = Bits(INPUT, width=1) // 0 = Z, 1 = D

    // Configuration
    val config = Bits(INPUT, width=64)

    val mem_bus_out = Bits(OUTPUT, width=W)
    val V_wire_out = Bits(OUTPUT, width=W)
    val H_wire_out = Bits(OUTPUT, width=W)
    val G_wire_out = Bits(OUTPUT, width=W)
  }
  // Data Signals
  val Z = Bits(0,W)
  val Z_reg_in = Bits(0,W)
  val D_reg_in = Bits(0,W)
  val Z_reg  = Reg(init = Bits(0,W))
  val D_reg  = Reg(init = Bits(0,W))
  val Z_out = Bits(0,W)
  val D_out = Bits(0,W)

  // Control Signals
  val Z_sel = Bool(false)
  Z_sel := io.config(12).toBool

  val D_sel = Bool(false)
  D_sel := io.config(11).toBool

 val H_sel = Bool(false)
  H_sel := io.config(10).toBool

 val G_sel = Bool(false)
  G_sel := io.config(9).toBool

 val V_sel = Bool(false)
  V_sel := io.config(8).toBool

  val config_G_out = Bits(width=3)
  config_G_out := io.config(7,5)

  val config_V_out = Bits(width=5)
  config_V_out := io.config(4,0)

  val store_Z = Bits(width=1)
  val store_D = Bits(width=1)

  store_Z := io.store_en & (~io.mem_D_or_Z)
  store_D := io.store_en & io.mem_D_or_Z
  

  // Configuration bit for input A,B,C,D addresses
  val config_X_in = Vec.fill(4){Bits(width=6)}
  config_X_in(0) := io.config(63, 58) 
  config_X_in(1) := io.config(55, 50)
  config_X_in(2) := io.config(47, 42)
  config_X_in(3) := io.config(39, 34)

  // A, B, C, D
  val X_in = Vec.fill(4){Bits(INPUT, width=W)}

  // Upper Muxes
  for(i <- 0 until 4) {
    // Take in 00, 10, Z_reg and D_reg
    when(config_X_in(i)(5,4) === Bits(0, width=2)) {
      switch(config_X_in(i)(1,0)){
        is(Bits(0, width=2)){
          X_in(i) := Bits(0) 
        }
        is(Bits(1, width=2)){
          X_in(i) := Bits(2) 
        }
        is(Bits(2, width=2)){
          X_in(i) := Z_reg
        }
        is(Bits(3, width=2)){
          X_in(i) := D_reg
        }
      }
      // V wires
    }.elsewhen(config_X_in(i)(5,4) === Bits(1, width=2)) {
      switch(config_X_in(i)(log2Up(V)-1,0)){
        for (j <- 0 until V){
          // For j=0, select V-1 for V_wire_in 
          is(Bits(j, width=log2Up(V))){
            X_in(i) := io.V_wire_in(V-j-1)
          }
        }
      }
      // H/G wires above
    }.elsewhen(config_X_in(i)(5,4) === Bits(2, width=2)) {
      // G wires
      when(config_X_in(i)(3,2) === Bits(3, width=2)){
        switch(config_X_in(i)(log2Up(G)-1,0)){
          for(j <- 0 until G){
            is(Bits(j, width=log2Up(G))){
              X_in(i) := io.G_wire_above_in(G-j-1)
            }
          }
        }
      }.otherwise{
        // H wires
        switch(config_X_in(i)(log2Up(H)-1,0)){
          for(j <- 0 until H){
            is(Bits(j, width=log2Up(H))){
              X_in(i) := io.H_wire_above_in(H-j-1)
            }
          }
        }
      }
    }
    // H/G wires below
  }.otherwise{
    // G wires
    when(config_X_in(i)(3,2) === Bits(3, width=2)){
      switch(config_X_in(i)(log2Up(G)-1,0)){
        for(j <- 0 until G){
          is(Bits(j, width=log2Up(G))){
            X_in(i) := io.G_wire_below_in(G-j-1)
          }
        }
      }
    }.otherwise{
      // H wires
      switch(config_X_in(i)(log2Up(H)-1,0)){
        for(j <- 0 until H){
          is(Bits(j, width=log2Up(H))){
            X_in(i) := io.H_wire_below_in(H-j-1)
          }
        }
      }
    }
  }
 
  // Functional Unit
  val FU = Module(new FunctionalUnitModule(W)).io
  FU.config := io.config
  FU.X_in := X_in
  FU.shift_X_in := io.shift_X_in
  FU.H_out_above := io.H_out_above
  FU.shift_carry_in := io.shift_carry_in
  FU.carry_in := io.carry_in


  io.shift_X_out := FU.shift_X_out
  io.shift_carry_out := FU.shift_carry_out
  io.carry_out := FU.carry_out
  Z := FU.Z

  Z_reg_in := Mux (store_Z.toBool, io.mem_bus_in, Z)
  D_reg_in := Mux (store_D.toBool, io.mem_bus_in, X_in(3)) // X_in(3) is D

  when(Z_sel){
    Z_reg := Z_reg_in
  }

  when(D_sel){
    D_reg := D_reg_in
  }

  io.mem_bus_out := Mux(io.mem_D_or_Z.toBool, D_reg, Z_reg)
  
  Z_out := Mux(Z_sel, Z_reg, Z)
  D_out := Mux(D_sel, D_reg, X_in(3))

  io.H_wire_out := Mux(H_sel, D_out, Z_out)
  io.G_wire_out := Mux(G_sel, D_out, Z_out)
  io.V_wire_out := Mux(V_sel, D_out, Z_out)
}
