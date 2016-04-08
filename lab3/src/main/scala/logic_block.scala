package garp

import Chisel._

// W bit width, H is at most 12 
class LogicBlockModule(val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4) extends Module {

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
    val Z_in = Bits(INPUT, width=W)
    val D_in = Bits(INPUT, width=W)
    val Z_out = Bits(OUTPUT, width=W)
    val D_out = Bits(OUTPUT, width=W)
    val test = Bool(INPUT)

    val mem_bus_out = Bits(OUTPUT, width=W)
    val V_wire_out = Bits(OUTPUT, width=W)
    val H_wire_out = Bits(OUTPUT, width=W)
    val G_wire_out = Bits(OUTPUT, width=W)

    val config_G_out = Bits(OUTPUT, width=3)
    val config_V_out = Bits(OUTPUT, width=5)
  }
  // Data Signals
  val Z = Bits(width=W)
  val Z_reg_in = Bits(width=W)
  val D_reg_in = Bits(width=W)
  val Z_reg  = Reg(init = Bits(0,W))
  val D_reg  = Reg(init = Bits(0,W))
  val Z_out = Bits(width=W)
  val D_out = Bits(width=W)

  // Control Signals
  val Z_sel = Bool()
  Z_sel := io.config(12).toBool

  val D_sel = Bool()
  D_sel := io.config(11).toBool

  val H_sel = Bool()
  H_sel := io.config(10).toBool

  val G_sel = Bool()
  G_sel := io.config(9).toBool

  val V_sel = Bool()
  V_sel := io.config(8).toBool

  io.config_G_out := io.config(7,5)
  io.config_V_out := io.config(4,0)

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
  val X_in = Vec.fill(4){Bits(width=W)}

  val IN_MUX = Module(new LogicBlockInModule(W, V, H, G)).io
  IN_MUX.V_wire_in := io.V_wire_in
  IN_MUX.H_wire_above_in := io.H_wire_above_in
  IN_MUX.H_wire_below_in := io.H_wire_below_in
  IN_MUX.G_wire_above_in := io.G_wire_above_in
  IN_MUX.G_wire_below_in := io.G_wire_below_in
  IN_MUX.Z_reg := Z_reg
  IN_MUX.D_reg := D_reg

  IN_MUX.config_X_in := config_X_in

  X_in := IN_MUX.X_in

  // Upper Muxes
  /*for(i <- 0 until 4) {
    X_in(i) := Bits(0)
    
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
  }*/
 
  // Functional Unit
  val FUpeek = Module(new FunctionalUnitModule(W))
  val FU = FUpeek.io
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

    // Z_sel might not be the best indicator 
    // Alternatively, a register can be bypassed on output, in which case it never latches except when it is written to via a memory bus.
  when(Z_sel){
    Z_reg := Z_reg_in
  }.elsewhen(io.test){
    Z_reg := io.Z_in
  }
  when(D_sel){
    D_reg := D_reg_in
  }.elsewhen(io.test){
    D_reg := io.D_in
  }

  io.Z_out := Z_reg
  io.D_out := D_reg

  io.mem_bus_out := Mux(io.mem_D_or_Z.toBool, D_reg, Z_reg)
  
  Z_out := Mux(Z_sel, Z_reg, Z)
  D_out := Mux(D_sel, D_reg, X_in(3))

  io.H_wire_out := Mux(H_sel, D_out, Z_out)
  io.G_wire_out := Mux(G_sel, D_out, Z_out)
  io.V_wire_out := Mux(V_sel, D_out, Z_out)
}

object replace_range {
  def apply(field:BigInt, bits:Int, size:Int, pos:Int) : BigInt = {
    val clear = BigInt(((1 << size) - 1)) << pos
    val mask = BigInt((bits & ((1 << size) - 1))) << pos
    return (field &~ clear) | mask
  }
}

object shift_invert {
  def apply(input:Int, shift_in:Int, prime:Int) : Int = {
    if (prime == 0) {
      return input
    } else if (prime == 1) {
      // Inverts the lower two bits
      return input ^ 3
    } else if (prime == 2) {
      return ((input & 1) << 1) | shift_in
    } else {
      return (((input & 1) << 1) | shift_in) ^ 3
    }
  }
}

