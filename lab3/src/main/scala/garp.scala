package garp

import Chisel._
import Array._
import java.math.BigInteger


class GarpAccel(val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4, val R: Int=2) extends Module {
  

  val io = new Bundle {
    val in = Vec.fill(4){Bits(INPUT, width=W)} 
    val sel =Vec.fill(4){Bits(INPUT, width=2)}
    val out =Vec.fill(4){Bits(OUTPUT, width=W)}
    
    val config = Vec.fill(24*R){Bits(INPUT, width=64)}
    val Z_in = Vec.fill(23*R){Bits(INPUT, width=W)}
    val D_in = Vec.fill(23*R){Bits(INPUT, width=W)}
    val Z_out = Vec.fill(23*R){Bits(OUTPUT, width=W)}
    val D_out = Vec.fill(23*R){Bits(OUTPUT, width=W)}
    val test = Bool(INPUT)

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
  //val row2 = Module(new ArrayRowModule(W, I=2)).io

  row1.G_wire_above := row0.G_wire_below
  //row2.G_wire_above := row1.G_wire_below

  row1.H_wire_above := row0.H_wire_below
  //row2.H_wire_above := row1.H_wire_below

  row1.mem_bus_in := row0.mem_bus_out
  //row2.mem_bus_in := row1.mem_bus_out

  row1.H_out_above := row0.H_out
  //row2.H_out_above := row1.H_out
  
  for (i <- 0 until 24){
    row0.config(i) := io.config(0+i)    
    row1.config(i) := io.config(24+i)   
    //row2.config(i) := io.config(48+i)   
  }

  row0.test := io.test
  row1.test := io.test

  for (i <- 0 until 23){
    row0.Z_in(i) := io.Z_in(0+i)
    row0.D_in(i) := io.D_in(0+i)
    row1.Z_in(i) := io.Z_in(23+i)
    row1.D_in(i) := io.D_in(23+i)
  
    io.Z_out(i) := row0.Z_out(i)
    io.D_out(i) := row0.D_out(i)
    io.Z_out(23+i) := row1.Z_out(i)
    io.D_out(23+i) := row1.D_out(i)
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

object range {
  def apply(field:BigInt, end:Int, start:Int) : BigInt = {

    val clear_high = (BigInt(1) << (end+1)) - 1
    val clear_low = (BigInt(1) << start) - 1

    val mask = clear_high & ~clear_low
    return (field & mask) >> start
  }
}

object print_mode { 
  def apply(mode:BigInt, mx: BigInt) : Unit = {
    if(mode == BigInt(0)){ printf("Table Mode\n")}
    else if (mode == BigInt(1)) { printf("Split Table Mode\n")}
    else if (mode == BigInt(2)) { if(mx == BigInt(1)){printf("Partial ")} 
                                  printf("Select Mode(suppress shifts in)\n")}
    else if (mode == BigInt(3)) { if(mx == BigInt(1)){printf("Partial ")} 
                                  printf("Select Mode(not suppress shifts in)\n")}
    else if (mode == BigInt(4)) { printf("Carry Chain Mode(suppress carry in)\n")}
    else if (mode == BigInt(5)) { printf("Carry Chain Mode(not suppress carry in)\n")}
    else if (mode == BigInt(6)) { printf("Triple Add Mode(suppress shifts, carry in)\n")}
    else if (mode == BigInt(7)) { printf("Triple Add Mode(not suppress shifts,carry in)\n")}
  }
}


object print_input { 
  def apply(X_in: BigInt, X_prime: BigInt, i: Int) : Unit = {

    printf("%c_in:", i + 65)
    if(range(X_in, 5,2) == BigInt(0)){
      if(range(X_in, 1,0) == BigInt(0))     { printf("00\t")}
      else if(range(X_in, 1,0) == BigInt(1)){ printf("10\t")}
      else if(range(X_in, 1,0) == BigInt(2)){ printf("Z_reg\t")}
      else                                  { printf("D_reg\t")}
    }else{
      // Wire index is reverse order
      if(range(X_in, 5,4) == BigInt(1)){
        printf("V wire pair %d\t", BigInt(15) - range(X_in,3,0))
      }
      else if(range(X_in, 5, 5) == BigInt(1)){
        // G wires 
        if(range(X_in, 3,2) == BigInt(3)){
          printf("G wire pair %d", BigInt(3) - range(X_in,1,0))
        }else{
          printf("H wire pair %d from right", BigInt(10) - range(X_in,3,0))
        }
        if (range(X_in, 4, 4) == BigInt(1)){printf(" below\t")}
        if (range(X_in, 4, 4) == BigInt(0)){printf(" above\t")}
      }
    }

    if(X_prime == BigInt(0)) { printf("X0X0\t")}
    if(X_prime == BigInt(1)) { printf("X0X1\t")}
    if(X_prime == BigInt(2)) { printf("X1X0\t")}
    if(X_prime == BigInt(3)) { printf("X1X1\t")}
  } 
}

object print_config {
  def apply(c:BigInt) : Unit = {
    val A_in = range(c, 63, 58)
    val B_in = range(c, 55, 50)
    val C_in = range(c, 47, 42)
    val D_in = range(c, 39, 34)
    val A_prime = range(c, 57, 56)
    val B_prime = range(c, 49, 48)
    val C_prime = range(c, 41, 40)
    val D_prime = range(c, 33, 32)
    val mx = range(c, 33, 32)
    val lut = range(c, 31, 16)
    val mode = range(c, 15, 13)
    val Z = range(c, 12, 12)
    val D = range(c, 11, 11)
    val H = range(c, 10, 10)
    val G = range(c, 9, 9)
    val V = range(c, 8, 8)
    val Gout = range(c, 7, 5)
    val Vout = range(c, 4, 0)

    print_mode(mode, mx)

    print_input(A_in, A_prime, 0)
    print_input(B_in, B_prime, 1)
    print_input(C_in, C_prime, 2)
    print_input(D_in, D_prime, 3)

    if (Z == BigInt(0)) { printf("Z:not latching\t")}
    else                { printf("Z:latching\t")}
    if (D == BigInt(0)) { printf("D:not latching\t")}
    else                { printf("D:latching\t")}
    if (H == BigInt(0)) { printf("Hout:Z\t")}
    else                { printf("Hout:D\t")}
    if (G == BigInt(0)) { printf("Gout:Z\t")}
    else                { printf("Gout:D\t")}
    if (V == BigInt(0)) { printf("Vout:Z\t")}
    else                { printf("Vout:D\t")}

    if (range(Gout, 2, 2) == BigInt(1)){
      printf("Gout: 0x%x\t", range(Gout, 1, 0))
    }else{
      printf("Gout: no output\t")
    }
    if (range(Vout, 4, 4) == BigInt(1)){
      printf("Vout: 0x%x\t", range(Vout, 3, 0))
    }else{
      printf("Vout: no output\t")
    }
    println("")
  }
}



class GarpAccelTests(c: GarpAccel) extends Tester(c) {

  //poke(c.io.in(0), 1)
  //poke(c.io.H_out_above, 1)

  val source = scala.io.Source.fromFile("src/main/config/test.config")
  val lines = try source.mkString finally source.close()
  val list = lines.split('\n').flatMap(x => x.split(',').map(x => x.trim))
  val list_strip = list.slice(2, list.length-1).map(x => x.replaceFirst("0x",""))
  var config = new Array[BigInt](0)
  for(i <- 0 until list_strip.length){
    if(i%2 != 0){
      config = config :+ new BigInt(new BigInteger( list_strip(i-1)+list_strip(i), 16)) 
    }
  }
  println(list_strip)
  for(i <- 0 until config.length){
    printf("%x ", config(i))
  }
  // Print all config
  config.map(x => print_config(x))

  poke(c.io.config, config)
  peek(c.io.config)
  step(1)




}


object GarpAccelMain {
  def main(args: Array[String]): Unit = {

    chiselMainTest(args.drop(3), () => Module(new GarpAccel(2, 16, 11, 4))){
      c => new GarpAccelTests(c)}
    }
}
                                                      







