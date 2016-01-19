package sha3

import Chisel._

class Sha3Accel(val W: Int) extends Module {
  //constants
  val r = 2*256
  val c = 25*W - r
  val round_size_words = c/W
  val rounds = 24 //12 + 2l
  val hash_size_words = 256/W
  val bytes_per_word = W/8

  val io = new Bundle {
    val message = Decoupled(Vec.fill(round_size_words){Bits(INPUT, width=W)}).flip
    val hash    = Decoupled(Vec.fill(hash_size_words){Bits(OUTPUT, width=W)})
  }

  val ctrl = Module(new CtrlModule(W))
  
  ctrl.io.message_val <> io.message.valid
  ctrl.io.message_rdy <> io.message.ready

  ctrl.io.hash_val    <> io.hash.valid
  ctrl.io.hash_rdy    <> io.hash.ready

  val dpath = Module(new DpathModule(W))

}

class Sha3AccelTests(c: Sha3Accel) extends Tester(c) {
  //This test is the same as the one in the keccak code on github
  //https://github.com/gvanas/KeccakCodePackage/blob/master/TestVectors/KeccakF-1600-IntermediateValues.txt
  val test_message = Array.fill(c.round_size_words){BigInt(0)}
  val test_hash = Array(BigInt("F1258F7940E1DDE7",16),
                        BigInt("84D5CCF933C0478A",16),
                        BigInt("D598261EA65AA9EE",16),
                        BigInt("BD1547306F80494D",16))
  poke(c.io.message.bits, test_message)
  poke(c.io.message.valid, 1)
  poke(c.io.hash.ready, 1)
  do {
    step(1)
    poke(c.io.message.valid, 0)
  }while(peek(c.io.hash.valid) == 0)
  expect(c.io.hash.valid, 1)
  expect(c.io.hash.bits, test_hash)
}

object Sha3AccelMain {
  def main(args: Array[String]): Unit = {
    chiselMainTest(args, () => Module(new Sha3Accel(64))){
                c => new Sha3AccelTests(c)}
              }
            }
