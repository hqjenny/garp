//`timescale 1 ns / 1 ps
module input_mux #(parameter W=2,  
	parameter V=16, 
	parameter H=11, 
	parameter G=4
)(
	input [V * W -1 :0] V_wire_in,
	input [H * W -1 :0] H_wire_above_in, 
	input [H * W -1 :0] H_wire_below_in, 

	input [G * W -1 :0] G_wire_above_in, 
	input [G * W -1 :0] G_wire_below_in, 
	input [ 4 * 6 -1 :0] config_X_in,
	input [W -1 :0] Z_reg,
	input [W -1 :0] D_reg,
	output reg [ 4 * W -1 :0] X_in 
	
);
	genvar i;
	generate 
	for (i = 0; i < 4; i = i + 1) begin: for_genenrate
	always@(*) begin
	case(config_X_in[(i * 6 + 5) : i * 6])
		6'b000000: X_in[(i * 2 + 1):(i * 2)] = 2'b00;		
		6'b000001: X_in[(i * 2 + 1):(i * 2)] = 2'b10;	
		6'b000010: X_in[(i * 2 + 1):(i * 2)] = Z_reg;
		6'b000011: X_in[(i * 2 + 1):(i * 2)] = D_reg;

		6'b010000: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[31:30];
                6'b010001: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[29:28];
                6'b010010: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[27:26];
                6'b010011: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[25:24];
                6'b010100: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[23:22];
                6'b010101: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[21:20];
                6'b010110: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[19:18];
                6'b010111: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[17:16];
                6'b011000: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[15:14];
                6'b011001: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[13:12];
                6'b011010: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[11:10];
                6'b011011: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[9:8];
                6'b011100: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[7:6];
                6'b011101: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[5:4];
                6'b011110: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[3:2];
                6'b011111: X_in[(i * 2 + 1):(i * 2)] = V_wire_in[1:0];

                6'b100000: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[1:0];
                6'b100001: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[3:2];
                6'b100010: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[5:4];
                6'b100011: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[7:6];
                6'b100100: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[9:8];
                6'b100101: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[11:10];
                6'b100110: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[13:12];
                6'b100111: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[15:14];
                6'b101000: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[17:16];
                6'b101001: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[19:18];
                6'b101010: X_in[(i * 2 + 1):(i * 2)] = H_wire_above_in[21:20];

                6'b101100: X_in[(i * 2 + 1):(i * 2)] = G_wire_above_in[7:6];
                6'b101101: X_in[(i * 2 + 1):(i * 2)] = G_wire_above_in[5:4];
                6'b101110: X_in[(i * 2 + 1):(i * 2)] = G_wire_above_in[3:2];
                6'b101111: X_in[(i * 2 + 1):(i * 2)] = G_wire_above_in[1:0];

                6'b110000: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[1:0];
                6'b110001: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[3:2];
                6'b110010: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[5:4];
                6'b110011: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[7:6];
                6'b110100: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[9:8];
                6'b110101: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[11:10];
                6'b110110: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[13:12];
                6'b110111: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[15:14];
                6'b111000: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[17:16];
                6'b111001: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[19:18];
                6'b111010: X_in[(i * 2 + 1):(i * 2)] = H_wire_below_in[21:20];

                6'b111100: X_in[(i * 2 + 1):(i * 2)] = G_wire_below_in[7:6];
                6'b111101: X_in[(i * 2 + 1):(i * 2)] = G_wire_below_in[5:4];
                6'b111110: X_in[(i * 2 + 1):(i * 2)] = G_wire_below_in[3:2];
                6'b111111: X_in[(i * 2 + 1):(i * 2)] = G_wire_below_in[1:0];
		
		default: X_in[(i * 2 + 1):(i * 2)] = 2'bxx;
	endcase
	end
	end
	endgenerate
endmodule


