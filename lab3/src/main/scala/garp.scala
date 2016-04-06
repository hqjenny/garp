package garp

import Chisel._



class GarpAccel(val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4, val R: Int=3) extends Module {
  

  val io = new Bundle {
    val in = Vec.fill(4){Bits(INPUT, width=W)} 
    val sel =Vec.fill(4){Bits(INPUT, width=2)}
    val out =Vec.fill(4){Bits(OUTPUT, width=W)}
    
    val config = Vec.fill(24*R){Bits(INPUT, width=64)}

  }

  //val test = Vec.fill(4){Module(new TestBlockModule(W)).io}

  //for (i <- 0 until 4){
  //    test(i).in := io.in(i)
  //    test(i).sel := io.sel(i)
  //    io.out(i) := test(i).out 
  //}

  // Haven't figure out how to connect them 
  val V_wire_in = Vec.fill(23*V){Bits(INPUT, width=W)}
  val G_wire_above =  Vec.fill(G){Bits(INPUT, width=W)}

  val row0 = Module(new ArrayRowModule(W, I=0)).io
  val row1 = Module(new ArrayRowModule(W, I=1)).io
  val row2 = Module(new ArrayRowModule(W, I=2)).io

  row1.G_wire_above := row0.G_wire_below
  row2.G_wire_above := row1.G_wire_below

  row1.H_wire_above := row0.H_wire_below
  row2.H_wire_above := row1.H_wire_below

  row1.mem_bus_in := row0.mem_bus_out
  row2.mem_bus_in := row1.mem_bus_out

  row1.H_out_above := row0.H_out
  row2.H_out_above := row1.H_out
  
  for (i <- 0 until 24){
    row0.config(i) := io.config(0+i)    
    row1.config(i) := io.config(24+i)   
    row2.config(i) := io.config(48+i)   

  }

  //row0.config := io.config(23,0)    
  /*val io = new Bundle { 
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


  val LB = Module(new LogicBlockModule)
  io <> LB.io*/ 
}

/*
class DefaultConfig() extends Config {
    override val topDefinitions:World.TopDefs = {
          (pname,site,here) => pname match {
                  case WidthP => 64
                      case Stages => Knob("stages")
          }
      }
                                override val topConstraints:List[ViewSym=>Ex[Boolean]] = List(
                                      ex => ex(WidthP) === 64,
                                          ex => ex(Stages) >= 1 && ex(Stages) <= 4 && (ex(Stages)%2 === 0 || ex(Stages) === 1)
                                            )
                                            override val knobValues:Any=>Any = {
                                                  case "stages" => 1
                                                    }
                                                  }*/


class GarpAccelTests(c: GarpAccel) extends Tester(c) {

  //poke(c.io.in(0), 1)
  //poke(c.io.H_out_above, 1)
}


object GarpAccelMain {
  def main(args: Array[String]): Unit = {

    chiselMainTest(args.drop(3), () => Module(new GarpAccel(2, 16, 11, 4))){
      c => new GarpAccelTests(c)}
    }
}
                                                      







