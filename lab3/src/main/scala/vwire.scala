package garp
import Chisel._

class BusPort(val W: Int=2) extends Bundle {
    val in = Bits(INPUT,W)
    val en = Bits(INPUT,1)
    val out = Bits(OUTPUT,W)
}

/*
Unfortunately black boxes with differing numbers of inputs need to be used for vwires.
I am sure there must be a much better way but for now all of the modules are listed.
Then a parameterized module selects between the Vwires so the code that uses them doesn't need to know.

*/

class Vwire2BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(2){Bits(INPUT, width=W)}
    val en = Vec.fill(2){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
  val VwireBlock = Module(new VwireBlackBox(W=W, L=2)).io
  VwireBlock.in := io.in
  VwireBlock.en := io.en
  io.out := VwireBlock.out

}

class Vwire3BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(3){Bits(INPUT, width=W)}
    val en = Vec.fill(3){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
  val VwireBlock = Module(new VwireBlackBox(W=W, L=3)).io
  VwireBlock.in := io.in
  VwireBlock.en := io.en
  io.out := VwireBlock.out

}

class Vwire4BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(4){Bits(INPUT, width=W)}
    val en = Vec.fill(4){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
  val VwireBlock = Module(new VwireBlackBox(W=W, L=4)).io
  VwireBlock.in := io.in
  VwireBlock.en := io.en
  io.out := VwireBlock.out

}

class Vwire6BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(6){Bits(INPUT, width=W)}
    val en = Vec.fill(6){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
  val VwireBlock = Module(new VwireBlackBox(W=W, L=6)).io
  VwireBlock.in := io.in
  VwireBlock.en := io.en
  io.out := VwireBlock.out

}

class Vwire8BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(8){Bits(INPUT, width=W)}
    val en = Vec.fill(8){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
  val VwireBlock = Module(new VwireBlackBox(W=W, L=8)).io
  VwireBlock.in := io.in
  VwireBlock.en := io.en
  io.out := VwireBlock.out

}

class Vwire12BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(12){Bits(INPUT, width=W)}
    val en = Vec.fill(12){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
  val VwireBlock = Module(new VwireBlackBox(W=W, L=12)).io
  VwireBlock.in := io.in
  VwireBlock.en := io.en
  io.out := VwireBlock.out
}

class Vwire16BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(16){Bits(INPUT, width=W)}
    val en = Vec.fill(16){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
  val VwireBlock = Module(new VwireBlackBox(W=W, L=16)).io
  VwireBlock.in := io.in
  VwireBlock.en := io.en
  io.out := VwireBlock.out
}

class Vwire24BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(24){Bits(INPUT, width=W)}
    val en = Vec.fill(24){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
  val VwireBlock = Module(new VwireBlackBox(W=W, L=24)).io
  VwireBlock.in := io.in
  VwireBlock.en := io.en
  io.out := VwireBlock.out
}

class Vwire32BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(32){Bits(INPUT, width=W)}
    val en = Vec.fill(32){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
  val VwireBlock = Module(new VwireBlackBox(W=W, L=32)).io
  VwireBlock.in := io.in
  VwireBlock.en := io.en
  io.out := VwireBlock.out
}

