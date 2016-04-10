def identify_logic_input(bits):
	if bits == "000000":
		return "00 (binary)"
	elif bits == "000001":
		return "01 (binary)"
	elif bits == "000010":
		return "internal Z register"
	elif bits == "000011":
		return "internal D register"
	elif bits[0:2] == "01":
		return "V wire pair " + str(15 - int(bits[2:6], 2))
	elif bits[0:2] == "10":
		index = int(bits[2:6], 2)
		if index <= 10:
			return str(index) + " from leftmost and " + str(10 - index) + " from rightmost H wire pair above"
		elif index == 11:
			return "an invalid encoding"
		else:
			return "G wire pair " + str(15 - index) + " above"
	elif bits[0:2] == "11":
		index = int(bits[2:6], 2)
		if index <= 10:
			return str(index) + " from leftmost and " + str(10 - index) + " from rightmost H wire pair below"
		elif index == 11:
			return "an invalid encoding"
		else:
			return "G wire pair " + str(15 - index) + " below"
	else:
		return "an invalid encoding"

def identify_logic_prime_crossbar(prime, bits):
	return prime + bits[0] + prime + bits[1]

def identify_logic_prime_shift(prime, bits):
	if bits == "00":
		return prime + "1" + prime + "0"
	elif bits == "01":
		return "!(" + prime + "1" + prime + "0)"
	elif btis == "10":
		return prime + "0" + prime + "-1"
	elif bits == "11":
		return "!(" + prime + "0" + prime + "-1)"
	else:
		return "an invalid encoding"

def identify_logic_result(mx):
	if mx == "00":
		return "Z = V"
	elif mx == "01":
		return "Z = carry out"
	elif mx == "10":
		return "Z = U ^ K"
	elif mx == "11":
		return "Z = !(U ^ K)"
	else:
		"an invalid encoding"

def identify_logic_mode_and_mx(mode, mx):
	if mode == "000":
		return ("ta", "D'")
	elif mode == "001":
		if mx == "01":
			return ("sp", "01")
		else:
			return ("iv", "an invalid encoding")
	elif mode[0:2] == "01":
		if mx == "00":
			return ("se", "00")
		elif mx == "01":
			return ("ps", "01")
		else:
			return ("iv", "an invalid encoding")
	elif mode[0:2] == "10":
		return ("cc", "result")
	elif mode[0:2] == "11":
		return ("tr", "result")
	else:
		return ("iv", "an invalid encoding")

def print_logic_block(encoding):
	a_in = encoding[0:6]
	a_prime = encoding[6:8]
	b_in = encoding[8:14]
	b_prime = encoding[14:16]
	c_in = encoding[16:22]
	c_prime = encoding[22:24]
	d_in = encoding[24:30]
	mx = encoding[30:32]
	lookup_table = encoding[32:48]
	mode = encoding[48:51]
	Z = encoding[51]
	D = encoding[52]
	H = encoding[53]
	G = encoding[54]
	V = encoding[55]
	G_out = encoding[56:59]
	V_out = encoding[59:64]
	
	which_mode, which_mx = identify_logic_mode_and_mx(mode, mx)
	mode_string = ""
	suppress_string = ""
	if which_mode == "ta":
		mode_string = "table mode"
	elif which_mode == "sp":
		mode_string = "split table mode"
	elif which_mode == "se":
		mode_string = "select mode"
		suppress_string = " (k = " + mode[2] + " " + ("allows" if mode[2] == "1" else "suppresses") + " shifts in)"
	elif which_mode == "ps":
		mode_string = "partial mode"
		suppress_string = " (k = " + mode[2] + " " + ("allows" if mode[2] == "1" else "suppresses") + " shifts in)"
	elif which_mode == "cc":
		mode_string = "carry chain mode"
		suppress_string = " (k = " + mode[2] + " " + ("allows" if mode[2] == "1" else "suppresses") + " carry in)"
	elif which_mode == "tr":
		mode_string = "triple add mode"
		suppress_string = " (k = " + mode[2] + " " + ("allows" if mode[2] == "1" else "suppresses") + " shifts, carries in)"
	else:
		mode_string ="an invalid mode"
	print("The logic block is in " + mode_string)

	# Upper fields
	use_crossbar = (which_mode == "ta") or (which_mode == "sp") or (which_mode == "cc")
	print("[63..58] A in {" + a_in + "} = " + identify_logic_input(a_in))
	print("[57..56] A' {" + a_prime + "} = " + (identify_logic_prime_crossbar("A", a_prime) if use_crossbar else identify_logic_prime_shift("A", a_prime)))
	print("[55..50] B in {" + b_in + "} = " + identify_logic_input(b_in))
	print("[49..48] B' {" + b_prime + "} = " + (identify_logic_prime_crossbar("B", b_prime) if use_crossbar else identify_logic_prime_shift("B", b_prime)))
	print("[47..42] C in {" + c_in + "} = " + identify_logic_input(c_in))
	print("[41..40] C' {" + c_prime + "} = " + (identify_logic_prime_crossbar("C", c_prime) if use_crossbar else identify_logic_prime_shift("C", c_prime)))
	print("[39..34] D in {" + d_in + "} = " + identify_logic_input(d_in))
	mx_string = ""
	if which_mx == "D'":
		mx_string = " = " + identify_logic_prime_crossbar("D", mx)
	elif which_mx == "result":
		mx_string = " => " + identify_logic_result(mx)
	print("[33..32] mx {" + mx + "} = " + which_mx + mx_string)

	# Lower fields
	print("[31..16] lookup table(s) {" + lookup_table + "}")
	print("[15..13] mode {" + mode + "} = " + mode_string + suppress_string)
	print("[12] Z {" + Z + "} = " + ("suppress latching of Z register; output Z directly" if Z == "0" else "latch Z register every cycle; output Z register"))
	print("[11] D {" + D + "} = " + ("suppress latching of D register; output D directly" if D == "0" else "latch D register every cycle; output D register"))
	print("[10] H {" + H + "} => Hout = " + ("Z" if H == "0" else "D"))
	print("[9] G {" + G + "} => Gout = " + ("Z" if G == "0" else "D"))
	print("[8] V {" + V + "} => Vout = " + ("Z" if V == "0" else "D"))
	G_out_string = "[7..5] G out {" + G_out + "} = "
	G_out_index = int(G_out, 2)
	if (G_out_index == 0):
		print(G_out_string + "no output to G wires below")
	elif (G_out_index >= 4):
		print(G_out_string + "output Gout to G wire pair " + str(7 - G_out_index) + " below")
	else:
		print(G_out_string + "an invalid encoding")
	V_out_string = "[4..0] V out {" + V_out + "} = "
	V_out_index = int(V_out, 2)
	if (V_out_index == 0):
		print(V_out_string + "no output to V wires below")
	elif (V_out_index >= 16):
		print(V_out_string + "output Vout to V wire pair " + str(31 - V_out_index) + " below")
	else:
		print(V_out_string + "an invalid encoding")



while(True):
	uce = raw_input("Upper configuration encoding:")
	uce_bits = bin(int(uce, 16))[2:]
	# Pad upper zero bits that were dropped with int()
	uce_bits = ("0" * (32 - len(uce_bits))) + uce_bits
	lce = raw_input("Lower configuration encoding:")
	lce_bits = bin(int(lce, 16))[2:]
	# Pad upper zero bits that were dropped with int()
	lce_bits = ("0" * (32 - len(lce_bits))) + lce_bits
	encoding = uce_bits + lce_bits
	print_logic_block(encoding)
