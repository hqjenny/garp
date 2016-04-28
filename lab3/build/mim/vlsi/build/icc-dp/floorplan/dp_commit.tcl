#########################################################################################
## Commit Plangroups									#
#########################################################################################
## Write out floorplan with plan groups
write_floorplan -create_terminal -placement {io hard_macro} -row -track ${RESULTS_DIR}/top_plangroup.tcl
write_def -version 5.6 -rows -macro -pins -blockages -verbose -output ${RESULTS_DIR}/top.def

echo "INFO: committing plan groups"
commit_fp_plan_groups -push_down_power_and_ground_straps

#preroute_instances \
#  -connect_instances specified \
#  -cells "core_core_Tile_1" \
#  -nets {VDD VSS} \
#  -skip_left_side \
#  -skip_right_side \
#  -route_pins_on_layer b2 \
#  -primary_routing_layer pin \
#  -extend_for_multiple_connections \
#  -extension_gap 10 \
#  -extend_to_boundaries_and_generate_pins

echo "INFO: report_mim"  
report_mim

save_mw_cel -hierarchy 

########################################################################################
#Split Libraries									#
########################################################################################
close_mw_lib
#split_mw_lib -from $MW_DESIGN_LIBRARY berkeley_site

#########################################################################################
# process new library (Tile)
#########################################################################################
# open_mw_lib lib_tile
open_mw_cel $HIERARCHICAL_DESIGNS
create_macro_fram
create_block_abstraction
save_mw_cel
#write_def -version 5.7 -rows_tracks_gcells -macro -pins -blockages -specialnets -vias -regions_groups -verbose -output Tile.DCT.def 
write_def -version 5.7 -rows_tracks_gcells -components -pins -blockages -specialnets -vias -regions_groups -verbose -output Tile.DCT.def 
write_floorplan -create_terminal -create_bound -row -track -preroute -placement {io terminal hard_macro soft_macro} Tile.DCT.fp 
write_verilog -diode_ports -no_physical_only_cells Tile.DCT.v
close_mw_cel
close_mw_lib

#########################################################################################
# process new library (commit_plangroups) - top level design with soft macros for tiles
#########################################################################################
# open_mw_lib lib_top
open_mw_cel $DESIGN_NAME
change_macro_view -reference $HIERARCHICAL_DESIGNS -view FRAM
set_top_implementation_options -block_references $HIERARCHICAL_DESIGNS
save_mw_cel
#write_def -version 5.7 -rows_tracks_gcells -macro -pins -blockages -specialnets -vias -regions_groups -verbose -output Top.DCT.def 
write_def -version 5.7 -rows_tracks_gcells -components -pins -blockages -specialnets -vias -regions_groups -verbose -output Top.DCT.def 
write_floorplan -create_terminal -create_bound -row -track -preroute -placement {io terminal hard_macro soft_macro} Top.DCT.fp 
write_verilog -diode_ports -no_physical_only_cells Top.DCT.v
close_mw_cel
close_mw_lib
stop_gui
exit
