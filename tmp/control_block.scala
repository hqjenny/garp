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
    val h_wire_above = Vec.fill(9){Bits(INPUT, width=2)}
    val h_wire_below = Vec.fill(9){Bits(INPUT, width=2)}
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
      for (i <- 0 until 9) {
        is (Bits(i + 2, width=3)) {
          io.a := h_wires(i)
        }
      }
    }
  }
}


// TODO: Constants need to be discussed
class ControlBlock extends Module {
  val io = new Bundle { 
    val H_wire_above_in = Vec.fill(9){Bits(INPUT, width=2)}
    val H_wire_below_in = Vec.fill(9){Bits(INPUT, width=2)}
    // Configuration
    val config = Bits(INPUT, width=64)
    // Output
    val B_prime = Bits(OUTPUT, width=1)
    val C_prime = Bits(OUTPUT, width=1)
    val D_prime = Bits(OUTPUT, width=1)
  }

  val indexes = Vec.fill(4){Bits(width=6)}
  val primes = Vec.fill(4){Bits(width=2)}

  // Configuration encoding
  val mode = io.config(2,0)
  val Hdir = io.config(4,3)
  //TODO: Use mode_specific fields
  val mode_specific_field = io.config(31,5)
  for (i <- 0 until 4) {
    indexes(i) := io.config(63 - (i * 8), 58 - (i * 8))
    primes(i) := io.config(57 - (i * 8), 56 - (i * 8))
    
  }

  // Declare muxes
  val muxes = Vec.fill(4){Module(new ControlMux()).io}

  // Declare reducers
  val reducers = Vec.fill(4){Module(new ControlReducer()).io}

  // Wire up muxes
  for (i <- 0 until 4) {
    muxes(i).h_wire_above := io.H_wire_above_in
    muxes(i).h_wire_below := io.H_wire_below_in
    muxes(i).select := indexes(i)
  }

  // Wire up reducers
  for (i <- 0 until 4) {
    reducers(i).a := muxes(i).a
    reducers(i).select := primes(i)
  }

  io.B_prime := reducers(0).a_prime & reducers(1).a_prime
  io.C_prime := reducers(0).a_prime & reducers(2).a_prime
  io.D_prime := reducers(0).a_prime & reducers(3).a_prime
}

