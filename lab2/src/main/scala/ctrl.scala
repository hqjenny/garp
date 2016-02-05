package sha3

import Chisel._
import scala.collection.mutable.HashMap
import scala.collection.mutable.ArrayBuffer
import scala.util.Random

class CtrlModule(val W: Int, val S: Int) extends Module with MemoryOpConstants{
  val r = 2*256
  val c = 25*W - r
  val round_size_words = c/W
  val rounds = 24 //12 + 2l
  val hash_size_words = 256/W
  val bytes_per_word = W/8

  val io = new Bundle { 
    val rocc_req_val      = Bool(INPUT)
    val rocc_req_rdy      = Bool(OUTPUT)
    val rocc_funct        = Bits(INPUT, 2)
    val rocc_rs1          = Bits(INPUT, 64)
    val rocc_rs2          = Bits(INPUT, 64)
    val rocc_rd           = Bits(INPUT, 5)

    val busy              = Bool(OUTPUT)

    val dmem_req_val      = Bool(OUTPUT)
    val dmem_req_rdy      = Bool(INPUT)
    val dmem_req_tag      = Bits(OUTPUT, 7)
    val dmem_req_addr     = Bits(OUTPUT, 32)
    val dmem_req_cmd      = Bits(OUTPUT, M_SZ)
    val dmem_req_typ      = Bits(OUTPUT, MT_SZ)

    val dmem_resp_val     = Bool(INPUT) 
    val dmem_resp_tag     = Bits(INPUT, 7)
    val dmem_resp_data    = Bits(INPUT, W)

    //Sha3 Specific signals
    val round       = UInt(OUTPUT,width=5)
    val stage       = UInt(OUTPUT,width=log2Up(S))
    val absorb      = Bool(OUTPUT)
    val init        = Bool(OUTPUT)
    val write       = Bool(OUTPUT)
    val windex      = UInt(OUTPUT,width=log2Up(hash_size_words+1))

    val buffer_out  = Vec.fill(round_size_words){Bits(OUTPUT,width=W)}
  }

  //RoCC HANDLER
  //rocc pipe state
  val r_idle :: r_eat_addr :: r_eat_len :: Nil = Enum(UInt(), 3)

  val msg_addr = Reg(init = UInt(0,64))
  val hash_addr= Reg(init = UInt(0,64))
  val msg_len  = Reg(init = UInt(0,64))

  val busy = Reg(init=Bool(false))

  val rocc_s = Reg(init=r_idle)
  //register inputs 
  val rocc_req_val_reg = Reg(next=io.rocc_req_val)
  val rocc_funct_reg = Reg(init = Bits(0,2))
  rocc_funct_reg := io.rocc_funct
  val rocc_rs1_reg = Reg(next=io.rocc_rs1)
  val rocc_rs2_reg = Reg(next=io.rocc_rs2)
  val rocc_rd_reg = Reg(next=io.rocc_rd)

  val dmem_resp_tag_reg = Reg(next=io.dmem_resp_tag)
  //memory pipe state
  val m_idle :: m_read :: m_wait :: m_pad :: m_absorb :: Nil = Enum(UInt(), 5)
  val mem_s = Reg(init=m_idle)


  val buffer  = Vec.fill(round_size_words){Reg(init = Bits(0, width=W))} 
  val buffer_valid = Reg(init = Bool(false))
  val buffer_count = Reg(init = UInt(0,5))
  val read    = Reg(init = UInt(0,32))
  val hashed  = Reg(init = UInt(0,32))
  val mindex  = Reg(init = UInt(0,5))
  val windex  = Reg(init = UInt(0,log2Up(hash_size_words+1)))
  val writes_done  = Vec.fill(hash_size_words){Reg(init = Bool(false))}
  val next_buff_val = Reg(next=
  (mindex >= UInt(round_size_words)) ||
  (read > msg_len)
  )

  io.buffer_out := buffer
  io.windex     := windex

  //misc padding signals
  val first_pad = Bits("b0000_0001")
  val last_pad  = Bits("b1000_0000")
  val both_pad  = first_pad | last_pad
  //last word with message in it
  val words_filled = mindex
  //last byte with message in it
  val byte_offset = (msg_len)%UInt(bytes_per_word)

  //hasher state
  val s_idle :: s_absorb :: s_hash :: s_write :: Nil = Enum(UInt(), 4)

  val state = Reg(init=s_idle)


  val rindex = Reg(init = UInt(rounds+1,5))
  val sindex = Reg(init = UInt(0,log2Up(S)))

  //default
  io.rocc_req_rdy := Bool(false)
  io.init := Bool(false)
  io.busy := busy
  io.round       := rindex
  io.stage       := sindex
  io.absorb      := Bool(false)
  io.write       := Bool(true)
  io.dmem_req_val:= Bool(false)
  io.dmem_req_tag:= rindex
  io.dmem_req_addr:= Bits(0, 32)
  io.dmem_req_cmd:= M_XRD
  io.dmem_req_typ:= MT_D

