module test;
  reg[1:0] io_mem_bus_in_23 = 0;
  reg[1:0] io_mem_bus_in_22 = 0;
  reg[1:0] io_mem_bus_in_21 = 0;
  reg[1:0] io_mem_bus_in_20 = 0;
  reg[1:0] io_mem_bus_in_19 = 0;
  reg[1:0] io_mem_bus_in_18 = 0;
  reg[1:0] io_mem_bus_in_17 = 0;
  reg[1:0] io_mem_bus_in_16 = 0;
  reg[1:0] io_mem_bus_in_15 = 0;
  reg[1:0] io_mem_bus_in_14 = 0;
  reg[1:0] io_mem_bus_in_13 = 0;
  reg[1:0] io_mem_bus_in_12 = 0;
  reg[1:0] io_mem_bus_in_11 = 0;
  reg[1:0] io_mem_bus_in_10 = 0;
  reg[1:0] io_mem_bus_in_9 = 0;
  reg[1:0] io_mem_bus_in_8 = 0;
  reg[1:0] io_mem_bus_in_7 = 0;
  reg[1:0] io_mem_bus_in_6 = 0;
  reg[1:0] io_mem_bus_in_5 = 0;
  reg[1:0] io_mem_bus_in_4 = 0;
  reg[1:0] io_mem_bus_in_3 = 0;
  reg[1:0] io_mem_bus_in_2 = 0;
  reg[1:0] io_mem_bus_in_1 = 0;
  reg[1:0] io_mem_bus_in_0 = 0;
  reg[4:0] io_addr = 0;
  wire[1:0] io_mem_bus_out_23;
  wire[1:0] io_mem_bus_out_22;
  wire[1:0] io_mem_bus_out_21;
  wire[1:0] io_mem_bus_out_20;
  wire[1:0] io_mem_bus_out_19;
  wire[1:0] io_mem_bus_out_18;
  wire[1:0] io_mem_bus_out_17;
  wire[1:0] io_mem_bus_out_16;
  wire[1:0] io_mem_bus_out_15;
  wire[1:0] io_mem_bus_out_14;
  wire[1:0] io_mem_bus_out_13;
  wire[1:0] io_mem_bus_out_12;
  wire[1:0] io_mem_bus_out_11;
  wire[1:0] io_mem_bus_out_10;
  wire[1:0] io_mem_bus_out_9;
  wire[1:0] io_mem_bus_out_8;
  wire[1:0] io_mem_bus_out_7;
  wire[1:0] io_mem_bus_out_6;
  wire[1:0] io_mem_bus_out_5;
  wire[1:0] io_mem_bus_out_4;
  wire[1:0] io_mem_bus_out_3;
  wire[1:0] io_mem_bus_out_2;
  wire[1:0] io_mem_bus_out_1;
  wire[1:0] io_mem_bus_out_0;
  reg clk = 0;
  reg reset = 1;
  integer clk_len;
  always #clk_len clk = ~clk;
  reg vcdon = 0;
  reg [1023:0] vcdfile = 0;
  reg [1023:0] vpdfile = 0;

  /*** DUT instantiation ***/
  GarpAccel GarpAccel(
    .clk(clk),
    .reset(reset),
    .io_mem_bus_in_23(io_mem_bus_in_23),
    .io_mem_bus_in_22(io_mem_bus_in_22),
    .io_mem_bus_in_21(io_mem_bus_in_21),
    .io_mem_bus_in_20(io_mem_bus_in_20),
    .io_mem_bus_in_19(io_mem_bus_in_19),
    .io_mem_bus_in_18(io_mem_bus_in_18),
    .io_mem_bus_in_17(io_mem_bus_in_17),
    .io_mem_bus_in_16(io_mem_bus_in_16),
    .io_mem_bus_in_15(io_mem_bus_in_15),
    .io_mem_bus_in_14(io_mem_bus_in_14),
    .io_mem_bus_in_13(io_mem_bus_in_13),
    .io_mem_bus_in_12(io_mem_bus_in_12),
    .io_mem_bus_in_11(io_mem_bus_in_11),
    .io_mem_bus_in_10(io_mem_bus_in_10),
    .io_mem_bus_in_9(io_mem_bus_in_9),
    .io_mem_bus_in_8(io_mem_bus_in_8),
    .io_mem_bus_in_7(io_mem_bus_in_7),
    .io_mem_bus_in_6(io_mem_bus_in_6),
    .io_mem_bus_in_5(io_mem_bus_in_5),
    .io_mem_bus_in_4(io_mem_bus_in_4),
    .io_mem_bus_in_3(io_mem_bus_in_3),
    .io_mem_bus_in_2(io_mem_bus_in_2),
    .io_mem_bus_in_1(io_mem_bus_in_1),
    .io_mem_bus_in_0(io_mem_bus_in_0),
    .io_addr(io_addr),
    .io_mem_bus_out_23(io_mem_bus_out_23),
    .io_mem_bus_out_22(io_mem_bus_out_22),
    .io_mem_bus_out_21(io_mem_bus_out_21),
    .io_mem_bus_out_20(io_mem_bus_out_20),
    .io_mem_bus_out_19(io_mem_bus_out_19),
    .io_mem_bus_out_18(io_mem_bus_out_18),
    .io_mem_bus_out_17(io_mem_bus_out_17),
    .io_mem_bus_out_16(io_mem_bus_out_16),
    .io_mem_bus_out_15(io_mem_bus_out_15),
    .io_mem_bus_out_14(io_mem_bus_out_14),
    .io_mem_bus_out_13(io_mem_bus_out_13),
    .io_mem_bus_out_12(io_mem_bus_out_12),
    .io_mem_bus_out_11(io_mem_bus_out_11),
    .io_mem_bus_out_10(io_mem_bus_out_10),
    .io_mem_bus_out_9(io_mem_bus_out_9),
    .io_mem_bus_out_8(io_mem_bus_out_8),
    .io_mem_bus_out_7(io_mem_bus_out_7),
    .io_mem_bus_out_6(io_mem_bus_out_6),
    .io_mem_bus_out_5(io_mem_bus_out_5),
    .io_mem_bus_out_4(io_mem_bus_out_4),
    .io_mem_bus_out_3(io_mem_bus_out_3),
    .io_mem_bus_out_2(io_mem_bus_out_2),
    .io_mem_bus_out_1(io_mem_bus_out_1),
    .io_mem_bus_out_0(io_mem_bus_out_0));

  initial begin
    clk_len = `CLOCK_PERIOD;
    $init_clks(clk_len);
    $init_rsts(reset);
    $init_ins(io_mem_bus_in_23, io_mem_bus_in_22, io_mem_bus_in_21, io_mem_bus_in_20, io_mem_bus_in_19, io_mem_bus_in_18, io_mem_bus_in_17, io_mem_bus_in_16, io_mem_bus_in_15, io_mem_bus_in_14, io_mem_bus_in_13, io_mem_bus_in_12, io_mem_bus_in_11, io_mem_bus_in_10, io_mem_bus_in_9, io_mem_bus_in_8, io_mem_bus_in_7, io_mem_bus_in_6, io_mem_bus_in_5, io_mem_bus_in_4, io_mem_bus_in_3, io_mem_bus_in_2, io_mem_bus_in_1, io_mem_bus_in_0, io_addr);
    $init_outs(io_mem_bus_out_23, io_mem_bus_out_22, io_mem_bus_out_21, io_mem_bus_out_20, io_mem_bus_out_19, io_mem_bus_out_18, io_mem_bus_out_17, io_mem_bus_out_16, io_mem_bus_out_15, io_mem_bus_out_14, io_mem_bus_out_13, io_mem_bus_out_12, io_mem_bus_out_11, io_mem_bus_out_10, io_mem_bus_out_9, io_mem_bus_out_8, io_mem_bus_out_7, io_mem_bus_out_6, io_mem_bus_out_5, io_mem_bus_out_4, io_mem_bus_out_3, io_mem_bus_out_2, io_mem_bus_out_1, io_mem_bus_out_0);
    $init_sigs(GarpAccel);
    /*** VCD & VPD dump ***/
    if ($value$plusargs("vcdfile=%s", vcdfile)) begin
      $dumpfile(vcdfile);
      $dumpvars(0, GarpAccel);
      $dumpoff;
      vcdon = 0;
    end
    if ($value$plusargs("vpdfile=%s", vpdfile)) begin
      $vcdplusfile(vpdfile);
      $vcdpluson(0);
      $vcdplusautoflushon;
    end
    if ($test$plusargs("vpdmem")) begin
      $vcdplusmemon;
    end
  end

  always @(negedge clk) begin
    $tick();
    if (vcdfile && (reset)) begin
      $dumpoff;
      vcdon = 0;
    end
    else if (vcdfile && !vcdon) begin
      $dumpon;
      vcdon = 1;
    end
  end

endmodule
