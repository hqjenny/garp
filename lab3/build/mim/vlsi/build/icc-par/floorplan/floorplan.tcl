#floorplan

# [stevo]: done in init_design_icc.tcl
#derive_pg_connection -power_net vdd -power_pin vdd -create_port top
#derive_pg_connection -ground_net gnd -ground_pin gnd -create_port top

# [stevo]: set unit tile name if different from default "unit"
set_fp_strategy -unit_tile_name unit12T

set_preferred_routing_direction   -layers {M2 M4 M6 B2 IB} -direction horizontal
set_preferred_routing_direction   -layers {M1 M3 M5 B1 IA} -direction vertical

set_pin_physical_constraints -layers {M4} -side 1 -offset 50 -pin_spacing 5 [get_ports]
set_fp_pin_constraints \
  -block_level \
  -use_physical_constraints on \
  -corner_keepout_percent_side 15 \
  -keep_buses_together on

create_floorplan \
  -control_type aspect_ratio \
  -core_utilization 0.7 \
  -flip_first_row \
  -start_first_row \
  -left_io2core 7 \
  -bottom_io2core 7 \
  -right_io2core 7 \
  -top_io2core 7 

# there's something funky about the synopsys 32nm educational library
# which results in create_floorplan leaving gaps between the standard cell rows it creates
# removing and regenerating the standard cell rows seems to get around the propblem
#cut_row -all
#add_row \
#  -within [get_attr [get_core_area] bbox] \
#  -direction horizontal \
#  -flip_first_row \
#  -tile_name unit12T

set_fp_placement_strategy -macros_on_edge auto  
set_fp_macro_options [all_macro_cells ] -legal_orientation {FN FS}
set_keepout_margin -type soft -north -outer {2 2 2 2} [all_macro_cells]
#set_fp_macro_array -name vrf -elements [list [get_cells "Tile/core_vu/vxu/b8lane/vuVXU_Banked8_Bank_rfile_rfile_sram Tile/core_vu/vxu/b8lane/vuVXU_Banked8_Bank_1/rfile_rfile_sram Tile/core_vu/vxu/b8lane/vuVXU_Banked8_Bank_2/rfile_rfile_sram Tile/core_vu/vxu/b8lane/vuVXU_Banked8_Bank_3/rfile_rfile_sram"] [get_cells "Tile/core_vu/vxu/b8lane/vuVXU_Banked8_Bank_4/rfile_rfile_sram Tile/core_vu/vxu/b8lane/vuVXU_Banked8_Bank_5/rfile_rfile_sram Tile/core_vu/vxu/b8lane/vuVXU_Banked8_Bank_6/rfile_rfile_sram Tile/core_vu/vxu/b8lane/vuVXU_Banked8_Bank_7/rfile_rfile_sram"]] -x_offset 10 -y_offset 10 -align_2d left-bottom -rectilinear
#set_fp_macro_array -name dcache -elements [list [get_cells "Tile/dcache/data_T12_sram Tile/dcache/data_T40_sram Tile/dcache/data_T85_sram Tile/dcache/data_T112_sram Tile/dcache/meta_tags_sram"]] -x_offset 10 -y_offset 10 -align_edge top -rectilinear
#set_fp_macro_array -name icache -elements [list [get_cells "Tile/icache/icache/T208_sram Tile/icache/icache/T248_sram Tile/icache/icache/tag_array_sram"]] -x_offset 10 -y_offset 10 -align_edge bottom -rectilinear
#set_fp_macro_array -name llc -elements [list [get_cells "uncore_outmemsys_llc/data_data_T27_sram uncore_outmemsys_llc/data_data_T52_sram uncore_outmemsys_llc/data_data_T94_sram uncore_outmemsys_llc/data_data_T119_sram uncore_outmemsys_llc/tags_T7_sram uncore_outmemsys_llc/data_data_T162_sram uncore_outmemsys_llc/data_data_T187_sram uncore_outmemsys_llc/data_data_T229_sram uncore_outmemsys_llc/data_data_T254_sram"]] -x_offset 10 -y_offset 10 -align_edge top -rectilinear
##set_fp_macro_array -name llc_1 -elements [list [get_cells "uncore_outmemsys_llc/data_data_T162_sram uncore_outmemsys_llc/data_data_T187_sram uncore_outmemsys_llc/data_data_T229_sram uncore_outmemsys_llc/data_data_T254_sram"]] -x_offset 10 -y_offset 10 -align_edge top -rectilinear
#set_fp_macro_array -name vu_icache -elements [list [get_cells "Tile/Frontend/icache_tag_array_sram Tile/Frontend/icache_T152_sram"]] -x_offset 10 -y_offset 10 -align_edge top -rectilinear
#set_fp_macro_array -name vrf -elements [list [get_cells "Tile/core/vu/vxu/b8lane/vuVXU_Banked8_Bank_1/rfile/rfile/sram Tile/core/vu/vxu/b8lane/vuVXU_Banked8_Bank_2/rfile/rfile/sram Tile/core/vu/vxu/b8lane/vuVXU_Banked8_Bank_3/rfile/rfile/sram Tile/core/vu/vxu/b8lane/vuVXU_Banked8_Bank_4/rfile/rfile/sram"] [get_cells "Tile/core/vu/vxu/b8lane/vuVXU_Banked8_Bank_5/rfile/rfile/sram Tile/core/vu/vxu/b8lane/vuVXU_Banked8_Bank_6/rfile/rfile/sram Tile/core/vu/vxu/b8lane/vuVXU_Banked8_Bank_7/rfile/rfile/sram Tile/core/vu/vxu/b8lane/vuVXU_Banked8_Bank/rfile/rfile/sram"]] -x_offset 10 -y_offset 10 -align_2d left-bottom -rectilinear
#set_fp_macro_array -name dcache -elements [list [get_cells "Tile/dcache/data/T12/sram Tile/dcache/data/T40/sram Tile/dcache/data/T85/sram Tile/dcache/data/T112/sram Tile/dcache/meta/tags/sram"]] -x_offset 10 -y_offset 10 -align_edge top -rectilinear
#set_fp_macro_array -name icache -elements [list [get_cells "Tile/icache/icache/T208/sram Tile/icache/icache/T248/sram Tile/icache/icache/tag_array/sram"]] -x_offset 10 -y_offset 10 -align_edge bottom -rectilinear
#set_fp_macro_array -name llc -elements [list [get_cells "uncore/outmemsys/llc/data/data/T27/sram uncore/outmemsys/llc/data/data/T52/sram uncore/outmemsys/llc/data/data/T94/sram uncore/outmemsys/llc/data/data/T119/sram uncore/outmemsys/llc/tags/T7/sram uncore/outmemsys/llc/data/data/T162/sram uncore/outmemsys/llc/data/data/T187/sram uncore/outmemsys/llc/data/data/T229/sram uncore/outmemsys/llc/data/data/T254/sram"]] -x_offset 10 -y_offset 10 -align_edge top -rectilinear
#set_fp_macro_array -name vu_icache -elements [list [get_cells "Tile/Frontend/icache/tag_array/sram Tile/Frontend/icache/T152/sram"]] -x_offset 10 -y_offset 10 -align_edge top -rectilinear

