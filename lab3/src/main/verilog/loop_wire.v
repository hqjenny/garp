//`timescale 1 ns / 1 ps

module loop_wire(
	input[1:0] in,
	output[1:0] out
);

assign out = in;

endmodule

