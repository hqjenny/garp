# create clock
create_clock clk -name clk -period ${CLOCK_PERIOD}
set_clock_uncertainty ${CLOCK_UNCERTAINTY} [get_clocks clk]

# set drive strength for inputs
#set_driving_cell -lib_cell C12T28SOI_LL_IVX4_P0 [all_inputs]
set_driving_cell -lib_cell INVX1_RVT [all_inputs]
# set load capacitance of outputs
set_load -pin_load 0.004 [all_outputs]

set all_inputs_but_clk [remove_from_collection [all_inputs] [get_ports clk]]
set_input_delay ${INPUT_DELAY} -clock [get_clocks clk] $all_inputs_but_clk
set_output_delay ${OUTPUT_DELAY} -clock [get_clocks clk] [all_outputs]

set_isolate_ports [all_outputs] -type buffer
set_isolate_ports [remove_from_collection [all_inputs] clk] -type buffer -force
 
# force no ungrouping the fake top level
#if {[current_design_name] == "FixNetwork2d"} {
if {[current_design_name] == "GarpAccel"} {
  set_ungroup [get_cells -hier network] false
}
