
//`timescale 1 ns / 1 ps

module tsbuffer(
	input en,
	input in,
	output out
);
bufif1 (out,in,en);
endmodule


