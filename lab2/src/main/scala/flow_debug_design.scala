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
        val hwire_write_en = Bool(OUTPUT)
        val vwire_read = Bool(INPUT)
        val vwire_write = Bool(OUTPUT)
        val vwire_write_en = Bool(OUTPUT)
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

    // Drive interconnects with output of flipflop
    io.hwire_write := f
    io.hwire_write_en := config_encoding(1)
    io.vwire_write := f
    io.vwire_write_en := config_encoding(0)
}

class Interconnect extends Module {
    val io = new Bundle {
        val write = Vec.fill(4){Bool(INPUT)}
        val write_en = Vec.fill(4){Bool(INPUT)}
        val read = Vec.fill(4){Bool(OUTPUT)}
    }
    // TODO: Actually make interconnect with tri-states
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

    // Instantiate blocks
    val blocks = Vec.fill(4){Module(new TileBlock()).io}

    // Set up configuration network
    for (i <- 0 until 4) {
        blocks(i).config := io.config
        blocks(i).config_en := Bool(false)
    }
    when (io.config_en) {
        switch (io.config_index) {
            is (Bits(0, width = 2)) {
                blocks(0).config_en := Bool(true)
            }
            is (Bits(1, width = 2)) {
                blocks(1).config_en := Bool(true)
            }
            is (Bits(2, width = 2)) {
                blocks(2).config_en := Bool(true)
            }
            is (Bits(3, width = 2)) {
                blocks(3).config_en := Bool(true)
            }
        }
    }

    // Initialize interconnects
    val hwire = Module(new Interconnect()).io
    val vwire = Module(new Interconnect()).io
    io.hwire_read := hwire.read
    io.vwire_read := vwire.read

    // Connect blocks to interconnects
    for (i <- 0 until 4) {
        blocks(i).hwire_read := hwire.read
        blocks(i).hwire_write := hwire.write
        blocks(i).hwire_write_en := hwire.write_en
        blocks(i).vwire_read := vwire.read
        blocks(i).vwire_write := vwire.write
        blocks(i).vwire_write_en := vwire.write_en
    }
}