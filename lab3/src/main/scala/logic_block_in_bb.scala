package garp

import Chisel._


class LogicBlockInBBModule(val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4) extends Module {

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

  val IN_MUX = Module(new LogicBlockInBBMuxModule(W, V, H, G)).io
  IN_MUX.V_wire_in := Bits(0)
  for(i <- 0 until V){
    IN_MUX.V_wire_in( W * i + 1 , W * i) := io.V_wire_in(i)
  }

  IN_MUX.H_wire_above_in := Bits(0)
  for(i <- 0 until H){
    IN_MUX.H_wire_above_in( W * i + 1 , W * i) := io.H_wire_above_in(i) 
  }

  IN_MUX.H_wire_below_in := Bits(0)
  for(i <- 0 until H){
    IN_MUX.H_wire_below_in( W * i + 1 , W * i) := io.H_wire_below_in(i) 
  }

  IN_MUX.G_wire_above_in := Bits(0)
  for(i <- 0 until G){
    IN_MUX.G_wire_above_in( W * i + 1 , W * i) := io.G_wire_above_in(i) 
  }

  IN_MUX.G_wire_below_in := Bits(0)
  for(i <- 0 until G){
    IN_MUX.G_wire_below_in( W * i + 1 , W * i) := io.G_wire_below_in(i) 
  }

  IN_MUX.config_X_in := Bits(0)
  for(i <- 0 until 4){
    IN_MUX.config_X_in( 6 * i + 5 , 6 * i) := io.config_X_in(i) 
  }

  //IN_MUX.X_in := Bits(0)
  for(i <- 0 until 4){
    io.X_in(i) :=  IN_MUX.X_in( W * i + 1 , W * i) 
  }

  IN_MUX.Z_reg := io.Z_reg
  IN_MUX.D_reg := io.D_reg

}

class LogicBlockInBBMuxModule(val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4) extends BlackBox() {
  val io = new Bundle {
    // 16 2-bit input
    val V_wire_in = Bits(INPUT, width=V * W)
    val H_wire_above_in = Bits(INPUT, width=W * H)
    val H_wire_below_in = Bits(INPUT, width=W * H)
    val G_wire_above_in = Bits(INPUT, width=W * G)
    val G_wire_below_in = Bits(INPUT, width=W * G)

    val config_X_in = Bits(INPUT, width=6 * 4)
    val Z_reg = Bits(INPUT, width=W)
    val D_reg = Bits(INPUT, width=W)

    val X_in = Bits(OUTPUT, width=W * 4)
  }

  io.V_wire_in.setName("V_wire_in")
  io.H_wire_above_in.setName("H_wire_above_in")
  io.H_wire_below_in.setName("H_wire_below_in")

  io.G_wire_above_in.setName("G_wire_above_in")
  io.G_wire_below_in.setName("G_wire_below_in")
  io.config_X_in.setName("config_X_in")
  io.Z_reg.setName("Z_reg")
  io.D_reg.setName("D_reg")
  io.X_in.setName("X_in")

  moduleName="input_mux"
}
 

