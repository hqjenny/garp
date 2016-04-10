package garp

import Chisel._
import Array._
import java.math.BigInteger

object range {
  def apply(field:BigInt, end:Int, start:Int) : BigInt = {

    val clear_high = (BigInt(1) << (end+1)) - 1
    val clear_low = (BigInt(1) << start) - 1

    val mask = clear_high & ~clear_low
    return (field & mask) >> start
  }
}

object print_mode { 
  def apply(mode:BigInt, mx: BigInt) : Unit = {
    if(mode == BigInt(0)){ printf("Table Mode\n")}
    else if (mode == BigInt(1)) { printf("Split Table Mode\n")}
    else if (mode == BigInt(2)) { if(mx == BigInt(1)){printf("Partial ")} 
                                  printf("Select Mode(suppress shifts in)\n")}
    else if (mode == BigInt(3)) { if(mx == BigInt(1)){printf("Partial ")} 
                                  printf("Select Mode(not suppress shifts in)\n")}
    else if (mode == BigInt(4)) { printf("Carry Chain Mode(suppress carry in)\n")}
    else if (mode == BigInt(5)) { printf("Carry Chain Mode(not suppress carry in)\n")}
    else if (mode == BigInt(6)) { printf("Triple Add Mode(suppress shifts, carry in)\n")}
    else if (mode == BigInt(7)) { printf("Triple Add Mode(not suppress shifts,carry in)\n")}
  }
}

object print_input { 
  def apply(X_in: BigInt, X_prime: BigInt, i: Int) : Unit = {

    printf("%c_in:", i + 65)
    if(range(X_in, 5,2) == BigInt(0)){
      if(range(X_in, 1,0) == BigInt(0))     { printf("00\t")}
      else if(range(X_in, 1,0) == BigInt(1)){ printf("10\t")}
      else if(range(X_in, 1,0) == BigInt(2)){ printf("Z_reg\t")}
      else                                  { printf("D_reg\t")}
    }else{
      // Wire index is reverse order
      if(range(X_in, 5,4) == BigInt(1)){
        printf("V wire pair %d\t", BigInt(15) - range(X_in,3,0))
      }
      else if(range(X_in, 5, 5) == BigInt(1)){
        // G wires 
        if(range(X_in, 3,2) == BigInt(3)){
          printf("G wire pair %d", BigInt(3) - range(X_in,1,0))
        }else{
          printf("H wire pair %d from right", BigInt(10) - range(X_in,3,0))
        }
        if (range(X_in, 4, 4) == BigInt(1)){printf(" below\t")}
        if (range(X_in, 4, 4) == BigInt(0)){printf(" above\t")}
      }
    }

    if(X_prime == BigInt(0)) { printf("X0X0\t")}
    if(X_prime == BigInt(1)) { printf("X0X1\t")}
    if(X_prime == BigInt(2)) { printf("X1X0\t")}
    if(X_prime == BigInt(3)) { printf("X1X1\t")}
  } 
}


object combine_array {
  def apply(c:Array[BigInt], s:Int) : BigInt = {
    var a = BigInt(0)
    for(i <- (0 to (s -1)).reverse){
      //val index = s - 1 - i
      //printf("%x ", a)
      a = (c(i) << (i << 1)) | a 
    }
    return a
  }
}

object print_config {
  def apply(c:BigInt) : Unit = {
    val A_in = range(c, 63, 58)
    val B_in = range(c, 55, 50)
    val C_in = range(c, 47, 42)
    val D_in = range(c, 39, 34)
    val A_prime = range(c, 57, 56)
    val B_prime = range(c, 49, 48)
    val C_prime = range(c, 41, 40)
    val D_prime = range(c, 33, 32)
    val mx = range(c, 33, 32)
    val lut = range(c, 31, 16)
    val mode = range(c, 15, 13)
    val Z = range(c, 12, 12)
    val D = range(c, 11, 11)
    val H = range(c, 10, 10)
    val G = range(c, 9, 9)
    val V = range(c, 8, 8)
    val Gout = range(c, 7, 5)
    val Vout = range(c, 4, 0)

    print_mode(mode, mx)

    print_input(A_in, A_prime, 0)
    print_input(B_in, B_prime, 1)
    print_input(C_in, C_prime, 2)
    print_input(D_in, D_prime, 3)

    if (Z == BigInt(0)) { printf("Z:not latching\t")}
    else                { printf("Z:latching\t")}
    if (D == BigInt(0)) { printf("D:not latching\t")}
    else                { printf("D:latching\t")}
    if (H == BigInt(0)) { printf("Hout:Z\t")}
    else                { printf("Hout:D\t")}
    if (G == BigInt(0)) { printf("Gout:Z\t")}
    else                { printf("Gout:D\t")}
    if (V == BigInt(0)) { printf("Vout:Z\t")}
    else                { printf("Vout:D\t")}

    if (range(Gout, 2, 2) == BigInt(1)){
      printf("Gout: 0x%x\t", range(Gout, 1, 0))
    }else{
      printf("Gout: no output\t")
    }
    if (range(Vout, 4, 4) == BigInt(1)){
      printf("Vout: 0x%x\t", range(Vout, 3, 0))
    }else{
      printf("Vout: no output\t")
    }
    println("")
  }
}



