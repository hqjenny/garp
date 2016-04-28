#floorplan

# [stevo]: grab dimensions
set TILE_WIDTH $env(TILE_WIDTH)
set TILE_HEIGHT $env(TILE_HEIGHT)
set TILE_GAP $env(TILE_GAP)

# [stevo]: get rows and columns from environment
set ROWS $env(ROWS)
set COLS $env(COLS)

# [stevo]: calculate core dimensions
set CORE_WIDTH [expr ($COLS+1)*$TILE_GAP+$COLS*$TILE_WIDTH]
set CORE_HEIGHT [expr ($ROWS+1)*$TILE_GAP+$ROWS*$TILE_HEIGHT]

set_preferred_routing_direction   -layers {M2 M4 M6} -direction horizontal
set_preferred_routing_direction   -layers {M1 M3 M5} -direction vertical

# [stevo]: set number of cores to use
set_host_options -max_cores $ICC_NUM_CORES

# set tie nets
derive_pg_connection -tie -power_net $MW_POWER_NET -ground_net $MW_GROUND_NET
# derive power and ground for macros (SRAMs)
set power_pins [get_pins -of_objects [all_macro_cells] -all -filter {@name=~vdd*}]
set ground_pins [get_pins -of_objects [all_macro_cells] -all -filter {@name=~gnd*}]
foreach_in_collection power_pin $power_pins {
  derive_pg_connection -power_net $MW_POWER_NET -power_pin [get_attribute $power_pin name] -cells [get_attribute $power_pin cell_name]
}
foreach_in_collection ground_pin $ground_pins {
  derive_pg_connection -ground_net $MW_GROUND_NET -ground_pin [get_attribute $ground_pin name] -cells [get_attribute $ground_pin cell_name]
}

# [stevo]: set unit tile name if different from default "unit"
#set_fp_strategy -unit_tile_name unit12T

# [stevo]: let tool decide where to place IO, we can optimize later when we generate pad frame
remove_pin_pad_physical_constraints
set_pin_physical_constraints -layers {M4} [get_ports]
set_fp_pin_constraints \
  -block_level \
  -use_physical_constraints on \
  -corner_keepout_percent_side 15 \

#[ben] Specify width and height manually so as to have specific positions for each tile
create_floorplan \
  -control_type width_and_height \
  -core_width $CORE_WIDTH \
  -core_height $CORE_HEIGHT \
  -flip_first_row \
  -start_first_row \
  -left_io2core 7 \
  -bottom_io2core 7 \
  -right_io2core 7 \
  -top_io2core 7

set_fp_placement_strategy -macros_on_edge auto
set_fp_macro_options [all_macro_cells ] -legal_orientation {FN FS N S}
#set_keepout_margin -type soft -north -outer {2 2 2 2} [all_macro_cells]

# [ben] Plan groups for MIM flow
create_plan_groups -target_utilization 0.8 -target_aspect_ratio 1 -cycle_color [get_cells -hier $HIERARCHICAL_CELLS]

set_object_snap_type -enabled true
set_object_snap_type -class plan_group -snap row_tile
set_object_snap_type -class hard_macro -snap row_tile

# note: if you change this, change it for DC above as well
# first place all plan groups, then adjust to line up with grid
set x 0
foreach tile $HIERARCHICAL_CELLS {
  set ROW [expr $x/$COLS]
  set COL [expr $x%$COLS]
  set LLX [expr $TILE_GAP*($COL+1)+$TILE_WIDTH*$COL]
  set LLY [expr $TILE_GAP*($ROW+1)+$TILE_HEIGHT*$ROW]
  set URX [expr $LLX+$TILE_WIDTH]
  set URY [expr $LLY+$TILE_HEIGHT]
  set_object_boundary [get_cells -hier $tile] -bbox [list $LLX $LLY $URX $URY]
  incr x
}
set x 0
set EVEN 0
set unit_tile_height [get_attribute [get_core_area] tile_height]
foreach tile $HIERARCHICAL_CELLS {
  set TILE_LLY [get_attribute [get_plan_groups [get_attribute [get_cells -hier $tile] full_name]] bbox_lly]
  set TILE_URY [get_attribute [get_plan_groups [get_attribute [get_cells -hier $tile] full_name]] bbox_ury]
  set CORE_LLY [lindex [lindex [get_attribute [get_core_areas] bbox] 0] 1]
  if { $x == 0 } {
    if { int(($TILE_LLY-$CORE_LLY)/$unit_tile_height)%2 == 0 } {
      set EVEN 1
    }
  } else { 
    if { int(($TILE_LLY-$CORE_LLY)/$unit_tile_height)%2 == 0 && $EVEN == 0 } {
      set_attribute [get_plan_groups [get_attribute [get_cells -hier $tile] full_name]] bbox_lly [expr $TILE_LLY+$unit_tile_height]
      set_attribute [get_plan_groups [get_attribute [get_cells -hier $tile] full_name]] bbox_ury [expr $TILE_URY+$unit_tile_height]
    } elseif { int(($TILE_LLY-$CORE_LLY)/$unit_tile_height)%2 != 0 && $EVEN == 1 } {
      set_attribute [get_plan_groups [get_attribute [get_cells -hier $tile] full_name]] bbox_lly [expr $TILE_LLY+$unit_tile_height]
      set_attribute [get_plan_groups [get_attribute [get_cells -hier $tile] full_name]] bbox_ury [expr $TILE_URY+$unit_tile_height]
    }
  }
  incr x
}

