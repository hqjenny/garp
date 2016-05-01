package garp

import Chisel._

// Reduces two bit signals
class ControlReducerModule extends Module {
  val io = new Bundle {
    val a = Bits(INPUT, width=2)
    val select = Bits(INPUT, width=2)
    val out = Bits(OUTPUT, width=1)
  }
  val a0 = io.a(0)
  val a1 = io.a(1)
  when (io.select === Bits(0, width=2)) {
    // When select is 00, A' = A0
    io.out := a0
  } .elsewhen (io.select === Bits(3, width=2)) {
    // When select is 11, A' = A1
    io.out := a1
  } .otherwise {
    // When select is 10, A' = A0 or A1
    // Technically, select being 01 is invalid
    io.out := a0 | a1
  }
}

// Muxes between control inputs
class ControlMuxModule extends Module {
  val io = new Bundle {
    val h_wire_above = Vec.fill(9){Bits(INPUT, width=2)}
    val h_wire_below = Vec.fill(9){Bits(INPUT, width=2)}
    val select = Bits(INPUT, width=6)
    val a = Bits(OUTPUT, width=2)
  }
  io.a := Bits(0)
  // Select which set of H wires to use
  val h_wires = Mux(io.select(4), io.h_wire_below, io.h_wire_above)
  when (io.select(5) === Bits(0, width=1)) {
    // Output a literal
    io.a := Cat(io.select(0), Bits(0, width=1))
  } .otherwise {
    // Output an H wire
    //switch (io.select(3,1)) {

    // JENNY
    switch (io.select(3,0)) {
      // select from 2 to 10 until 11 
      for (i <- 0 until 9) {
        is (Bits(i + 2, width=4)) {
        //is (Bits(i + 2, width=3)) {
          io.a := h_wires(i)
        }
      }
    }
  }
}

// TODO: Constants need to be discussed
class ControlBlockModule extends Module {
  val io = new Bundle { 
    val H_wire_above_in = Vec.fill(9){Bits(INPUT, width=2)}
    val H_wire_below_in = Vec.fill(9){Bits(INPUT, width=2)}
    // Configuration
    val config = Bits(INPUT, width=64)

   // Output Control

    // 00: H wires driven from right end (shift left)
    // 01: H wires driven from cente
    // 10: H wires driven from left end (shift right)
    val Hdir = Bits(OUTPUT, width=2)

    val read_demand_access = Bool(OUTPUT)
    val write_demand_access = Bool(OUTPUT)
    val prefetch_demand_access = Bool(OUTPUT)

    val load_transfer_access = Bool(OUTPUT)
    val store_transfer_access = Bool(OUTPUT)

    val mem_D_or_Z = Bits(OUTPUT, width=1)
  }

  // Control Signals
  val A_prime = Bool()
  val B_prime = Bool()
  val C_prime = Bool()
  val D_prime = Bool()

  val indexes = Vec.fill(4){Bits(width=6)}
  val primes = Vec.fill(4){Bits(width=2)}

  // Configuration encoding
  val mode = io.config(2,0)
  io.Hdir := io.config(4,3)
  //TODO: Use mode_specific fields
  val mode_specific_field = io.config(31,5)
  for (i <- 0 until 4) {
    indexes(i) := io.config(63 - (i * 8), 58 - (i * 8))
    primes(i) := io.config(57 - (i * 8), 56 - (i * 8))
    
  }

  // Declare muxes
  val muxes = Vec.fill(4){Module(new ControlMuxModule()).io}

  // Declare reducers
  val reducers = Vec.fill(4){Module(new ControlReducerModule()).io}

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
  // prime 0-3 A-D
  A_prime := (reducers(0).out).toBool
  B_prime := (reducers(0).out & reducers(1).out).toBool
  C_prime := (reducers(0).out & reducers(2).out).toBool
  D_prime := (reducers(0).out & reducers(3).out).toBool
  
  // 1 Processor Mode 010 //
  val proc_mode = Bool() 
  proc_mode := (mode === Bits(2,width=3))

  // If the A' and C' inputs are both 1, the array clock counter is zeroed at the end of the current array clock cycle, thus halting array execution.
  val zero_counter = Bool()
  zero_counter := proc_mode && A_prime && C_prime

  // If A' and D' are both 1, the main processor is forced to take an interrupt.
  val en_interrupt = Bool()
  en_interrupt := proc_mode && A_prime && D_prime 

  // 2 Memory Mode 110 // 
  val mem_mode = Bool() 
  mem_mode := (mode === Bits(6,width=3))

  // 2.1 Initial Step ////
  // A demand access to memory is initiated whenever A' and B ' are both 1.
  val demand_access = Bool()
  demand_access := mem_mode && A_prime && B_prime

  // p242
  // We use the default Rocket cache policy 
  // 01 - read/prefetch, cache allocate
  // 10 - read/write, cache allocate
  // 11 - read/write, no cache allocate 
  val demand_access_type = Bits(width=2)
  demand_access_type := io.config(31,30)

  //0 -> 7: 1 -> 8 cycles 
  val delay = Bits(width=3)
  delay := io.config(26,24)

  // 0 -> 8 bits; 1 -> 16 bits; 2 -> 32 bits
  val size_a = Bits(width=2)
  size_a := io.config(23,22)

  // aligned address? can be ignored for now?
  val N = Bits(width=1)
  N := io.config(21)

  // Demand access 1, 2, 4 words, but we only have 1 memory bus
  // Assert K to be 0 
  val K = Bits(width=2)
  K := io.config(17,16)
  assert ((K === Bits(0, width=2)), "We only have 1 memory bus!\n")

  // If D' is 0 at that time, the access will be a read; otherwise it will be either a write or a prefetch, depending on the configuration.
  //val read_demand_access = Bool(false)
  io.read_demand_access := demand_access && ~D_prime 

  //If an access is not a read (that is, if D' = 1), it is either a write or a prefetch. The type field of the configuration (Figure A.42) determines whether a non-read memory access is a write or a prefetch, and also whether a cache miss should cause data to be brought into the cache
  //val write_demand_access = Bool(false)
  io.write_demand_access := demand_access && D_prime && demand_access_type(1).toBool  

  //val prefetch_demand_access = Bool(false)
  io.prefetch_demand_access := demand_access && D_prime && ~(demand_access_type(1).toBool)

  // 2.2 Transfer Step ////
  // 0 -> 8 bits; 1 -> 16 bits; 2 -> 32 bits
  // col 4->7; col 4->11; col 4->19
  val size_r = Bits(width=2)
  size_r := io.config(15,14)

  // 0 -> Z reg; 1 -> D reg ; mem_D_or_Z for logic block
  val R = Bits(width=1)
  R := io.config(13)
  io.mem_D_or_Z := R

  // bus 0 - 3 
  // we only have one bus for now 
  val bus = Bits(width=2)
  bus := io.config(9,8)
  assert(bus === Bits(0, width=2), "We only have memory bus 0!\n")
 

  // The transfer step performs the actual movement of data, either simultaneously with the initiate step in the case of writes, or after the data has been read from memory.
  // The C' input to the control block decides, for each clock cycle, whether a transfer into or out of the row occurs on that cycle.
  val transfer_access = Bool()
  transfer_access := mem_mode && A_prime && C_prime 

  // val load_transfer_access = Bool(false)
  io.load_transfer_access := transfer_access && ~D_prime 

  // val store_transfer_access = Bool(false)
  io.store_transfer_access := transfer_access && D_prime 

}

