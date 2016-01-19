package sha3

import Chisel._
import Node._
import scala.collection.mutable.HashMap
import scala.collection.mutable.ArrayBuffer
import scala.util.Random

class DpathModule(val W: Int) extends Module {
  //constants
  val r = 2*256
  val c = 25*W - r
  val round_size_words = c/W
  val rounds = 24 //12 + 2l
  val hash_size_words = 256/W
  val bytes_per_word = W/8

  val io = new Bundle { 
    //YOU MAY ADD MORE PORTS
  }
  //YOUR IMPLEMENTATION HERE
}
