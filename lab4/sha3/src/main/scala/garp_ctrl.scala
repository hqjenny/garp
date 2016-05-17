package garp

import Chisel._
import uncore._
import rocket._
import cde.{Parameters, Field}
import cde.Implicits._


class CtrlModule(val W: Int=2, val V: Int=16, val H: Int=11, val G: Int=4, val R: Int=32)(implicit p: Parameters) extends Module {
  val io = new bundle {
    //RoCC signals
    val cmd = Decoupled(new RoCCCommand).flip
    val resp = Decoupled(new RoCCResponse)
    val busy = Bool(OUTPUT)
    val interrupt = Bool(OUTPUT)

    //Memory signals
    val mem = new HellaCacheIO()(p.alterPartial({ case CacheName => "L1D" }))

    //Garp signals
    val config_val = Bool(OUTPUT)
    val config_row = UInt(OUTPUT, log2Up(R))
    val config_col = UInt(OUTPUT, log2Up(24))

    val clock_cur = UInt(INPUT, 32)
    val clock_new = UInt(OUTPUT, 32)
    val clok_val = Bool(OUTPUT)
  }

  // The Rocc Handler is either idle, loading a configuration, moving values to or from registers, or bumping or stopping the clock
  val r_idle :: r_load_config :: r_move_to :: r_move_from :: r_bump :: r_stop :: Nil = Enum(UInt(), 6)
  val rocc_s = Reg(init=r_idle)

  // Memory is either idle, being read from, or waiting on buffered configurations to be written
  val m_idle :: m_read :: m_wait :: Nil = Enum(UInt(), 3)
  val mem_s = Reg(init=m_idle)

  // Store configuration encidings from memory to one buffer while the other one is being written to the array
  val mem_buffer = Vec.fill(2){Reg(init = Bits(0, 64))}
  val mem_buffer_val = Vec.fill(2){Reg{init = Bool(false)}}
  val mem_buffer_empty = !(mem_buffer_val(0) || mem_buffer_val(1))
  val mem_buffer_full = mem_buffer_val(0) && mem_buffer_val(1)

  // Store how many words are left to read for a configuration
  val words_left = Reg(init=UInt(0,16))


  // Make registers for RoCC output signals
  val cmd_rdy = Reg(init=Bool(true))

  val resp_bits = Reg(init=Bits(0, width=xLen))
  val resp_val = Reg(init=Bool(false))

  val busy = Reg(init=Bool(false))

  // Assign RoCC output signals to appropriate I/O
  io.cmd.ready := cmd_rdy

  io.resp.bits := resp_bits
  io.resp.valid := resp_val

  io.busy := busy

  // Current command register is valid when busy is true
  val command = Reg(init = new RoCCCommand()(p))

  // Make registers for memory output signals
  val mem_req_addr = Reg(init=Bits(0))
  val mem_req_tag = Reg(init=Bits(0))
  val mem_val = Reg(init=Bool(false))

  // Store whether there is an outbound memory request currently
  val cur_mem_req = Reg(init=Bool(false))

  when(io.mem.req.fire()) {
    cur_mem_req := Bool(true)
  }
  when (io.mem.resp.valid) {
    cur_mem_req := Bool(false)
  }


  // Assign memory output signals to appropriate I/O
  io.mem.req.valid := mem_val
  io.mem.req.bits.addr := mem_req_addr
  io.mem.req.bits.tag := mem_req_tag
  io.mem.req.bits.cmd := M_XRD
  io.mem.req.bits.typ := MT_D
  io.mem.req.bits.data := Bits(0)
  io.mem.invalidate_lr := Bool(false)

  io.interrupt := Bool(false)
  io.imem.acquire.valid := Bool(false)
  io.imem.grant.ready := Bool(false)
  io.dmem.head.acquire.valid := Bool(false)
  io.dmem.head.grant.ready := Bool(false)
  io.iptw.req.valid := Bool(false)
  io.dptw.req.valid := Bool(false)
  io.pptw.req.valid := Bool(false)


  // Garp output signal defualts
  io.config_val := Bool(false)
  io.config_row := Bool(false)
  io.config_col := Bool(false)

  io.clock_val := Bool(false)
  io.clock_new := UInt(0)


  // Ready to take in commands when the RoCC interface is idle
  cmd_ready := rocc_s === r_idle


  // State machine for the RoCC interface
  switch(rocc_s) {
    is(r_idle) {
      when(io.cmd.fire()){
        // Performed transaction to read RoCC command
        // Store it, become busy, and change state to handle command
        command := io.cmd.bits
        busy := Bool(true)
        val funct = io.cmd.bits.inst.funct
        when(funct === Bits(0)) {
          rocc_s := r_load_config
        } .elsewhen(funct(2,1) === Bits(2)) {
          rocc_s := r_move_to
        } .elsewhen(funct(2,1) === Bits(3)) {
          rocc_s := r_move_from
        } .elsewhen(funct === Bits(8)) {
          rocc_s := r_bump
        } .elsewhen(funct === Bits(9)) {
          rocc_s := r_stop
        }
      }
    }
    is(r_load_config) {
      when(mem_buffer_empty) {
        when(words_left === UInt(0)) {
          when(! io.mem.resp.val) {
            when (! cur_mem_req) {
              // Load first memory address to find out the size of the configuration encoding
              mem_req_addr := command.rs1
              mem_val := Bool(true)
            }
          } .otherwise {
            // Read the response and update the number of words left
            words_left := (io.mem.rest.data.toUInt()) * UInt(24)
          }
        } .otherwise {
          // Wait for new values to be fetched by the memory system
        }
      } .otherwise {
        // Process values

        when(words_left === UInt(0)) {
          // Transition to idle because there are no more values
        }
      }
    }
    is(r_move_to) {

    }
    is(r_move_from) {

    }
    is(r_bump) {

    }
    is(r_stop) {

    }
  }


  // State machine for the memory interface
  switch(mem_s) {
    is(m_idle) {
      when (words_left > UInt(0)) {
        when (mem_buffer_full) {
          mem_s := m_wait
        } .otherwise {
          mem_s := m_read
        }
      }
    }
    is(m_read) {
      
    }
    is(m_wait) {

    }
  }



}