object crossbar {
  def apply(input:Int, prime:Int) : Int = {
    if (prime == 0) {
      return ((input & 1) << 1) | (input & 1)
    } else if (prime == 1) {
      return ((input & 1) << 1) | (input >> 1)
    } else if (prime == 2) {
      return input
    } else {
      return (input & 2) | (input >> 1)
    }
  }
}

class LogicBlockModuleTests(c: LogicBlockModule) extends Tester(c) {
  val rand = scala.util.Random
  val TRIALS = 10

  println("\n=====Basic Propagation Test=====")
  // Pass an incoming V wire signal from D to an H wire
  var encoding = BigInt(0x0000000000000000L)
  // Set D to be V wire pair 0
  encoding = replace_range(encoding, 0x1F, 6, 34)
  // Set Hout to D
  encoding = replace_range(encoding, 0x1, 1, 10)
  println("Encoding: 0x" + encoding.toString(16))
  peek(c.io.H_wire_out)
  poke(c.io.config, encoding)
  poke(c.io.V_wire_in(0), 2)
  expect(c.io.H_wire_out, 2)


  println("\n=====Table Mode Tests=====")
  encoding = BigInt(0x0000000000000000L)
  poke(c.io.config, encoding)
  // Set to table mode
  encoding = replace_range(encoding, 0x0, 3, 13)
  // Set A to be H wire pair 1 above and set A'
  encoding = replace_range(encoding, 0xA6, 8, 56)
  // Set B to be H wire pair 2 below and set B'
  encoding = replace_range(encoding, 0xE2, 8, 48)
  // Set C to be G wire pair 3 above and set C'
  encoding = replace_range(encoding, 0xB2, 8, 40)
  // Set D to be V wire pair 0 and set D'
  encoding = replace_range(encoding, 0x7E, 8, 32)

  for (trial <- 1 to TRIALS) {
    println("--Trial " + trial.toString + "--")
    var inputs = new Array[Int](4)
    for (i <- 0 until 4) {
      inputs(i) = rand.nextInt(4)
    }
    println("Random Inputs: " + inputs.mkString(", "))
    var table = rand.nextInt(0x10000)
    var big_table = BigInt(table)
    println("Random Table: 0x" + big_table.toString(16))
    encoding = replace_range(encoding, table, 16, 16)
    println("Encoding: 0x" + encoding.toString(16))
    poke(c.io.config, encoding)
    poke(c.io.H_wire_above_in(1), inputs(0))
    poke(c.io.H_wire_below_in(2), inputs(1))
    poke(c.io.G_wire_above_in(3), inputs(2))
    poke(c.io.V_wire_in(0), inputs(3))
    var upper_addr = 1 * (inputs(0) >> 1) + 2 * (inputs(1) >> 1) + 4 * (inputs(2) >> 1) + 8 * (inputs(3) >> 1)
    var lower_addr = 1 * (inputs(0) & 1) + 2 * (inputs(1) & 1) + 4 * (inputs(2) & 1) + 8 * (inputs(3) & 1)
    var upper_result = if(big_table.testBit(upper_addr)) 1 else 0
    var lower_result = if(big_table.testBit(lower_addr)) 1 else 0
    expect(c.io.H_wire_out, (upper_result << 1) | lower_result)
    println("")
  }


  println("\n=====Split Table Mode Tests=====")
  encoding = BigInt(0x0000000000000000L)
  poke(c.io.config, encoding)
  // Set to split table mode
  encoding = replace_range(encoding, 0x1, 3, 13)
  // Set A to be H wire pair 1 above and set A'
  encoding = replace_range(encoding, 0xA6, 8, 56)
  // Set B to be H wire pair 2 below and set B'
  encoding = replace_range(encoding, 0xE2, 8, 48)
  // Set C to be G wire pair 3 above and set C'
  encoding = replace_range(encoding, 0xB2, 8, 40)
  // Set D to be V wire pair 0 and set mx to 01
  encoding = replace_range(encoding, 0x7D, 8, 32)

