package garp
import Chisel._
import Array._
import sys.process._ 
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

  //val test = Vec.fill(4){Module(new TestBlockModule()).io}
  
  // i is mapped to module's index
  val test = Seq.tabulate(R){index => Module(new ArrayRowModule(W=W, V=V, H=H, G=G, I=index))}
  
  // Haven't figure out how to connect them 
  val V_wire_in = Vec.fill(23*V){Bits(width=W)}

  val rows = test.map(x => x.io)
  for(i <- 1 until R){
    rows(i).G_wire_above := rows(i-1).G_wire_below
    rows(i).H_wire_above := rows(i-1).H_wire_below
    rows(i).mem_bus_in := rows(i-1).mem_bus_in
    rows(i).H_out_above := rows(i-1).H_out
  }
  
  for(i <- 0 until R){
    for (j <- 0 until 24){
      rows(i).config(j) := io.config(24 * i + j)
    }

    for (j <- 0 until 23){
      rows(i).Z_in(j) := io.Z_in(23 * i + j)
      rows(i).D_in(j) := io.D_in(23 * i + j)
      io.Z_out(23 * i + j) := rows(i).Z_out(j)
      io.D_out(23 * i + j) := rows(i).D_out(j)
    }
  }
      
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


object read_config {
  def apply(test:String) : Array[BigInt] = {
   // Read in .config
  val source = scala.io.Source.fromFile("../garp_config/examples/" + test + ".config")
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
  return config
  }
}

class GarpAccelTests(c: GarpAccel) extends Tester(c) {

  //val garp_path = sys.env("GARP")
  //***********************************************************//
  // TEST 0: add
  //***********************************************************//
  val test = "add"

  // Read in  the configuration
  val config = read_config(test)

   // Print all config
  config.map(x => print_config(x))
 

  // Generate positive random number
  val rand = scala.util.Random
  //val Z = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 4
  //val D = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 4
  val Z = BigInt(0x11111111)
  val D = BigInt(0x11111111)

  // Initialize array of simluation input  
  var Z_in = Array.fill(c.R*23){BigInt(0)}
  var D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z, 2*i+1, 2*i) 
    D_in(i) = range(D, 2*i+1, 2*i)
  }

  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  val input = "src/main/input/"+test+".in"
  val output = "src/main/output/"+test+".out"
  val writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z) + "\n")
  writer.write("sd 0 " + "%x".format(D) + "\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  println(input)
  val emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  val detele = "Configuration of \\d+ rows loaded.\n".r
  val data=detele.replaceAllIn(emulator_out, "")
  //val re = """(\d+): Z:(\d+) D:(\d+) H:(\d+) V:(\d+) G:(\d+) C:...""".r
  val re = """(\S+):\s+Z\:(\S+)\s+D:(\S+)\s+H:(\S+)\s+V:(\S+)\s+G:(\S+)\s+""".r

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  val golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  //println(golden_result)
  val D_ref = new BigInt(new BigInteger( golden_result(1)(2), 16))
  printf("D_ref%x\n", D_ref)
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)

  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  val Z_out_array = peek(c.io.Z_out).reverse
  val D_out_array = peek(c.io.D_out).reverse

  //for(i <- 0 until 23){
  //  printf("Z_out %d %x\t", i , Z_out_array(i))
  //}
  
  var Z_out = new Array[BigInt](0)
  var D_out = new Array[BigInt](0)

  for (i <- 0 until c.R){
    printf("Z_out(%d) ", i)
    //val Z_out = combine_array(Z_out_array.slice(i * 23, (i+1)*23), 23)
    Z_out = Z_out :+ combine_array(Z_out_array.slice(i * 23, (i+1)*23), 23) 
    printf("%x\t", Z_out(i));

    printf("D_out(%d) ", i)
    D_out = D_out :+ combine_array(D_out_array.slice(i * 23, (i+1)*23), 23)
    printf("%x\t", D_out(i));

    val H_wire_below = peek(c.rows(i).H_wire_below).reverse
    val H_wire_above = peek(c.rows(i).H_wire_above).reverse

    for(j <- 0 until 33){
      printf("%x\t", H_wire_below(j))
      printf("%x\t", H_wire_above(j))
    }
  }
  assert (D_out(1) == D_ref)

}


object GarpAccelMain {
  def main(args: Array[String]): Unit = {

    chiselMainTest(args.drop(3), () => Module(new GarpAccel(2, 16, 11, 4))){
      c => new GarpAccelTests(c)}
    }
}
                                                      







