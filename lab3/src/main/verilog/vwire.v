module vwire #(parameter W=2,  
	parameter L=4
)(
	input [L * W -1 :0] in,
	input [L -1 :0] en,
	output tri[W -1 :0] out

);
	
	//tri [W :0] bus; 

	genvar i;
	generate 
	for (i = 0; i < L; i = i + 1) begin: for_genenrate
		tsbuffer#(.W(2)) tsb(out, in[i * W + 1 :i * W], en[i]);
	end
	endgenerate

endmodule
