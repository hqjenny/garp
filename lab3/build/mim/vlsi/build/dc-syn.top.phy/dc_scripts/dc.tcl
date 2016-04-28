source -echo dc_setup.tcl

#################################################################################
# Setup for Formality verification
#################################################################################

set_svf ${RESULTS_DIR}/${DCRM_SVF_OUTPUT_FILE}

#################################################################################
# Read in the RTL Design
#################################################################################

define_design_lib WORK -path ./WORK
analyze -format verilog ${RTL_SOURCE_FILES}
elaborate ${DESIGN_NAME}
#analyze_datapath_extraction > ${REPORTS_DIR}/${DESIGN_NAME}.datapath.rpt
#remove_unconnected_ports [find -hier cell "*"]
#remove_unconnected_ports -blast_buses [find -hier cell "*"]

# Remove the RTL version of the hierarchical blocks in case they were read in

if { ${DDC_HIER_DESIGNS} != ""} {
  remove_design -hierarchy ${DDC_HIER_DESIGNS}
}

if { ${DC_ILM_HIER_DESIGNS} != ""} {
  remove_design -hierarchy ${DC_ILM_HIER_DESIGNS}
}

if { ${ICC_ILM_HIER_DESIGNS} != ""} {
  remove_design -hierarchy ${ICC_ILM_HIER_DESIGNS}
}

if { ${ICC_BLOCK_ABSTRACTION_DESIGNS} != ""} {
  remove_design -hierarchy ${ICC_BLOCK_ABSTRACTION_DESIGNS}
  set_top_implementation_options -block_references ${ICC_BLOCK_ABSTRACTION_DESIGNS}
}

# Store the elaborated design without the hierarchical physical blocks
write -hierarchy -format ddc -output ${RESULTS_DIR}/${DCRM_ELABORATED_DESIGN_DDC_OUTPUT_FILE}


#################################################################################
# Load Hierarchical Designs
#################################################################################

# Read in compiled hierarchical blocks
# For topographical mode top-level synthesis all physical blocks are required to
# be compiled in topographical mode.

foreach design ${DDC_HIER_DESIGNS} {
  read_ddc ${design}.mapped.ddc
}

foreach design ${DC_ILM_HIER_DESIGNS} {
  read_ddc ${design}.mapped.ILM.ddc
}

current_design ${DESIGN_NAME}
link

# Report the block abstraction settings and usage
if { ${ICC_BLOCK_ABSTRACTION_DESIGNS} != ""} {
  report_top_implementation_options
  report_block_abstraction
}

echo "listing block_abstraction cells:"
get_cells -hierarchical -filter is_block_abstraction==true

if { ${DDC_HIER_DESIGNS} != ""} {
  if {[shell_is_in_topographical_mode]} {
    # Hierarchical .ddc blocks must be marked as physical hierarchy
    # In case of multiply instantiated designs, only set_physical_hierarchy on ONE instance
    set_physical_hierarchy [sub_instances_of -hierarchy -master_instance -of_references ${DDC_HIER_DESIGNS} ${DESIGN_NAME}]
    echo "get_physical_hierarchy:"
    get_physical_hierarchy
  } else {
    # Don't touch these blocks in DC-WLM
    set_dont_touch [get_designs ${DDC_HIER_DESIGNS}]
  }
}

set_boundary_optimization ${HIERARCHICAL_DESIGNS} false
set_app_var compile_preserve_subdesign_interfaces true

#################################################################################
# Apply Logical Design Constraints
#################################################################################

source -echo ${DCRM_CONSTRAINTS_INPUT_FILE}

#################################################################################
# Create Default Path Groups
#
# Separating these paths can help improve optimization.
# Remove these path group settings if user path groups have already been defined.
#################################################################################

set ports_clock_root [filter_collection [get_attribute [get_clocks] sources] object_class==port]
group_path -name REGOUT -to [all_outputs]
group_path -name REGIN -from [remove_from_collection [all_inputs] $ports_clock_root]
group_path -name FEEDTHROUGH -from [remove_from_collection [all_inputs] $ports_clock_root] -to [all_outputs]

# multi-vt flow
set_leakage_optimization true

##################################################################################
# Specify ignored layers for routing to improve correlation
# Use the same ignored layers that will be used during place and route
##################################################################################
if { ${MIN_ROUTING_LAYER} != ""} {
  set_ignored_layers -min_routing_layer ${MIN_ROUTING_LAYER}
}
if { ${MAX_ROUTING_LAYER} != ""} {
  set_ignored_layers -max_routing_layer ${MAX_ROUTING_LAYER}
}
#report_ignored_layers

#################################################################################
# Apply Additional Optimization Constraints
#################################################################################
# Prevent assignment statements in the Verilog netlist.
set_fix_multiple_port_nets -all -buffer_constants

# read in floorplan info
#if { ${DCRM_DCT_FLOORPLAN_INPUT_FILE} != ""} {
#  read_floorplan $DCRM_DCT_FLOORPLAN_INPUT_FILE
#}
if { ${DCRM_DCT_DEF_INPUT_FILE} != ""} {
  extract_physical_constraints $DCRM_DCT_DEF_INPUT_FILE
}

#################################################################################
# Compile the Design
#################################################################################

check_design

compile_ultra -gate_clock -timing_high_effort_script 

#################################################################################
# Write Out Final Design and Reports
#
#        .ddc:   Recommended binary format used for subsequent Design Compiler sessions
#        .v  :   Verilog netlist for ASCII flow (Formality, PrimeTime, VCS)
#        .sdf:   SDF backannotated topographical mode timing for PrimeTime
#        .sdc:   SDC constraints for ASCII flow
#
#################################################################################

change_names -rules verilog -hierarchy

if {![shell_is_in_topographical_mode]} {
  if { ${DDC_HIER_DESIGNS} != "" } {
    remove_design -hierarchy ${DDC_HIER_DESIGNS}
  }
}

set_svf -off
write -format ddc -hierarchy -output ${RESULTS_DIR}/${DCRM_FINAL_DDC_OUTPUT_FILE}
write -f verilog -hierarchy -output ${RESULTS_DIR}/${DCRM_FINAL_VERILOG_OUTPUT_FILE}
write_sdc -nosplit ${RESULTS_DIR}/${DCRM_FINAL_SDC_OUTPUT_FILE}

source find_regs.tcl
find_regs ${STRIP_PATH}

#################################################################################
# Generate Final Reports
#################################################################################

report_qor > ${REPORTS_DIR}/${DCRM_FINAL_QOR_REPORT}
report_timing -input_pins -capacitance -transition_time -nets -significant_digits 4 -nosplit -nworst 10 -max_paths 10 > ${REPORTS_DIR}/${DESIGN_NAME}.mapped.timing.rpt
report_area -hierarchy -physical -nosplit > ${REPORTS_DIR}/${DCRM_FINAL_AREA_REPORT}
report_area -hierarchy -nosplit > ${REPORTS_DIR}/${DCRM_FINAL_AREA_REPORT}
report_power -nosplit -hier > ${REPORTS_DIR}/${DCRM_FINAL_POWER_REPORT}
report_clock_gating -nosplit > ${REPORTS_DIR}/${DCRM_FINAL_CLOCK_GATING_REPORT}
report_reference -nosplit -hierarchy > ${REPORTS_DIR}/${DESIGN_NAME}.mapped.reference.rpt
report_resources -nosplit -hierarchy > ${REPORTS_DIR}/${DESIGN_NAME}.mapped.resources.rpt

exit
