//`timescale 1 ns / 1 ps

module tsbuffer#(parameter W=2)(
	input en,
	input [W-1:0] in,
	output tri[W-1:0] out
);

	genvar i;
	generate 
	for (i = 0; i < W; i = i + 1) begin: for_genenrate
		bufif1 b(out[i],in[i],en);
	end
	endgenerate
endmodule


