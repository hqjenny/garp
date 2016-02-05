package sha3

import Chisel._
import Node._
import scala.collection.mutable.HashMap
import scala.collection.mutable.ArrayBuffer
import scala.util.Random

class DpathModule(val W: Int, val S: Int) extends Module {
  //constants
  val r = 2*256
  val c = 25*W - r
  val round_size_words = c/W
  val rounds = 24 //12 + 2l
  val hash_size_words = 256/W
  val bytes_per_word = W/8

  val io = new Bundle { 
    val absorb = Bool(INPUT)
    val init   = Bool(INPUT)
    val write  = Bool(INPUT)
    val round  = UInt(INPUT,width=5)
    val stage  = UInt(INPUT,width=log2Up(S))
    val message_in = Vec.fill(round_size_words){Bits(INPUT, width = W)}
    val hash_out = Vec.fill(hash_size_words){Bits(OUTPUT, width = W)}
  }

  val state = Vec.fill(5*5){Reg(init = Bits(0, width = W))}

  //submodules
  val theta = Module(new ThetaModule(W)).io
  val rhopi = Module(new RhoPiModule(W)).io
  val chi   = Module(new ChiModule(W)).io
  val iota  = Module(new IotaModule(W))

  //default
  theta.state_i := Vec.fill(25){Bits(0,W)}
  iota.io.round     := UInt(0)

  //connect submodules to each other
  theta.state_i := state
  rhopi.state_i <> theta.state_o
  chi.state_i   <> rhopi.state_o
  iota.io.state_i  <> chi.state_o
  state         := iota.io.state_o

  iota.io.round    := io.round
  
  when(io.absorb){
    state := state
    for( i <- 0 until round_size_words){
      state((i%5)*5+(i/5)) := state((i%5)*5+(i/5)) ^ io.message_in(i)
    }
  }

  val hash_res = Vec.fill(hash_size_words){Bits(width = W)}
  for( i <- 0 until hash_size_words){
    io.hash_out(i) := state(i*5)
  }

  //keep state from changing while we write
  when(io.write){
    state := state
  }

  //initialize state to 0 for new hashes or at reset
  when(io.init){
    state := Vec.fill(5*5){Bits(0, width = W)}
  }

  when(reset){
    state := Vec.fill(5*5){Bits(0, width = W)}
  }
}