  val rindex_reg = Reg(next=rindex)


  switch(rocc_s) {
  is(r_idle) {
    io.rocc_req_rdy := !busy
    when(io.rocc_req_val && !busy){
      when(rocc_funct_reg === UInt(0)){
        io.rocc_req_rdy := Bool(true)
        msg_addr  := rocc_rs1_reg
        hash_addr := rocc_rs2_reg
        io.busy := Bool(true)
      }.elsewhen(rocc_funct_reg === UInt(1)) {
        busy := Bool(true)
        io.rocc_req_rdy := Bool(true)
        io.busy := Bool(true)
        msg_len := rocc_rs1_reg
      }
    }
  }
  }

  //END RoCC HANDLER
  //START MEM HANDLER


  switch(mem_s){
  is(m_idle){
  //we can start filling the buffer if we aren't writing and if we got a new message 
    //or the hashing started 
      //and there is more to read
      //and the buffer has been absorbed
    val canRead = busy && ( (read < msg_len || (read === msg_len && msg_len === UInt(0)) ) &&
                  (!buffer_valid || ((buffer_count === UInt(0)))))
    when(canRead){
      buffer := Vec.fill(round_size_words){Bits(0,W)}
      buffer_count := UInt(0)
      mindex := UInt(0)
      mem_s := m_read
    }.otherwise{
      mem_s := m_idle
    }
  }
  is(m_read) {
    //dmem signals
    //only read if we aren't writing
    when(state =/= s_write){
      io.dmem_req_val := read < msg_len && mindex < UInt(round_size_words)
      io.dmem_req_addr := msg_addr + (mindex << UInt(3))
      io.dmem_req_tag := mindex
      io.dmem_req_cmd := M_XRD
      io.dmem_req_typ := MT_D

      when(io.dmem_req_rdy && io.dmem_req_val){
        mindex := mindex + UInt(1)
        read := read + UInt(8)//read 8 bytes
        mem_s := m_wait
      }.otherwise{//didn't fire so keep sending
        mem_s := m_read
      }
      //TODO: don't like special casing this
      when(msg_len === UInt(0)){
        read := UInt(1)
      }
    }
  }
  is(m_wait){
    //the code to process read responses
    when(io.dmem_resp_val) {
      //This is read response
      buffer(mindex-UInt(1)) := io.dmem_resp_data
      buffer_count := buffer_count + UInt(1)

      when(mindex >= UInt(round_size_words)){
        when(read > msg_len){
          //nothing
        }.otherwise{
          buffer_valid := Bool(true)
        }
      }
    }
    
    //next state
    when(mindex < UInt(round_size_words- 1)){
      //TODO: in pad check buffer_count ( or move on to next thread?)
      when(msg_len > read){
        //not sure if this case will be used but this means we haven't
        //sent all the requests yet (maybe back pressure causes this)
        when((msg_len+UInt(8)) < read){
          buffer_valid := Bool(false)
          mem_s := m_pad
        }
        mem_s := m_read
      }.otherwise{
        //its ok we didn't send them all because the message wasn't big enough
          buffer_valid := Bool(false)
        mem_s := m_pad
      }
    }.otherwise{
      when(mindex < UInt(round_size_words) &&
           !(io.dmem_req_rdy && io.dmem_req_val)){
        //we are still waiting to send the last request
        mem_s := m_read
      }.otherwise{
        //we have reached the end of this chunk
        mindex := mindex + UInt(1)
        read := read + UInt(8)//read 8 bytes
        //we sent all the requests
        msg_addr := msg_addr + UInt(round_size_words << 3)
        when((msg_len < (read+UInt(8) ))){
          //but the buffer still isn't full
          buffer_valid := Bool(false)
          mem_s := m_pad
        }.otherwise{
          //we have more to read eventually
          mem_s := m_idle
        }
      }
    }
  }
  is(m_pad) {
    //local signals
    //make sure we have received all the responses for this message
    buffer_valid := next_buff_val

    //local state
    //first pad
    //set everything to 0000_000 after end of message first
    for(i <- 0 until (round_size_words - 1)){
      when(UInt(i) <= words_filled) {
        //dont overwrite message
      }.otherwise{
        buffer(i) := Bits(0,W)
      }
    }
    //now put in pad bits
    when(byte_offset === UInt(0) && words_filled > UInt(0)) {
      //last byte in the word so we need to put start in next word
      when(words_filled >= UInt(round_size_words -1)) {
        //unless were at the end
        //this is a failure we should not reach this part
      }.otherwise{
        buffer(words_filled+UInt(1)) := Cat(Bits(0,W-8),first_pad)
        buffer(round_size_words - 1) := 
          Cat(last_pad, buffer(round_size_words -1)((bytes_per_word-1)*8-1, 0))
      }
    }.otherwise{
      //not last byte so we put first pad here
      when(byte_offset === UInt(0)){
        buffer(words_filled) := first_pad
      }.elsewhen(byte_offset === UInt(1)){
        buffer(words_filled) := Cat(first_pad,buffer(words_filled)(7,0))
      }.elsewhen(byte_offset === UInt(2)){
        buffer(words_filled) := Cat(first_pad,buffer(words_filled)(15,0))
      }.elsewhen(byte_offset === UInt(3)){
        buffer(words_filled) := Cat(first_pad,buffer(words_filled)(23,0))
      }.elsewhen(byte_offset === UInt(4)){
        buffer(words_filled) := Cat(first_pad,buffer(words_filled)(31,0))
      }.elsewhen(byte_offset === UInt(5)){
        buffer(words_filled) := Cat(first_pad,buffer(words_filled)(39,0))
      }.elsewhen(byte_offset === UInt(6)){
        buffer(words_filled) := Cat(first_pad,buffer(words_filled)(47,0))
      }
      //and put the last pad
      when((words_filled === UInt(round_size_words - 1)) && (byte_offset === UInt(bytes_per_word -1))){
        //together with the first pad
        buffer(words_filled) := Cat(both_pad, buffer(words_filled)(55,0))
      }.otherwise{
        //at the end of the last word
        buffer(round_size_words - 1) := Cat(last_pad, buffer(round_size_words -1)((bytes_per_word-1)*8-1,0))
      }
    }
    //next state 
    when(next_buff_val){
    //we have received all responses so the buffer is as full as it will get
      when(io.absorb && rindex_reg >= UInt(rounds)){
        //we already started absorbing so skip to idle and go to next thread
        buffer_count := UInt(0)
        mindex := UInt(0)//reset this for absorb
        mem_s := m_idle
      }.otherwise{
        mem_s := m_absorb
      }
    }.otherwise{
      mem_s := m_pad
    }
  }
  is(m_absorb){
    buffer_valid := Bool(true)
    //move to idle when we know this thread was absorbed
    when(io.absorb && rindex_reg >= UInt(rounds)){
      buffer_valid := Bool(false)
        buffer_count := UInt(0)
      
      mem_s := m_idle
    }
  }
  }
  //END MEM HANDLER