# create initial placement
create_fp_placement
    
# copy a middle MIM cell
copy_mim [get_attribute [get_cells -hier [lindex $HIERARCHICAL_CELLS [expr int([sizeof_collection [get_cells -hier $HIERARCHICAL_CELLS]]/2)]]] full_name]

set_dont_touch_placement [all_macro_cells]

foreach tile $HIERARCHICAL_CELLS {
  set TILE_POLY [get_attribute [get_plan_groups [get_attribute [get_cells -hier $tile] full_name]] points]
  create_power_plan_regions $tile -polygon $TILE_POLY
}
synthesize_fp_rings \
  -nets [list $MW_POWER_NET $MW_GROUND_NET] \
  -power_plan_regions [get_power_plan_regions] \
  -layers {M5 M4} \
  -width {1 1} \
  -space {0.5 0.5} \
  -offset {0.5 0.5}

# create power distribution network
synthesize_fp_rings \
  -nets [list $MW_POWER_NET $MW_GROUND_NET] \
  -core \
  -layers {M9 M8} \
  -width {2 2} \
  -space {1 1} \
  -offset {1 1}


set_power_plan_strategy macros \
  -nets [list $MW_POWER_NET $MW_GROUND_NET] \
  -macros [all_macro_cells] \
  -template tile.tpl:macro_mesh

set_power_plan_strategy groups \
  -nets [list $MW_POWER_NET $MW_GROUND_NET] \
  -power_plan_regions [get_power_plan_regions] \
  -template tile.tpl:tile_mesh \
  -extension {stop: innermost_ring}

set_power_plan_strategy core \
  -nets [list $MW_POWER_NET $MW_GROUND_NET] \
  -core \
  -template core.tpl:core_mesh \
  -extension {stop: outermost_ring} \
  -blockage {plan_group: [get_plan_groups]}

suppress_message "PGRT-030"

set_preroute_drc_strategy \
  -min_layer M4 \
  -max_layer M5
compile_power_plan -strategy groups
set_preroute_drc_strategy \
  -min_layer M6 \
  -max_layer M7
compile_power_plan -strategy macros
set_preroute_drc_strategy \
  -min_layer M8 \
  -max_layer M9
compile_power_plan -strategy core

set_preroute_drc_strategy

# add filler cells so all cell sites are populated
# synopsys recommends you do this before routing standard cell power rails
insert_stdcell_filler \
  -cell_without_metal $FILLER_CELL \
  -connect_to_power [list $MW_POWER_NET] \
  -connect_to_ground [list $MW_GROUND_NET]

# [stevo]: prevent any vias from being made
  # JENNY this causes issues for the following script doing set_preroute_drc_strategy
set_preroute_drc_strategy \
  -min_layer M2 \
  -max_layer M2

# preroute standard cell rails
preroute_standard_cells \
  -connect horizontal     \
  -port_filter_mode off   \
  -cell_master_filter_mode off    \
  -cell_instance_filter_mode off  \
  -voltage_area_filter_mode off \
  -do_not_route_over_macros \
  -no_routing_outside_working_area \
  -route_pins_on_layer M2 \
  -fill_empty_rows \
  -fill_empty_sites

