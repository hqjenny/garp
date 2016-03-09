package garp

import Chisel._

class CrossbarModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Bits(INPUT, width=W) 
    val sel = Bits(INPUT, width=2)
    val out = Bits(OUTPUT, width=W) 
  }
  for(i <- 0 until 2){
    when (io.sel(i) === Bits(0, width=1)){
      io.out(i) := io.in(0)
    }.otherwise {
      io.out(i) := io.in(1)
    }
  }
}

class ShiftInvertModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val shift_in = Bits(INPUT, width=1) 
    val in = Bits(INPUT, width=W) 
    val sel = Bits(INPUT, width=2)
    val out = Bits(OUTPUT, width=W) 
    val shift_out = Bits(OUTPUT, width=1) 
  }
  
  switch(io.sel){
    is(Bits(0, width=2)){
      io.out(0):= io.in(0)
      io.out(1):= io.in(1)
    }
    is(Bits(1, width=2)){
      io.out(0):= ~io.in(0)
      io.out(1):= ~io.in(1)
    }
    is(Bits(2, width=2)){
      io.out(0):= io.shift_in
      io.out(1):= io.in(0)
    }
    is(Bits(3, width=2)){
      io.out(0):= ~io.shift_in
      io.out(1):= ~io.in(0)
    }
  }
  io.shift_out := io.in(1)
}

// We assume directly shifting 1 carry bit left 
class CarrySaveAdderModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val shift_carry_in = Bits(INPUT, width=1)
    val in = Vec.fill(3){Bits(INPUT, width=W)}
    val sum = Bits(INPUT, width=W)
    val carry = Bits(INPUT, width=W)
    val shift_carry_out = Bits(OUTPUT, width=1)
  }

  val carry = Bits(width=W)
  for(i <- 0 until 2){
    io.sum(i):= io.in(2)(i)^io.in(1)(i)^io.in(0)(i)
    carry(i) := (io.in(2)(i)&io.in(1)(i)) | (io.in(1)(i)&io.in(0)(i)) | 
(io.in(2)(i)&io.in(0)(i)) 
  }

  io.shift_carry_out := carry(1)
  io.carry(1) := carry(0)
  io.carry(0) := io.shift_carry_in
}

class SelectorModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(4){Bits(INPUT, width=W)}
    val sel = Bits(INPUT, width=2)
    val out = Bits(OUTPUT, width=W)
  }
  switch(io.sel){
    for(i <- 0 until 4){
      is(Bits(i, width=2)){
        io.out:= io.in(i)
      }
    }
  }
}

class LookUpTableModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(3){Bits(INPUT, width=W)}
    val config = Bits(INPUT, width=8)
    val out = Bits(OUTPUT, width=W)
  }

  // Turn AA'BB'CC' into ABC & A'B'C'
  val in = Vec.fill(2){Bits(width=3)}
  for (i <- 0 until 2){
    for(j <- 0 until 3){
      in(i)(j) := io.in(j)(i)
    }
  }

  for (i <- 0 until 2){
    switch(in(i)){
      for(j <- 0 until 8){
        is(Bits(j, width=3)){
          io.out(i):= io.config(j)
        }
      }
    }
  }
}

class CarryChainModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val propagate = Bits(INPUT, width=W)
    val generate = Bits(INPUT, width=W)
    val carry_in = Bits(OUTPUT, width=1)
    val out = Bits(OUTPUT, width=W)
    val carry_out = Bits(OUTPUT, width=1)
  }
  val mux0_out = Bits(width=1)
  
  mux0_out := Mux(io.propagate(0), io.carry_in, io.generate(0)) 
  io.carry_out := Mux(io.propagate(1), mux0_out, io.generate(1)) 

  io.out(0) := io.carry_in
  io.out(1) := mux0_out
}

class ResultFunctionModule(val W: Int=2) extends Module {
   val io = new Bundle { 
    val U = Bits(INPUT, width=2)
    val V = Bits(INPUT, width=2)
    val K = Bits(INPUT, width=2)
    val mx = Bits(INPUT, width=2)
    val carry_out = Bits(INPUT, width=1)
    val out = Bits(OUTPUT, width=W)
  }
    
  switch(io.mx){
    is(Bits(0, width=2)){
      io.out := io.V
    }
    is(Bits(1, width=2)){
      io.out(0) := io.carry_out
      io.out(1) := io.carry_out
    }
    is(Bits(2, width=2)){
      io.out := io.U ^ io.K
    }
    is(Bits(3, width=2)){
      io.out := ~(io.U ^ io.K)
    }
  }
}

// This function may not be correct 
class TableSelectorModule(val W: Int=2) extends Module {
  val io = new Bundle { 
    val in = Vec.fill(2){Bits(INPUT, width=W)}
    val sel = Bits(INPUT, width=2)
    val out = Bits(OUTPUT, width=2)
  }
  for (i <- 0 until 2){
    // Choose from different tables
    io.out(i) := Mux(io.sel(i), io.in(1)(i), io.in(0)(i))
  }

}

//TODO shift bits may be zero based on configurations
// mode 0 bit 

class FunctionalUnitModule(val W: Int=2) extends Module {

