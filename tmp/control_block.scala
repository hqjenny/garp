package garp

import Chisel._

// Reduces two bit signals
class ControlReducer extends Module {
  val io = new Bundle {
    val a = Bits(INPUT, width=2)
    val select = Bits(INPUT, width=2)
    val a_prime = Bits(OUTPUT, width=1)
  }
  val a0 = io.a(0)
  val a1 = io.a(1)
  when (io.select === Bits(0, width=2)) {
    // When select is 00, A' = A0
    io.a_prime := a0
  } .elsewhen (io.select === Bits(4, width=2)) {
    // When select is 11, A' = A1
    io.a_prime := a1
  } .otherwise {
    // When select is 10, A' = A0 or A1
    // Technically, select being 01 is invalid
    io.a_prime := a0 | a1
  }
}

// Muxes between control inputs
class ControlMux extends Module {
  val io = new Bundle {
    val h_wire_above = Vec.fill(5){Bits(INPUT, width=2)}
    val h_wire_below = Vec.fill(5){Bits(INPUT, width=2)}
    val select = Bits(INPUT, width=6)
    val a = Bits(OUTPUT, width=2)
  }
  // Select which set of H wires to use
  val h_wires = Mux(io.select(4), io.h_wire_below, io.h_wire_above)
  when (io.select(5) === Bits(0, width=1)) {
    // Output a literal
    io.a := Cat(io.select(0), Bits(0, width=1))
  } .otherwise {
    // Output an H wire
    switch (io.select(3,1)) {
      is (Bits(1, width=3)) {
        io.a := h_wires(0)
      }
      is (Bits(2, width=3)) {
        io.a := h_wires(1)
      }
      is (Bits(3, width=3)) {
        io.a := h_wires(2)
      }
      is (Bits(4, width=3)) {
        io.a := h_wires(3)
      }
      is (Bits(5, width=3)) {
        io.a := h_wires(4)
      }
    }
  }
}


// TODO: Constants need to be discussed
class ControlBlock extends Module {
  val io = new Bundle { 
    val H_wire_above_in = Vec.fill(5){Bits(INPUT, width=2)}
    val H_wire_below_in = Vec.fill(5){Bits(INPUT, width=2)}
    // Configuration
    val config = Bits(INPUT, width=64)
    // Output
    // TODO: Find real names
    val Control_one = Bits(OUTPUT, width=1)
    val Control_two = Bits(OUTPUT, width=1)
    val Control_three = Bits(OUTPUT, width=1)
  }

  // Configuration encoding
  val mode = io.config(2,0)
  val Hdir = io.config(4,3)
  //TODO: Use these
  val mode_specific_field = io.config(31,5)
  val D_prime = io.config(33,32)
  val D_in = io.config(39,34)
  val C_prime = io.config(41,40)
  val C_in = io.config(47,42)
  val B_prime = io.config(49,48)
  val B_in = io.config(55,50)
  val A_prime = io.config(57,56)
  val A_in = io.config(63,58)

  // TODO: Make this not unrolled
  // Declare muxes
  val A_mux = Module(new ControlMux()).io
  val B_mux = Module(new ControlMux()).io
  val C_mux = Module(new ControlMux()).io
  val D_mux = Module(new ControlMux()).io

  // Declare reducers
  val A_reduce = Module(new ControlReducer()).io
  val B_reduce = Module(new ControlReducer()).io
  val C_reduce = Module(new ControlReducer()).io
  val D_reduce = Module(new ControlReducer()).io

  // Wire up muxes
  A_mux.h_wire_above := io.H_wire_above_in
  B_mux.h_wire_above := io.H_wire_above_in
  C_mux.h_wire_above := io.H_wire_above_in
  D_mux.h_wire_above := io.H_wire_above_in

  A_mux.h_wire_below := io.H_wire_below_in
  B_mux.h_wire_below := io.H_wire_below_in
  C_mux.h_wire_below := io.H_wire_below_in
  D_mux.h_wire_below := io.H_wire_below_in

  A_mux.select := A_in
  B_mux.select := B_in
  C_mux.select := C_in
  D_mux.select := D_in

  // Wire up reducers
  A_reduce.a := A_mux.a
  B_reduce.a := B_mux.a
  C_reduce.a := C_mux.a
  D_reduce.a := D_mux.a

  A_reduce.select := A_prime
  B_reduce.select := B_prime
  C_reduce.select := C_prime
  D_reduce.select := D_prime

  io.Control_one := A_reduce.a_prime & B_reduce.a_prime
  io.Control_two := A_reduce.a_prime & C_reduce.a_prime
  io.Control_three := A_reduce.a_prime & D_reduce.a_prime
}

