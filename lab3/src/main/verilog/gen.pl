#!/usr/bin/perl

my $x_in = 1;
my $i = undef;
for ($i = 0; $i < 16; $i++){
	printf("\t\t6'b%06b: X_in = V_wire_in[%d:%d];\n", $i+16, (15-$i)*2 +1,  (15-$i)*2);
}
for ($i = 0; $i < 11; $i++){
	printf("\t\t6'b%06b: X_in = H_wire_above_in[%d:%d];\n", $i+32, $i*2 +1,  ($i)*2);
}
for ($i = 0; $i < 4; $i++){
	printf("\t\t6'b%06b: X_in = G_wire_above_in[%d:%d];\n", $i+44, (3-$i)*2 +1,  (3-$i)*2);
}
for ($i = 0; $i < 11; $i++){
	printf("\t\t6'b%06b: X_in = H_wire_below_in[%d:%d];\n", $i+48, $i*2 +1,  ($i)*2);
}
for ($i = 0; $i < 4; $i++){
	printf("\t\t6'b%06b: X_in = G_wire_below_in[%d:%d];\n", $i+60, (3-$i)*2 +1,  (3-$i)*2);
}