  for (trial <- 1 to TRIALS) {
    println("--Trial " + trial.toString + "--")
    var inputs = new Array[Int](4)
    for (i <- 0 until 4) {
      inputs(i) = rand.nextInt(4)
    }
    println("Random Inputs: " + inputs.mkString(", "))
    var table = rand.nextInt(0x10000)
    var big_table = BigInt(table)
    println("Random Table: 0x" + big_table.toString(16))
    encoding = replace_range(encoding, table, 16, 16)
    println("Encoding: 0x" + encoding.toString(16))
    poke(c.io.config, encoding)
    poke(c.io.H_wire_above_in(1), inputs(0))
    poke(c.io.H_wire_below_in(2), inputs(1))
    poke(c.io.G_wire_above_in(3), inputs(2))
    poke(c.io.V_wire_in(0), inputs(3))
    // Expect D' be 10 as seen by functional unit.
    var upper_addr = 1 * (inputs(0) >> 1) + 2 * (inputs(1) >> 1) + 4 * (inputs(2) >> 1) + 8 * (1)
    var lower_addr = 1 * (inputs(0) & 1) + 2 * (inputs(1) & 1) + 4 * (inputs(2) & 1) + 8 * (0)
    var upper_result = if(big_table.testBit(upper_addr)) 1 else 0
    var lower_result = if(big_table.testBit(lower_addr)) 1 else 0
    expect(c.io.H_wire_out, (upper_result << 1) | lower_result)
    println("")
  }


  println("\n=====Select Mode Tests=====")
  encoding = BigInt(0x0000000000000000L)
  poke(c.io.config, encoding)
  // Set to select mode
  encoding = replace_range(encoding, 0x2, 3, 13)
  // Set A to be H wire pair 1 above and set A'
  encoding = replace_range(encoding, 0x29, 6, 58)
  // Set B to be H wire pair 2 below and set B'
  encoding = replace_range(encoding, 0x38, 6, 50)
  // Set C to be G wire pair 3 above and set C'
  encoding = replace_range(encoding, 0x2C, 6, 42)
  // Set D to be V wire pair 0 and set mx to 00
  encoding = replace_range(encoding, 0x7C, 8, 32)
  // Set table to predefined constant
  encoding = replace_range(encoding, 0xCCCC, 16, 16)

  for (trial <- 1 to TRIALS) {
    println("--Trial " + trial.toString + "--")
    var inputs = new Array[Int](4)
    for (i <- 0 until 4) {
      inputs(i) = rand.nextInt(4)
    }
    println("Random Inputs: " + inputs.mkString(", "))
    var primes = new Array[Int](3)
    for (i <- 0 until 3) {
      primes(i) = rand.nextInt(4)
    }
    println("Random Primes: " + primes.mkString(", "))
    var shift_ins = new Array[Int](3)
    for (i <- 0 until 3) {
      shift_ins(i) = rand.nextInt(2)
    }
    println("Random Shift Ins: " + shift_ins.mkString(", "))
    var hout_above = rand.nextInt(4)
    println("Hout Above: " + hout_above.toString)
    var k = rand.nextInt(2)
    println("k: " + k.toString)
    encoding = replace_range(encoding, k, 1, 13)
    encoding = replace_range(encoding, primes(0), 2, 56)
    encoding = replace_range(encoding, primes(1), 2, 48)
    encoding = replace_range(encoding, primes(2), 2, 40)
    println("Encoding: 0x" + encoding.toString(16))
    poke(c.io.config, encoding)
    poke(c.io.H_wire_above_in(1), inputs(0))
    poke(c.io.H_wire_below_in(2), inputs(1))
    poke(c.io.G_wire_above_in(3), inputs(2))
    poke(c.io.V_wire_in(0), inputs(3))
    poke(c.io.shift_X_in(0), shift_ins(0))
    poke(c.io.shift_X_in(1), shift_ins(1))
    poke(c.io.shift_X_in(2), shift_ins(2))
    poke(c.io.H_out_above, hout_above)

    var shift_outs = new Array[Int](3)
    var intermediates = new Array[Int](3)
    for (i <- 0 until 3) {
      shift_outs(i) = inputs(i) >> 1
      intermediates(i) = shift_invert(inputs(i), shift_ins(i) & k, primes(i))
    }
    var result = 0
    if (intermediates(2) == 0) {
      result = intermediates(0)
    } else if (intermediates(2) == 1) {
      result = intermediates(1)
    } else if (intermediates(2) == 2) {
      result = inputs(3)
    } else {
      result = hout_above
    }
    expect(c.io.H_wire_out, result)
    expect(c.io.shift_X_out(0), shift_outs(0))
    expect(c.io.shift_X_out(1), shift_outs(1))
    expect(c.io.shift_X_out(2), shift_outs(2))
    println("")
  }

