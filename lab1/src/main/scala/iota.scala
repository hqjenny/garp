package sha3

import Chisel._
import scala.collection.mutable.HashMap
import scala.collection.mutable.ArrayBuffer
import scala.util.Random

class IotaModule(val W: Int = 64) extends Module {
  val io = new Bundle { 
    val state_i = Vec.fill(5*5){Bits(INPUT,W)}
    val state_o = Vec.fill(5*5){Bits(OUTPUT,W)}
    val round = UInt(INPUT, 5)
  }

  for(i <- 0 until 5) {
    for(j <- 0 until 5) {
      if(i !=0 || j!=0)
        io.state_o(i*5+j) := io.state_i(i*5+j)
    }
  }
  val const = IOTA.round_const(io.round)
  io.state_o(0) := io.state_i(0) ^ const
}

class IotaModuleTests(c: IotaModule) extends Tester(c) {
    val W       = 64
    val maxInt  = 1 << (5*5*W)
      val round = 0
      val state = Array.fill(5*5){BigInt(3)}
      val out_state = Array.fill(5*5){BigInt(3)}
      out_state(0) = state(0) ^ BigInt(1)
      poke(c.io.state_i, state)
      poke(c.io.round, round)
      step(1)
      expect(c.io.state_o, out_state)
}
object iotaMain { 
  def main(args: Array[String]): Unit = {
    chiselMainTest(args,
    () => Module(new IotaModule())){c => new IotaModuleTests(c)
    }
  }
}