# create initial placement
#create_fp_placement -effort high -timing_driven
create_fp_placement -effort high -timing_driven -optimize_pins

# create power distribution network
# probably need to redo this later, as rings will all be in IA like raven?
synthesize_fp_rings \
  -nets [list $MW_POWER_NET $MW_GROUND_NET] \
  -core \
  -layers {IA IB} \
  -width {2 2} \
  -space {1 1} \
  -offset {1 1}

set_power_plan_strategy core \
  -nets [list $MW_POWER_NET $MW_GROUND_NET] \
  -core \
  -template st28.tpl:raven_mesh \
  -extension {stop: outermost_ring}

#set macros [collection_to_list -no_braces -name_only [get_cells -hierarchical -filter "mask_layout_type==macro"]]
#set_power_plan_strategy macros \
  -nets {VDD VSS} \
  -macros $macros \

compile_power_plan

# add filler cells so all cell sites are populated
# synopsys recommends you do this before routing standard cell power rails
insert_stdcell_filler \
        -cell_without_metal $FILLER_CELL \
        -connect_to_power [list $MW_POWER_NET] \
        -connect_to_ground [list $MW_GROUND_NET]

# preroute standard cell rails
preroute_standard_cells \
  -within [get_attribute [get_core_area] bbox] \
  -connect horizontal \
  -port_filter_mode off \
  -cell_master_filter_mode off \
  -cell_instance_filter_mode off \
  -voltage_area_filter_mode off \
  -do_not_route_over_macros \
  -no_routing_outside_working_area

# get rid of filler cells
remove_stdcell_filler -stdcell

# verify connectivity of power/ground nets
verify_pg_nets
