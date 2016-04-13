package sha3

import Chisel._

class SizeModule(val S: Int = 511, val W: Int = 1) extends Module {
  val io = new Bundle { 
    val inbits = Vec.fill(S){Bits(INPUT,W)}
    val outbits = Vec.fill(S){Bits(OUTPUT,W)}
  }
  for(i <- 0 until S) {
  	io.outbits(i) := io.inbits(i)
  }
}

class SizeModuleTests(c: SizeModule) extends Tester(c) {
      step(1)
}

object sizeMain { 
  def main(args: Array[String]): Unit = {
    //chiselMainTest(Array[String]("--backend", "c", "--genHarness", "--compile", "--test"),
    chiselMainTest(args,
    () => Module(new SizeModule())){c => new SizeModuleTests(c)
    }
  }
}

