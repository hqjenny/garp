package sha3

import Chisel._

object common {
  def ROTL(x: UInt, y: UInt, W: UInt) = (((x) << (y)) | ((x) >> (W - (y))))
}
