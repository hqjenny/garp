package garp

import Chisel._

class CrossbarModule(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Bits(INPUT, width=W) 
    val sel = Bits(INPUT, width=2)
    val out = Bits(OUTPUT, width=W) 
  }
  io.out := Bits(0)
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
  io.out := Bits(0)
  io.shift_out := Bits(0)
  
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
    val sum = Bits(OUTPUT, width=W)
    val carry = Bits(OUTPUT, width=W)
    val shift_carry_out = Bits(OUTPUT, width=1)
  }

  io.sum := Bits(0)
  io.carry := Bits(0)
  val carry = Bits(width=W)
  carry := Bits(0)

  for(i <- 0 until 2){
    printf("i %d: %d %d %d\n", Bits(i,2), io.in(0), io.in(1), io.in(2))
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
  io.out := Bits(2)
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

  io.out := Bits(0)
  // Turn AA'BB'CC' into ABC & A'B'C'
  val in = Vec.fill(2){Bits(width=3)}
  in := Bits(0)
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
    val carry_in = Bits(INPUT, width=1)
    val out = Bits(OUTPUT, width=W)
    val carry_out = Bits(OUTPUT, width=1)
  }
  io.out := Bits(0)
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
    
  io.out := Bits(0)
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
  io.out := Bits(0)
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
  // Set Default Value
  io.Z := Bits(0)

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
  val csa_or_crossbar = Bool() 
  val D_sel = Bool() 

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
  // mode[0] -> 1 shift_in 
  // mode[0] -> 0 0 
  for (i <- 0 until 3){
      shiftinvert(i).shift_in := Mux(mode(0).toBool, io.shift_X_in(i), Bits(0, width=1))
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
  // mode[0] -> 1 shift_in 
  // mode[0] -> 0 0 
  carrychain.propagate := lut_out(1)
  carrychain.generate := lut_out(0)
  carrychain.carry_in := Mux(mode(0).toBool, io.carry_in, Bits(0, width=1))
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

  // Selector
  // mx[0] -> 0 in2 -> D in3 -> H_out_above
  // mx[1] -> 1 in2 -> B in3 -> 0
  selector.in(0) := shiftinvert_out(0)
  selector.in(1) := shiftinvert_out(1)
  selector.in(2) := Mux (mx(0).toBool, io.X_in(1), io.X_in(3))
  selector.in(3) := Mux (mx(0).toBool, Bits(0, width=2), io.H_out_above)
  selector.sel := shiftinvert_out(2)
  selector_out := selector.out

  // Output 
  // mode -> 000 mx -> D' : table mode 
  // mode -> 001 mx -> 01 : split table mode 
  // mode -> 01k mx -> 00 : select mode 
  // mode -> 01k mx -> 01 : partial select mode 
  // mode -> 10k mx -> result : carry chain mode 
  // mode -> 11k mx -> result : triple add mode
  switch(mode(2,1)){
    is(Bits(0, width=2)){
     io.Z := tableselector_out 
    }
    is(Bits(1, width=2)){
      io.Z := selector_out 
    }
    is(Bits(2, width=2)){
      io.Z := resultfunct_out
    }
    is(Bits(3, width=2)){
      io.Z := resultfunct_out 
    }
  }
}

class CarrySaveAdderModuleTests(c: CarrySaveAdderModule) extends Tester(c) {
      
      // Exhausive Tests
      for (i <- 0 until 2){

        poke(c.io.shift_carry_in, i)
        for (j <- 0 until 4){

          poke(c.io.in(0), j)
          for (k <- 0 until 4){

            poke(c.io.in(1), k)
            for (l <- 0 until 4){

                poke(c.io.in(2), l)
                val sum0 = (j & 0x1) ^ (k & 0x1) ^ (l & 0x1) 
                val sum1 = (j & 0x2) ^ (k & 0x2) ^ (l & 0x2) 

                val carry1 = ((j & 0x1) + (k & 0x1) + (l & 0x1)) >> 1 
                val carry2 = ((j & 0x2) + (k & 0x2) + (l & 0x2)) >> 2 

                step(1)
                expect(c.io.sum(0), sum0)
                expect(c.io.sum(1), sum1)
                expect(c.io.carry(0), i)
                expect(c.io.carry(1), carry1)
                expect(c.io.carry(2), carry2)

            }
          }
        }
      }
      /*
      val in = Array( 
        BigInt(1),
        BigInt(2),
        BigInt(3)
      )
      poke(c.io.shift_carry_in, 0)
      //poke(c.io.in, in)

      peek(c.io.in(0))
      peek(c.io.in(1))
      peek(c.io.in(2))
      poke(c.io.in(0), 1)
      poke(c.io.in(1), 2)
      poke(c.io.in(2), 3)
      step(1)
      peek(c.io.sum)
      peek(c.io.carry)
      peek(c.io.shift_carry_out)*/
}

object CarrySaveAdderMain { 
  def main(args: Array[String]): Unit = {
    //chiselMainTest(Array[String]("--backend", "c", "--genHarness", "--compile", "--test"),
    chiselMainTest(args,
    () => Module(new CarrySaveAdderModule(2))){c => new CarrySaveAdderModuleTests(c)
    }
  }
}
