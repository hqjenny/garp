#=======================================================================
# UCB VLSI FLOW: Makefile for pt-pwr
#-----------------------------------------------------------------------
# Yunsup Lee (yunsup@cs.berkeley.edu)
#
# This makefile will use Synopsys PrimeTime to check timing and analyze
# power of your gate-level verilog netlist.
#

default: dhrystone

base_dir = $(abspath ../..)
vlsi_dir = $(abspath ..)
sim_dir = $(abspath ../vcs-sim-gl-par)
output_dir = $(sim_dir)/output

include $(vlsi_dir)/Makefrag
include $(base_dir)/Makefrag
include $(vlsi_dir)/dc-syn/Makefrag
include $(vlsi_dir)/icc-par/Makefrag
include $(vlsi_dir)/vcs-sim-gl-par/Makefrag
-include $(generated_dir)/$(MODEL).$(CONFIG).d
include $(vlsi_dir)/pt-pwr/Makefrag

#--------------------------------------------------------------------
# Default make target
#--------------------------------------------------------------------

dhrystone: $(pt_cur_build_dir)/$(pt_reports_dir)/dhrystone.riscv.power.avg.max.report

#--------------------------------------------------------------------
# Clean up
#--------------------------------------------------------------------

junk +=

clean:
	rm -rf build* current* $(junk) *~ \#*