  switch(state) {
  is(s_idle) {
  val canAbsorb = busy && (rindex_reg >= UInt(rounds) && buffer_valid && hashed <= msg_len)
    when(canAbsorb){
      io.absorb := Bool(true)
      io.write := Bool(false)
      busy  := Bool(true)
      rindex := UInt(0)
      io.round := UInt(0)
      sindex := UInt(0)
      hashed := hashed + UInt(8*round_size_words)
      state := s_hash
      buffer_valid := Bool(false)
      buffer_count := UInt(0)
    }.otherwise{
      state := s_idle
    }
  }
  is(s_absorb){
    state := s_hash
    rindex := UInt(0)
    sindex := UInt(0)
    io.write := Bool(false)
  }
  is(s_hash){
    when(rindex < UInt(rounds)){
      rindex := rindex + UInt(1)
      io.round := rindex
      io.write := Bool(false)
      state := s_hash
    }.otherwise{
      io.write := Bool(true)
    val canHash = rindex < UInt(rounds) && 
	    (read > hashed || 
      (read === hashed && !buffer_valid) ||
      (read >= msg_len) )
      when(hashed > msg_len || (hashed === msg_len && rindex === UInt(rounds))){
        windex := UInt(0)
        state := s_write
      }.otherwise{
        state := s_idle
      }
    }
  }
  is(s_write){
    //we are writing 
    //request
    io.dmem_req_val := windex < UInt(hash_size_words)
    io.dmem_req_addr := hash_addr + (windex << UInt(3))
    io.dmem_req_tag := UInt(round_size_words) + windex
    io.dmem_req_cmd := M_XWR

    when(io.dmem_req_rdy){
      windex := windex + UInt(1)
    }

    //response
    when(io.dmem_resp_val){
      //there is a response from memory
      when(dmem_resp_tag_reg(4,0) >= UInt(round_size_words)) {
        //this is a response to a write
        writes_done(dmem_resp_tag_reg(4,0)-UInt(round_size_words)) := Bool(true)
      }
    }
    when(writes_done.reduce(_&&_)){
      //all the writes have been responded to
      //this is essentially reset time
      busy := Bool(false)

      writes_done := Vec.fill(hash_size_words){Bool(false)}
      windex := UInt(hash_size_words)
      rindex := UInt(rounds+1)
      buffer_count := UInt(0)
      msg_addr := UInt(0)
      hash_addr := UInt(0)
      msg_len := UInt(0)
      hashed := UInt(0)
      read := UInt(0)
      buffer_valid := Bool(false)
      io.init := Bool(true)
      state := s_idle
    }.otherwise{
      state := s_write
    }
  }
  }
}