  println("\n=====Partial Select Mode Tests=====")
  encoding = BigInt(0x0000000000000000L)
  poke(c.io.config, encoding)
  // Set to partial select mode
  encoding = replace_range(encoding, 0x2, 3, 13)
  // Set A to be H wire pair 1 above and set A'
  encoding = replace_range(encoding, 0x29, 6, 58)
  // Set B to be H wire pair 2 below and set B'
  encoding = replace_range(encoding, 0x38, 6, 50)
  // Set C to be G wire pair 3 above and set C'
  encoding = replace_range(encoding, 0x2C, 6, 42)
  // Set D to be V wire pair 0 and set mx to 01
  encoding = replace_range(encoding, 0x7D, 8, 32)
  // Set table to predefined constant
  encoding = replace_range(encoding, 0xCCCC, 16, 16)

  for (trial <- 1 to TRIALS) {
    println("--Trial " + trial.toString + "--")
    var inputs = new Array[Int](4)
    for (i <- 0 until 4) {
      inputs(i) = rand.nextInt(4)
    }
    println("Random Inputs: " + inputs.mkString(", "))
    var primes = new Array[Int](3)
    for (i <- 0 until 3) {
      primes(i) = rand.nextInt(4)
    }
    println("Random Primes: " + primes.mkString(", "))
    var shift_ins = new Array[Int](3)
    for (i <- 0 until 3) {
      shift_ins(i) = rand.nextInt(2)
    }
    println("Random Shift Ins: " + shift_ins.mkString(", "))
    var k = rand.nextInt(2)
    println("k: " + k.toString)
    encoding = replace_range(encoding, k, 1, 13)
    encoding = replace_range(encoding, primes(0), 2, 56)
    encoding = replace_range(encoding, primes(1), 2, 48)
    encoding = replace_range(encoding, primes(2), 2, 40)
    println("Encoding: 0x" + encoding.toString(16))
    poke(c.io.config, encoding)
    poke(c.io.H_wire_above_in(1), inputs(0))
    poke(c.io.H_wire_below_in(2), inputs(1))
    poke(c.io.G_wire_above_in(3), inputs(2))
    poke(c.io.V_wire_in(0), inputs(3))
    poke(c.io.shift_X_in(0), shift_ins(0))
    poke(c.io.shift_X_in(1), shift_ins(1))
    poke(c.io.shift_X_in(2), shift_ins(2))

    var shift_outs = new Array[Int](3)
    var intermediates = new Array[Int](3)
    for (i <- 0 until 3) {
      shift_outs(i) = inputs(i) >> 1
      intermediates(i) = shift_invert(inputs(i), shift_ins(i) & k, primes(i))
    }
    var result = 0
    if (intermediates(2) == 0) {
      result = intermediates(0)
    } else if (intermediates(2) == 1) {
      result = intermediates(1)
    } else if (intermediates(2) == 2) {
      result = inputs(1)
    } else {
      result = 0
    }
    expect(c.io.H_wire_out, result)
    expect(c.io.shift_X_out(0), shift_outs(0))
    expect(c.io.shift_X_out(1), shift_outs(1))
    expect(c.io.shift_X_out(2), shift_outs(2))
    println("")
  }

  println("\n=====Carry Chain Mode Tests=====")
  encoding = BigInt(0x0000000000000000L)
  poke(c.io.config, encoding)
  // Set to carry chain mode
  encoding = replace_range(encoding, 0x4, 3, 13)
  // Set A to be H wire pair 1 above
  encoding = replace_range(encoding, 0x29, 6, 58)
  // Set B to be H wire pair 2 below
  encoding = replace_range(encoding, 0x38, 6, 50)
  // Set C to be G wire pair 3 above
  encoding = replace_range(encoding, 0x2C, 6, 42)
  // Set D to be V wire pair 0
  encoding = replace_range(encoding, 0x1F, 6, 34)

