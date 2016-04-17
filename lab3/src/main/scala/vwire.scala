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
}

class Vwire3BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(3){Bits(INPUT, width=W)}
    val en = Vec.fill(3){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
}

class Vwire4BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(4){Bits(INPUT, width=W)}
    val en = Vec.fill(4){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
}

class Vwire6BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(6){Bits(INPUT, width=W)}
    val en = Vec.fill(6){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
}

class Vwire8BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(8){Bits(INPUT, width=W)}
    val en = Vec.fill(8){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
}

class Vwire12BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(12){Bits(INPUT, width=W)}
    val en = Vec.fill(12){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
}

class Vwire16BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(16){Bits(INPUT, width=W)}
    val en = Vec.fill(16){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
}

class Vwire24BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(24){Bits(INPUT, width=W)}
    val en = Vec.fill(24){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
}

class Vwire32BB(val W: Int=2) extends Module {
  val io = new Bundle {
    val in = Vec.fill(32){Bits(INPUT, width=W)}
    val en = Vec.fill(32){Bits(INPUT, width=1)}
    val out = Bits(OUTPUT, width=W)
  }
  //Need blackbox
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

class VwireModule(val R: Int = 32, val C: Int = 23, val W: Int = 2) extends Module {
  val io = new Bundle { 
    val VwirePorts  = Vec.fill(R){Vec.fill(C){new BusPort(W)}}
  }

  val wire_consts = VWIRECONSTANTS.vwire_32
  for (row <- wire_consts) {
    println(row(0))
    println(row.slice(1, row.length).deep.mkString(","))
  }
  io.VwirePorts(30)(20).out := io.VwirePorts(31)(21).in
  val wirea = Module(new Vwire(2))
  val wireb = Module(new Vwire(3))
  val wirec = Module(new Vwire(8))
}

class VwireModuleTests(c: VwireModule) extends Tester(c) {
      poke(c.io.VwirePorts(31)(21).in, 0)
      peek(c.io.VwirePorts(30)(20).out)
      peek(c.io.VwirePorts(31)(21).out)
      step(1)
      poke(c.io.VwirePorts(31)(21).in, 2)
      peek(c.io.VwirePorts(30)(20).out)
      peek(c.io.VwirePorts(31)(21).out)
}

object VwireModuleMain { 
  def main(args: Array[String]): Unit = {
    //chiselMainTest(Array[String]("--backend", "c", "--genHarness", "--compile", "--test"),
    chiselMainTest(args,
    () => Module(new VwireModule())){c => new VwireModuleTests(c)
    }
  }
}