  val io = new Bundle { 
    // 0:A, 1:B, 2:C, 3:D
    val X_in = Vec.fill(4){Bits(INPUT, width=W)}

    // Shift_A_in, Shift_B_in, Shift_C_in
    val shift_X_in = Vec.fill(3){Bits(INPUT, width=1)}

    // Hout Above
    val H_out_above = Bits(INPUT, width=W)

    val shift_carry_in = Bits(INPUT, width=1)
    val carry_in = Bits(INPUT, width=1)

    val config = Bits(INPUT, width=64)
   
    // Shift_A_out, Shift_B_out, Shift_C_out
    val shift_X_out = Vec.fill(3){Bits(OUTPUT, width=1)}
    val shift_carry_out = Bits(OUTPUT, width=1)
    val carry_out = Bits(OUTPUT, width=1)
    val Z = Bits(OUTPUT, width=W)
  }

  // Data Signals
  val crossbar_out = Vec.fill(4){Bits(width=W)}
  val shiftinvert_out = Vec.fill(3){Bits(width=W)}
  val csa_out_carry = Bits(width=W) 
  val csa_out_sum = Bits(width=W) 
  val lut_in = Vec.fill(3){Bits(width=W)}
  val lut_out = Vec.fill(2){Bits(width=W)}
  val carrychain_out = Bits(width=W) 
  val resultfunct_out = Bits(width=W) 
  val tableselector_out = Bits(width=W) 
  val D_sel_out = Bits(width=W) 
  val selector_out = Bits(width=W) 

  // Control Signals
  val X_in_sel = Vec.fill(4){Bits(width=2)}
  val mode = Bits(width=3)
  val mx = Bits(width=2)
  val csa_or_crossbar = Bool(false) 
  val D_sel = Bool(false) 

  mode := io.config(15,13)
  X_in_sel(0) := io.config(57,56)
  X_in_sel(1) := io.config(49,48)
  X_in_sel(2) := io.config(41,40)
  // mx field 
  X_in_sel(3) := io.config(33,32)
  mx := io.config(33,32)


  // Only triple add mode(11k) needs output from adder  
  csa_or_crossbar := (mode(2,1) === Bits(3, width=2))

  // carry chain 
  val crossbar = Vec.fill(4){Module(new CrossbarModule(W)).io}
  val shiftinvert = Vec.fill(3){Module(new ShiftInvertModule(W)).io}
  val csa = Module(new CarrySaveAdderModule(W)).io
  val lut = Vec.fill(2){Module(new LookUpTableModule(W)).io}
  val carrychain = Module(new CarryChainModule(W)).io
  val resultfunct = Module(new ResultFunctionModule(W)).io
  val tableselector = Module(new TableSelectorModule(W)).io
  val selector = Module(new SelectorModule(W)).io

  // Crossbar
  for (i <- 0 until 4){
      crossbar(i).in := io.X_in(i)
      crossbar(i).sel := X_in_sel(i)
      crossbar_out(i) := crossbar(i).out 
  }

  // ShiftInvert
  for (i <- 0 until 3){
      shiftinvert(i).shift_in := io.shift_X_in(i)
      shiftinvert(i).in := io.X_in(i)
      shiftinvert(i).sel := X_in_sel(i)
      shiftinvert_out(i) := shiftinvert(i).out 
      io.shift_X_out(i) := shiftinvert(i).shift_out
  }
  
  // CarrySaveAdder
  csa.shift_carry_in := io.shift_carry_in
  csa.in := shiftinvert_out
  csa_out_sum := csa.sum
  csa_out_carry := csa.carry
  io.shift_carry_out := csa.shift_carry_out


  // csa_or_crossbar muxes
  lut_in(0) := Mux(csa_or_crossbar, csa_out_carry, crossbar_out(0))  
  lut_in(1) := Mux(csa_or_crossbar, csa_out_sum, crossbar_out(1))  
  lut_in(2) := crossbar_out(2)


  // LookUpTable
  lut(1).config := io.config(31, 24) // Propagate p216
  lut(0).config := io.config(23, 16) // Generate
  for(i <- 0 until 2){
    lut(i).in := lut_in
    lut_out(i) := lut(i).out
  }

  // CarryChain
  carrychain.propagate := lut_out(1)
  carrychain.generate := lut_out(0)
  carrychain.carry_in := io.carry_in
  carrychain_out := carrychain.out
  io.carry_out := carrychain.carry_out

  // ResultFunction
  resultfunct.U := lut_out(1)
  resultfunct.V := lut_out(0)
  resultfunct.K := carrychain_out
  resultfunct.mx := mx
  resultfunct_out := resultfunct.out

  // D_sel  
  // Table Mode 000 -> D
  // Split Table Mode 001 -> 2'b10
  D_sel := mode(0)
  D_sel_out := Mux ( D_sel, Bits(2, width=2), crossbar_out(3))
  

  // TableSelector
  tableselector.in := lut_out
  tableselector.sel := D_sel_out
  tableselector_out := tableselector.out

}



class CarrySaveAdderModuleTests(c: CarrySaveAdderModule) extends Tester(c) {
      poke(c.io.shift_carry_in, 0)
      //poke(c.io., 0)
      step(1)
}

object CarrySaveAdderMain { 
  def main(args: Array[String]): Unit = {
    //chiselMainTest(Array[String]("--backend", "c", "--genHarness", "--compile", "--test"),
    chiselMainTest(args,
    () => Module(new CarrySaveAdderModule(2))){c => new CarrySaveAdderModuleTests(c)
    }
  }
}