class Vwire(val N: Int, val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(N){Bits(INPUT, width=W)}
    val en = Vec.fill(N){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  val VwireBB = N match {
    case 2 => Module(new Vwire2BB()).io
    case 3 => Module(new Vwire3BB()).io
    case 4 => Module(new Vwire4BB()).io
    case 6 => Module(new Vwire6BB()).io
    case 8 => Module(new Vwire8BB()).io
    case 12 => Module(new Vwire12BB()).io
    case 16 => Module(new Vwire16BB()).io
    case 24 => Module(new Vwire24BB()).io
    case 32 => Module(new Vwire32BB()).io
    case _ => Module(new Vwire32BB()).io //Default to longest wire. Fails because vec lengths are mismatched.
  }
  VwireBB.in := io.in
  VwireBB.en := io.en
  io.out := VwireBB.out
}

object calc_base_index {
  def apply(nom_len:Int, nom_first:Int) : Int = {
    if (nom_len == 4) {
      return 2*(((nom_first%2)+2)%2)
    } else {
      return 2*(BigInt(nom_len).bitLength - 2)
    }
  }
}

object shift_index {
  def apply(nom_len:Int, nom_first:Int, row:Int) : Int = {
    val fourth = nom_len / 4
    if(row < (nom_first + fourth) || row >= (nom_first + 3 * fourth)) {
      return 0
    } else {
      return 1
    }
  }
}

class VwireModule(val R: Int = 8, val C: Int = 23, val V: Int = 16, val W: Int = 2) extends Module {
  val io = new Bundle { 
    val VwirePorts  = Vec.fill(R){Vec.fill(C){Vec.fill(V){new BusPort(W)}}}
  }

  val printout = true

  val wire_consts = R match {
    case 4 => VWIRECONSTANTS.vwire_4
    case 8 => VWIRECONSTANTS.vwire_8
    case 16 => VWIRECONSTANTS.vwire_16
    case 32 => VWIRECONSTANTS.vwire_32
    case 64 => VWIRECONSTANTS.vwire_64
    case 128 => VWIRECONSTANTS.vwire_128
    case 256 => VWIRECONSTANTS.vwire_256
    case _ => VWIRECONSTANTS.vwire_256 //Default to largest set, will probably result in errors.
  }

  for (i <- 0 until wire_consts.length) {
    val wire_bundle = wire_consts(i).asInstanceOf[Array[Any]]
    val actual_len = wire_bundle(0).asInstanceOf[Int]
    if (printout) {
      println("Generate Vwires with actual length " + actual_len)
    }
    for (j <- 1 until wire_bundle.length) {
      val wire = wire_bundle(j).asInstanceOf[Array[Int]]
      val nom_len = wire(0)
      val nom_first = wire(1)
      val first = wire(2)
      val last = wire(3)
      val base_index = calc_base_index(nom_len, nom_first)
      if (printout) {
        println("\tGenerate specific Vwire with properties {nom_len: " + nom_len + " nom_first: " + nom_first + " first: " + first + " last: " + last + "}")
      }
      //Generate C copies of this wire to add to each column
      val current_Vwires = Vec.fill(C){Module(new Vwire(actual_len)).io}
      for (row <- first to last) {
        // Calculate which index this wire should connect to in the logic block
        val vwire_index = base_index + shift_index(nom_len, nom_first, row)
        val current_VwirePorts = io.VwirePorts(row)
        if (printout) {
          println("\t\t Connecting Vwire port " + (row - first) + " to array port in row " + row + " at index " + vwire_index)
        }
        for (k <- 0 until C) {
          current_Vwires(k).in(row - first) := current_VwirePorts(k)(vwire_index).in
          current_Vwires(k).en(row - first) := current_VwirePorts(k)(vwire_index).en
          current_VwirePorts(k)(vwire_index).out := current_Vwires(k).out
        }
      }
    }
  }
}

class VwireModuleTests(c: VwireModule) extends Tester(c) {
      /*
      poke(c.io.VwirePorts(31)(21).in, 0)
      peek(c.io.VwirePorts(30)(20).out)
      peek(c.io.VwirePorts(31)(21).out)
      step(1)
      poke(c.io.VwirePorts(31)(21).in, 2)
      peek(c.io.VwirePorts(30)(20).out)
      peek(c.io.VwirePorts(31)(21).out)
      */
}

object VwireModuleMain { 
  def main(args: Array[String]): Unit = {
    //chiselMainTest(Array[String]("--backend", "c", "--genHarness", "--compile", "--test"),
    chiselMainTest(args,
    () => Module(new VwireModule())){c => new VwireModuleTests(c)
    }
  }
}

