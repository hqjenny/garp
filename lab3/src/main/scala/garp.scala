package garp
import Chisel._
import Array._
import sys.process._ 
import java.math.BigInteger


class GarpAccel(val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4, val R: Int=8) extends Module {
  

  val io = new Bundle {
    //val in = Vec.fill(R){Bits(INPUT, width=W)} 
    //val sel =Vec.fill(R){Bits(INPUT, width=2)}
    //val out =Vec.fill(R){Bits(OUTPUT, width=W)}
    
    val config = Vec(24 * R, Bits(INPUT, width=64))
    val Z_in = Vec(23 * R, Bits(INPUT, width=W))
    val D_in = Vec(23 * R, Bits(INPUT, width=W))
    val Z_out = Vec(23 * R, Bits(OUTPUT, width=W))
    val D_out = Vec(23 * R, Bits(OUTPUT, width=W))
    val test = Bool(INPUT)

  }

  /*val test = Vec.fill(R){Module(new TestBlockModule()).io}
  for(i <- 0 until R){
    test(i).in := io.in(i)
    test(i).sel := io.sel(i)
    io.out(i) := test(i).out  
  }*/
  
  // i is mapped to module's index
  //val garp_array = Seq.tabulate(R){index => Module(new ArrayRowModule(W=W, V=V, H=H, G=G, I=index))}
  //val garp_array = Vec.tabulate(R){index => Module(new ArrayRowModule(W=W, V=V, H=H, G=G, I=index))}
  //val garp_array = Vec.fill(R){Module(new ArrayRowModule(W=W, V=V, H=H, G=G, I=8))}
  val rows = Vec.tabulate(R){index => Module(new ArrayRowModule(W=W, V=V, H=H, G=G, I=index)).io}
  //val rows = Vec.fill(R){Module(new ArrayRowModule(W=W, V=V, H=H, G=G, I=8)).io}
   
  // Haven't figure out how to connect them 
  //val V_wire_in = Vec.fill(23*V){Bits(width=W)}
/*
  val V_wire_block = Module(new VwireModule(R=8)).io
  //val VwirePorts  = Vec.fill(R){Vec.fill(C){Vec.fill(V){new BusPort(W)}}}
       
  for (i <- 0 until R){
    // For each row
    for (j <- 0 until 23){
      for (k <- 0 until V ) {

        // For row i, col j, wire k
        rows(i).V_wire_out(j * 23 + k)
      }
    }
  }
        for (k <- 0 until C) {
          current_Vwires(k).in(row - first) := current_VwirePorts(k)(vwire_index).in
          current_Vwires(k).en(row - first) := current_VwirePorts(k)(vwire_index).en
          current_VwirePorts(k)(vwire_index).out := current_Vwires(k).out
        }
*/


  //for (i <- 0 until 23){
  //  rows(0).mem_bus_in := Bits(0)
  //}

  rows(0).mem_bus_in := Vec.fill(23){Bits(width=W)}
  rows(0).H_out_above :=  Vec.fill(23){Bits(width=W)} 
  rows(0).G_wire_above :=  Vec.fill(G){Bits(width=W)}
  rows(0).H_wire_above :=  Vec.fill(33){Bits(width=W)}

  //val rows = garp_array.map(x => x.io)
  for(i <- 1 until R){
    rows(i).G_wire_above := rows(i-1).G_wire_below
    rows(i).H_wire_above := rows(i-1).H_wire_below
    rows(i).mem_bus_in := rows(i-1).mem_bus_out
    rows(i).H_out_above := rows(i-1).H_out
  }
  