  for (trial <- 1 to TRIALS) {
    println("--Trial " + trial.toString + "--")
    var inputs = new Array[Int](4)
    for (i <- 0 until 4) {
      inputs(i) = rand.nextInt(4)
    }
    println("Random Inputs: " + inputs.mkString(", "))
    var primes = new Array[Int](4)
    for (i <- 0 until 4) {
      primes(i) = rand.nextInt(4)
    }
    println("Random Primes and mx: " + primes.mkString(", "))
    var carry_in = rand.nextInt(2)
    println("Random Carry In: " + carry_in.toString)
    var k = rand.nextInt(2)
    println("k: " + k.toString)
    encoding = replace_range(encoding, k, 1, 13)
    encoding = replace_range(encoding, primes(0), 2, 56)
    encoding = replace_range(encoding, primes(1), 2, 48)
    encoding = replace_range(encoding, primes(2), 2, 40)
    encoding = replace_range(encoding, primes(3), 2, 32)
    var table = rand.nextInt(0x10000)
    var big_table = BigInt(table)
    println("Random Table: 0x" + big_table.toString(16))
    encoding = replace_range(encoding, table, 16, 16)
    println("Encoding: 0x" + encoding.toString(16))
    poke(c.io.config, encoding)
    poke(c.io.H_wire_above_in(1), inputs(0))
    poke(c.io.H_wire_below_in(2), inputs(1))
    poke(c.io.G_wire_above_in(3), inputs(2))
    poke(c.io.V_wire_in(0), inputs(3))

    poke(c.io.carry_in, carry_in)

    var intermediates = new Array[Int](3)
    for (i <- 0 until 3) {
      intermediates(i) = crossbar(inputs(i), primes(i))
    }
    var upper_addr = 1 * (intermediates(0) >> 1) + 2 * (intermediates(1) >> 1) + 4 * (intermediates(2) >> 1)
    var lower_addr = 1 * (intermediates(0) & 1) + 2 * (intermediates(1) & 1) + 4 * (intermediates(2) & 1)
    var U_upper_result = if(big_table.testBit(upper_addr + 8)) 1 else 0
    var V_upper_result = if(big_table.testBit(upper_addr)) 1 else 0
    var U_lower_result = if(big_table.testBit(lower_addr + 8)) 1 else 0
    var V_lower_result = if(big_table.testBit(lower_addr)) 1 else 0
    var U = (U_upper_result << 1) | U_lower_result
    var V = (V_upper_result << 1) | V_lower_result
    var K0 = (carry_in & k)
    var K1 = if(U_lower_result == 1) K0 else V_lower_result
    var K = (K1 << 1) | K0
    var carry_out = if(U_upper_result == 1) K1 else V_upper_result
    var result = 0
    if (primes(3) == 0) {
      result = V
    } else if (primes(3) == 1) {
      result = carry_out * 3
    } else if (primes(3) == 2) {
      result = U ^ K
    } else {
      result = (U ^ K) ^ 3
    }
    expect(c.io.H_wire_out, result)
    expect(c.io.carry_out, carry_out)
    println("")
  }

  println("\n=====Triple Add Mode Tests=====")
  encoding = BigInt(0x0000000000000000L)
  poke(c.io.config, encoding)
  // Set to triple add mode
  encoding = replace_range(encoding, 0x6, 3, 13)
  // Set A to be H wire pair 1 above
  encoding = replace_range(encoding, 0x29, 6, 58)
  // Set B to be H wire pair 2 below
  encoding = replace_range(encoding, 0x38, 6, 50)
  // Set C to be G wire pair 3 above
  encoding = replace_range(encoding, 0x2C, 6, 42)
  // Set D to be V wire pair 0
  encoding = replace_range(encoding, 0x1F, 6, 34)