# [stevo]: drop vias
set_preroute_drc_strategy
#create_preroute_vias -nets [list $MW_POWER_NET $MW_GROUND_NET] -from_layer IA -to_layer M2 -from_object_strap -to_object_std_pin_connection
set mim_master_bbox [get_attribute [get_plan_groups [get_attribute [get_cells -hier [lindex $HIERARCHICAL_CELLS [expr int([sizeof_collection [get_cells -hier $HIERARCHICAL_CELLS]]/2)]]] full_name]] bbox]
#create_preroute_vias -nets [list $MW_POWER_NET $MW_GROUND_NET] -from_layer B1 -to_layer M2 -from_object_strap -to_object_std_pin_connection -within $mim_master_bbox
#create_preroute_vias -nets [list $MW_POWER_NET $MW_GROUND_NET] -from_layer M6 -to_layer M4 -from_object_strap -to_object_macro_io_pin -within $mim_master_bbox

# get rid of filler cells
remove_stdcell_filler -stdcell

# copy a middle MIM cell
copy_mim [get_attribute [get_cells -hier [lindex $HIERARCHICAL_CELLS [expr int([sizeof_collection [get_cells -hier $HIERARCHICAL_CELLS]]/2)]]] full_name]

#########################################################################################
## Pin Assignment for MIM                                                               #
#########################################################################################
set_fp_pin_constraints \
  -use_physical_constraints on \
  -allow_feedthroughs off \
  -allowed_layers "M3 M4 M5" \
  -keep_buses_together on \
  -corner_keepout_percent_side 10

select_mim_master_instance [get_attribute [get_cells -hier [lindex $HIERARCHICAL_CELLS [expr int([sizeof_collection [get_cells -hier $HIERARCHICAL_CELLS]]/2)]]] full_name]

set tile_pins [get_attribute [get_pins -of_objects [get_cells -hier [lindex $HIERARCHICAL_CELLS 0]]] name]

foreach pin_name $tile_pins {
  foreach instance $HIERARCHICAL_CELLS {
    # [ben] Place tile ports based on facing
    if {[string first "io_ports_0" $pin_name] != -1} {
      set_pin_physical_constraints -side 1 -cell [get_attribute [get_cells -hier $instance] full_name] -pin_name $pin_name
    } elseif {[string first "io_ports_1" $pin_name] != "-1"} {
      set_pin_physical_constraints -side 2 -cell [get_attribute [get_cells -hier $instance] full_name] -pin_name $pin_name
    } elseif {[string first "io_ports_2" $pin_name] != "-1"} {
      set_pin_physical_constraints -side 3 -cell [get_attribute [get_cells -hier $instance] full_name] -pin_name $pin_name
    } elseif {[string first "io_ports_3" $pin_name] != "-1"} {
      set_pin_physical_constraints -side 4 -cell [get_attribute [get_cells -hier $instance] full_name] -pin_name $pin_name
    } elseif {[string first "io_host" $pin_name] != "-1"} {
      set_pin_physical_constraints -side 2 -cell [get_attribute [get_cells -hier $instance] full_name] -pin_name $pin_name
    }
  }
}

report_fp_pin_constraints

place_fp_pins -effort high [get_plan_groups *] 

#save_mw_cel -as dp_place_fp_pins

# placement again after pin assignment
# [stevo]: TODO: is this necessary?
create_fp_placement -effort high -timing_driven -optimize_pins
copy_mim [get_attribute [get_cells -hier [lindex $HIERARCHICAL_CELLS [expr int([sizeof_collection [get_cells -hier $HIERARCHICAL_CELLS]]/2)]]] full_name]

# [stevo]: follow steps in suggested MIM flow
# https://solvnet.synopsys.com/retrieve/customer/application_notes/attached_files/029893/MIM_Flow_ICC_2011_09.pdf?1336783908122
optimize_fp_timing -effort high
# can also try ultra effort?
set_route_zrt_common_options -plan_group_aware all_routing -connect_floating_shapes true
route_zrt_global -effort high
# [stevo]: view ports which can't be budgeted
# check_fp_timing_environment -block_pin_stats
allocate_fp_budget -file_format_spec $RESULTS_DIR/m.sdc; # generates only one SDC for all MIMs
#allocate_fp_budget -file_format_spec $RESULTS_DIR/i.sdc; # generates one SDC for each tile
check_fp_budget_result -file_name $REPORTS_DIR/fp_budget.rpt

source -echo -verbose dp_commit.tcl
