package garp

import Chisel._

//  Simple block which includes some salient features of garp blocks
class TileBlock extends Module {
    val io = new Bundle {
        // Configuration encoding signals
        val config = Bits(INPUT, width = 5)
        val config_en = Bool(INPUT)

        // Interconnect signals
        val hwire_read = Bool(INPUT)
        val hwire_write = Bool(OUTPUT)
        val vwire_read = Bool(INPUT)
        val vwire_write = Bool(OUTPUT)
    }
    // Keep track of configuration encoding
    val config_encoding = Reg(init = Bits(0, width = 5))
    when(io.config_en) {
        config_encoding := io.config
    }

    // Multiplex input signals
    val selected = Bool()
    switch (config_encoding(3,2)) {
        is (Bits(0, width = 2)) {
            selected := Bool(false)
        }
        is (Bits(1, width = 2)) {
            selected := Bool(true)
        }
        is (Bits(2, width = 2)) {
            selected := io.hwire_read
        }
        is (Bits(3, width = 2)) {
            selected := io.vwire_read
        }
    }

    // State element of simple block
    val f = Reg(init = Bool(false))
    when(config_encoding(4) === Bits(1, width = 1)) {
        f := selected
    }

    // Drive buses with tri-states
    // TODO: Use blackboxed tri-states instead of logic gates
    when (config_encoding(0) === Bits(1, width = 1)) {
        io.vwire_write := f
    }
    when (config_encoding(1) === Bits(1, width = 1)) {
        io.hwire_write := f
    }
}

// Sample array to connect and configure four blocks
class SampleArray extends Module {
    val io = new Bundle {
        // Configuration encoding signals
        val config = Bits(INPUT, width = 5)
        val config_index = Bits(INPUT, width = 2)
        val config_en = Bool(INPUT)

        // Interconnect signals
        val hwire_read = Bool(OUTPUT)
        val vwire_read = Bool(OUTPUT)
    }

    // Initialize interconnects
    val hwire = Bool(false)
    val vwire = Bool(false)
    io.hwire_read := hwire
    io.vwire_read := vwire

    // Instantiate blocks
    val block_0_0 = Module(new TileBlock()).io
    val block_0_1 = Module(new TileBlock()).io
    val block_1_0 = Module(new TileBlock()).io
    val block_1_1 = Module(new TileBlock()).io

    // Set up configuration network
    block_0_0.config := io.config
    block_0_1.config := io.config
    block_1_0.config := io.config
    block_1_1.config := io.config
    when (io.config_en) {
        switch (io.config_index) {
            is (Bits(0, width = 2)) {
                block_0_0.config_en := Bool(true)
            }
            is (Bits(1, width = 2)) {
                block_0_1.config_en := Bool(true)
            }
            is (Bits(2, width = 2)) {
                block_1_0.config_en := Bool(true)
            }
            is (Bits(3, width = 2)) {
                block_1_1.config_en := Bool(true)
            }
        }
    }

    // Connect blocks to buses
    // TODO: Drive buses from more than one block with tri-states
    hwire := block_0_0.hwire_write
    vwire := block_0_0.vwire_write
    block_0_0.hwire_read := hwire
    block_0_1.hwire_read := hwire
    block_1_0.hwire_read := hwire
    block_1_1.hwire_read := hwire
    block_0_0.vwire_read := vwire
    block_0_1.vwire_read := vwire
    block_1_0.vwire_read := vwire
    block_1_1.vwire_read := vwire
}