  for (trial <- 1 to TRIALS) {
    println("--Trial " + trial.toString + "--")
    var inputs = new Array[Int](4)
    for (i <- 0 until 4) {
      inputs(i) = rand.nextInt(4)
    }
    println("Random Inputs: " + inputs.mkString(", "))
    var primes = new Array[Int](4)
    for (i <- 0 until 4) {
      primes(i) = rand.nextInt(4)
    }
    println("Random Primes and mx: " + primes.mkString(", "))
    var shift_ins = new Array[Int](3)
    for (i <- 0 until 3) {
      shift_ins(i) = rand.nextInt(2)
    }
    println("Random Shift Ins: " + shift_ins.mkString(", "))
    var shift_carry_in = rand.nextInt(2)
    println("Random Shift Carry In: " + shift_carry_in.toString)
    var carry_in = rand.nextInt(2)
    println("Random Carry In: " + carry_in.toString)
    var k = rand.nextInt(2)
    println("k: " + k.toString)
    encoding = replace_range(encoding, k, 1, 13)
    encoding = replace_range(encoding, primes(0), 2, 56)
    encoding = replace_range(encoding, primes(1), 2, 48)
    encoding = replace_range(encoding, primes(2), 2, 40)
    encoding = replace_range(encoding, primes(3), 2, 32)
    var U_table = rand.nextInt(0x10)
    var V_table = rand.nextInt(0x10)
    var table = (U_table << 12) | (U_table << 8) | (V_table << 4) | V_table
    var big_table = BigInt(table)
    println("Random Table: 0x" + big_table.toString(16))
    encoding = replace_range(encoding, table, 16, 16)
    println("Encoding: 0x" + encoding.toString(16))
    poke(c.io.config, encoding)
    poke(c.io.H_wire_above_in(1), inputs(0))
    poke(c.io.H_wire_below_in(2), inputs(1))
    poke(c.io.G_wire_above_in(3), inputs(2))
    poke(c.io.V_wire_in(0), inputs(3))

    poke(c.io.shift_X_in(0), shift_ins(0))
    poke(c.io.shift_X_in(1), shift_ins(1))
    poke(c.io.shift_X_in(2), shift_ins(2))
    poke(c.io.shift_carry_in, shift_carry_in)
    poke(c.io.carry_in, carry_in)

    // Optional shift, invert
    var shift_outs = new Array[Int](3)
    var intermediates = new Array[Int](3)
    for (i <- 0 until 3) {
      shift_outs(i) = inputs(i) >> 1
      intermediates(i) = shift_invert(inputs(i), shift_ins(i) & k, primes(i))
    }
    // Carry-save addition
    var sum = intermediates(0) ^ intermediates(1) ^ intermediates(2)
    var carry = (intermediates(0) & intermediates (1)) | (intermediates(0) & intermediates (2)) | (intermediates(1) & intermediates (2))
    val shift_carry_out = carry >> 1
    carry = ((carry & 1) << 1) | (shift_carry_in & k)

    //Dual 2-input table lookup
    var upper_addr = 1 * (carry >> 1) + 2 * (sum >> 1)
    var lower_addr = 1 * (carry & 1) + 2 * (sum & 1)
    var U_upper_result = if(big_table.testBit(upper_addr + 8)) 1 else 0
    var V_upper_result = if(big_table.testBit(upper_addr)) 1 else 0
    var U_lower_result = if(big_table.testBit(lower_addr + 8)) 1 else 0
    var V_lower_result = if(big_table.testBit(lower_addr)) 1 else 0
    var U = (U_upper_result << 1) | U_lower_result
    var V = (V_upper_result << 1) | V_lower_result

    // Carry chain
    var K0 = (carry_in & k)
    var K1 = if(U_lower_result == 1) K0 else V_lower_result
    var K = (K1 << 1) | K0
    var carry_out = if(U_upper_result == 1) K1 else V_upper_result

/*   
    peek(c.FUpeek.resultfunct.U)
    println("U: " + U.toString)
    peek(c.FUpeek.resultfunct.V)
    println("V: " + V.toString)
    peek(c.FUpeek.resultfunct.K)
    println("K: " + K.toString)
*/


    //Result function
    var result = 0
    if (primes(3) == 0) {
      result = V
    } else if (primes(3) == 1) {
      result = carry_out * 3
    } else if (primes(3) == 2) {
      result = U ^ K
    } else {
      result = (U ^ K) ^ 3
    }
    expect(c.io.H_wire_out, result)
    expect(c.io.shift_X_out(0), shift_outs(0))
    expect(c.io.shift_X_out(1), shift_outs(1))
    expect(c.io.shift_X_out(2), shift_outs(2))
    expect(c.io.shift_carry_out, shift_carry_out)
    expect(c.io.carry_out, carry_out)
    println("")
  }
}

object LogicBlockModuleMain {
  def main(args: Array[String]): Unit = {
    // Default values
    chiselMainTest(args, () => Module(new LogicBlockModule())){
      c => new LogicBlockModuleTests(c)
    }
  }
}