  for(i <- 0 until R){
    rows(i).test := io.test
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

  
class GarpAccelTests(c: GarpAccel) extends Tester(c) {

  var test = ""
  var config: Array[BigInt]= _
  val rand = scala.util.Random
  var Z0: BigInt= _
  var D0: BigInt= _
  var Z1: BigInt= _
  var D1: BigInt= _

  var Z_in: Array[BigInt]= _
  var D_in: Array[BigInt]= _
  var input = ""
  var output = ""
  var writer: java.io.PrintWriter= _
  var emulator_out = ""
  var data = ""
  var golden_result: List[List[String]] = _
  var D_ref: BigInt= _

  var Z_out: Array[BigInt]= _ 
  var D_out: Array[BigInt]= _ 

  var Z_out_array: Array[BigInt]=_
  var D_out_array: Array[BigInt]=_

  val delete = "Configuration of \\d+ rows loaded.\n".r
  val re = """(\S+):\s+Z\:(\S+)\s+D:(\S+)\s+H:(\S+)\s+V:(\S+)\s+G:(\S+)\s+""".r
  //val garp_path = sys.env("GARP")
  //----------------------------------------------------------------------------------------//
  // TEST 0: add: D1 = Z0 + D0
  //----------------------------------------------------------------------------------------//
  test = "add"

  // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  //val Z = BigInt(0x11111111)
  //val D = BigInt(0x11111111)

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(1)(2), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  //for(i <- 0 until 23){
  //  printf("Z_out %d %x\t", i , Z_out_array(i))
  //}
  
  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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

    //for(j <- 0 until 33){
     // printf("%x\t", H_wire_below(j))
     // printf("%x\t", H_wire_above(j))
    //}
  }
  assert (D_out(1) == D_ref)

  printf("PASS TEST0\n")
  step(1)
  poke(c.io.config, reset_config(c.R))
  step(1)

  //----------------------------------------------------------------------------------------//
  // TEST 1: add3 : Z0 + Z1 + D1 = D2
  //----------------------------------------------------------------------------------------//
  test = "add3"
  // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z1 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D1 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  // Row 0
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i)
  }
  // Row 1
  for(i <- 0 until 23){
    Z_in(23 + i) = range(Z1, 2*i+1, 2*i) 
    D_in(23 + i) = range(D1, 2*i+1, 2*i)
  }

  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sz 1 " + "%x".format(Z1) + "\n")
  writer.write("sd 1 " + "%x".format(D1) + "\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(2)(2), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  //for(i <- 0 until 23){
  //  printf("Z_out %d %x\t", i , Z_out_array(i))
  //}
  
  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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

    //for(j <- 0 until 33){
     // printf("%x\t", H_wire_below(j))
     // printf("%x\t", H_wire_above(j))
    //}
  }
  assert (D_out(2) == D_ref)

  printf("PASS TEST1\n")
  step(1)
  poke(c.io.config, reset_config(c.R))
  step(1)

  //----------------------------------------------------------------------------------------//
  // TEST 2: add3sub D2 = Z0 + Z1 - D1
  //----------------------------------------------------------------------------------------//
  test = "add3sub"
  // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  //Z1 = BigInt(1) << 8
  //Z0 = BigInt(2) << 8
  //D1 = BigInt(3) << 8
  Z1 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D1 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  // Row 0
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i)
  }
  // Row 1
  for(i <- 0 until 23){
    Z_in(23 + i) = range(Z1, 2*i+1, 2*i) 
    D_in(23 + i) = range(D1, 2*i+1, 2*i)
  }

  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sz 1 " + "%x".format(Z1) + "\n")
  writer.write("sd 1 " + "%x".format(D1) + "\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(2)(2), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  //for(i <- 0 until 23){
  //  printf("Z_out %d %x\t", i , Z_out_array(i))
  //}
  
  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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

    //for(j <- 0 until 33){
     // printf("%x\t", H_wire_below(j))
     // printf("%x\t", H_wire_above(j))
    //}
  }
  assert (D_out(2) == D_ref)

  printf("PASS TEST2\n")
  step(1)
  poke(c.io.config, reset_config(c.R))
  step(1)

  //----------------------------------------------------------------------------------------//
  // TEST 3: sub D1 = Z0 - D0
  //----------------------------------------------------------------------------------------//
  test = "sub"

  // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(1)(2), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  //for(i <- 0 until 23){
  //  printf("Z_out %d %x\t", i , Z_out_array(i))
  //}
  
  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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

    //for(j <- 0 until 33){
     // printf("%x\t", H_wire_below(j))
     // printf("%x\t", H_wire_above(j))
    //}
  }
  assert (D_out(1) == D_ref)

  printf("PASS TEST3\n")
  poke(c.io.config, reset_config(c.R))
  step(1)
  
  //----------------------------------------------------------------------------------------//
  // TEST 4: reverse Z1 = reverse(Z0)
  //----------------------------------------------------------------------------------------//
  test = "reverse"
  // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  //Z0 = BigInt(0x11111111) << 8
  //D0 = BigInt(0) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(1)(1), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  //for(i <- 0 until 23){
  //  printf("Z_out %d %x\t", i , Z_out_array(i))
  //}
  
  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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

    val G_wire_below = peek(c.rows(i).G_wire_below).reverse
    val G_wire_above = peek(c.rows(i).G_wire_above).reverse
  }
  assert (Z_out(1) == D_ref)

  printf("PASS TEST4\n")
  poke(c.io.config, reset_config(c.R))
  step(1)

  //----------------------------------------------------------------------------------------//
  // TEST 5: countlz Z2 = countlz(D0)  count leading zeros
  //-- Row 0 
  //-- For each of the 4 bytes in the word, determine whether that byte is all
  //-- zero.  Distribute the results of these tests over the global wires.
  //----------------------------------------------------------------------------------------//
  test = "countlz"

  // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  //Z0 = BigInt(0x11111111) << 8
  //D0 = BigInt(0) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  //writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(2)(1), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  step(1)
  step(1)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  //for(i <- 0 until 23){
  //  printf("Z_out %d %x\t", i , Z_out_array(i))
  //}
  
  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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

    val G_wire_below = peek(c.rows(i).G_wire_below).reverse
    val G_wire_above = peek(c.rows(i).G_wire_above).reverse
    
    //val G_wire_below_wire = combine_array(G_wire_below, 4)
    //val G_wire_above_wire = combine_array(G_wire_above, 4)

    //for(j <- 0 until 33){
     // printf("%x\t", H_wire_below(j))
     // printf("%x\t", H_wire_above(j))
    //}
  }
  assert (Z_out(2) == D_ref)

  printf("PASS TEST5\n")
  poke(c.io.config, reset_config(c.R))
  step(1)

  //----------------------------------------------------------------------------------------//
  // TEST 6:  D1 = max(D0,Z0)
  //----------------------------------------------------------------------------------------//
  test = "max"

  // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(1)(2), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  //for(i <- 0 until 23){
  //  printf("Z_out %d %x\t", i , Z_out_array(i))
  //}
  
  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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

  }
  assert (D_out(1) == D_ref)

  printf("PASS TEST6\n")
  poke(c.io.config, reset_config(c.R))
  step(1)

  //----------------------------------------------------------------------------------------//
  // TEST 7: shiftInt32Left Z3 = Z0 << D0
  //----------------------------------------------------------------------------------------//
  test = "shiftInt32Left"

  // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(3)(1), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  step(1)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  //for(i <- 0 until 23){
  //  printf("Z_out %d %x\t", i , Z_out_array(i))
  //}
  
  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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

    //for(j <- 0 until 33){
     // printf("%x\t", H_wire_below(j))
     // printf("%x\t", H_wire_above(j))
    //}
  }
  assert (Z_out(3) == D_ref)

  printf("PASS TEST7\n")
  poke(c.io.config, reset_config(c.R))
  step(1)


  //----------------------------------------------------------------------------------------//
  // TEST 8: shiftInt32Right D1 = signed(Z0 >> D0)
  //----------------------------------------------------------------------------------------//
  test = "shiftInt32Right"

 // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(3)(1), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  step(1)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  //for(i <- 0 until 23){
  //  printf("Z_out %d %x\t", i , Z_out_array(i))
  //}
  
  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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

  }
  assert (Z_out(3) == D_ref)


  printf("PASS TEST8\n")
  poke(c.io.config, reset_config(c.R))
  step(1)

  //----------------------------------------------------------------------------------------//
  // TEST 9: shiftUint32Right D1 = Z0 >> D0
  //----------------------------------------------------------------------------------------//
  test = "shiftUint32Right"

 // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(3)(1), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  step(1)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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
  }
  assert (Z_out(3) == D_ref)

  printf("PASS TEST9\n")
  poke(c.io.config, reset_config(c.R))
  step(1)

  //----------------------------------------------------------------------------------------//
  // TEST 10: divUint32By3 Z4 = Z0 / 3
  //----------------------------------------------------------------------------------------//
  test = "divUint32By3"

 // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(4)(1), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  step(1)
  step(1)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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
  }
  assert (Z_out(4) == D_ref)


  printf("PASS TEST10\n")
  poke(c.io.config, reset_config(c.R))
  step(1)

  //----------------------------------------------------------------------------------------//
  // TEST 11: divUint32By5 Z4 = Z0 / 5
  //----------------------------------------------------------------------------------------//
  test = "divUint32By5"

 // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(4)(1), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  step(1)
  step(1)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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
  }
  assert (Z_out(4) == D_ref)

  printf("PASS TEST11\n")
  poke(c.io.config, reset_config(c.R))
  step(1)

  //----------------------------------------------------------------------------------------//
  // TEST 12: divUint32By9 Z4 = Z0 / 9
  //----------------------------------------------------------------------------------------//
  test = "divUint32By9"

 // Read in  the configuration
  config = read_config(test, c.R)

   // Print all config
  config.map(x => print_config(x))

  // Generate positive random number
  Z0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8
  D0 = BigInt(rand.nextInt(Integer.MAX_VALUE)) << 8

  // Initialize array of simluation input  
  Z_in = Array.fill(c.R*23){BigInt(0)}
  D_in = Array.fill(c.R*23){BigInt(0)}
  //for(i <- 4  until 20){
  for(i <- 0 until 23){
    Z_in(i) = range(Z0, 2*i+1, 2*i) 
    D_in(i) = range(D0, 2*i+1, 2*i)
  }
 
  // Generate input for emulator
  "mkdir -p src/main/input".!
  "mkdir -p src/main/output".!
  input = "src/main/input/"+test+".in"
  output = "src/main/output/"+test+".out"
  writer = new java.io.PrintWriter(new java.io.File(input))
  writer.write("lc ../garp_config/examples/" + test + ".config\n")
  // Print Z to the input
  writer.write("sz 0 " + "%x".format(Z0) + "\n")
  writer.write("sd 0 " + "%x".format(D0) + "\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("step\n")
  writer.write("qa\n")
  writer.close()

  //println("Finished writing!")
  //"../garp_config/development/gatoconfig/build/gatoconfig " ! 
  //println(input)
  emulator_out=("../garp_config/development/ga-emulate/build/ga-emulate -x " #< new java.io.File(input)).!!
  //println(emulator_out)
  data=delete.replaceAllIn(emulator_out, "")

  // 2D array (row index) (index:0, Z:1, D:2, H:3, V:4, G:5, C:6)
  golden_result = re.findAllIn(data).matchData.toList.map(m=>m.subgroups)
  println(golden_result)
  D_ref = new BigInt(new BigInteger( golden_result(4)(1), 16))
  
  poke(c.io.Z_in, Z_in)
  poke(c.io.D_in, D_in)
  poke(c.io.test, 1)

  step(1)
  poke(c.io.config, config)
  poke(c.io.test, 0)
  step(1)
  step(1)
  step(1)
  step(1)
  //peek(c.io.config)
  // ATTENTION: peed returns an array with reverse indexing 
  Z_out_array = peek(c.io.Z_out).reverse
  D_out_array = peek(c.io.D_out).reverse

  Z_out = new Array[BigInt](0)
  D_out = new Array[BigInt](0)

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
  }
  assert (Z_out(4) == D_ref)

  printf("PASS TEST11\n")
  poke(c.io.config, reset_config(c.R))
  step(1)

  // Needs V wires
  // test = "divInt32By15"
  // test = "mul16To32_1"



  // Needs memory
  // test = "addCharVector"
  // test = "addInt32Vector"
  // test = "demandRead"

}


object GarpAccelMain {
  def main(args: Array[String]): Unit = {

    chiselMainTest(args.drop(3), () => Module(new GarpAccel(2, 16, 11, 4))){
      c => new GarpAccelTests(c)}
    }
}
                                                      







