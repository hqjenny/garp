
//`timescale 1 ns / 1 ps

module tsbuffer(
	input en,
	input in,
	output out
);

assign b = (en) ? in : 1'bz;

endmodule


