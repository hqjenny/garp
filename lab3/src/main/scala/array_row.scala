package garp

import Chisel._

// I is the index of the row
class ArrayRowModule (val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4, val I: Int) extends Module {
  val io = new Bundle {


  }


 


  val CB = Module(new ControlBlockModule()).io
  // 23 Logic Blocks per row
  val LB = Vec.fill(23){Module(new ControlReducerModule()).io}

  // Enable driver to a specific V wire, each block has 16 wires 
  val V_wire_en = Vec.fill(23){Bits(width=V)}

  val G_wire_below_en = Vec.fill(23){Bits(width=G)}

  // Like the G wires, each H wire can be driven by a logic block from above the wire and can be read by any block above or below the wire.
  val H_wire_below_en = Vec.fill(23){Bits(width=H)}

  
  for (i <- 0 until 23) {



  }

    // 16 2-bit input
    /*val V_wire_in = Vec.fill(V){Bits(INPUT, width=W)}
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
  }*/




  




}
