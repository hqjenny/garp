#=======================================================================
# UCB VLSI FLOW: Makefile for vcs-sim-gl-syn
#-----------------------------------------------------------------------
# Yunsup Lee (yunsup@cs.berkeley.edu)
#
# This makefile will build a rtl simulator and run various tests to
# verify proper functionality.
#

default : all

base_dir = $(abspath ../../..)
vlsi_dir = $(abspath ../..)
build_dir = $(abspath ..)
sim_dir = .

include $(build_dir)/Makefrag
include $(build_dir)/dc-syn/Makefrag
include $(build_dir)/icc-par/Makefrag
include $(build_dir)/vcs-sim-gl-par/Makefrag

#--------------------------------------------------------------------
# Default make target
#--------------------------------------------------------------------

.PHONY: run

all : $(vcs_sim)
	cp $(vcs_sim) Sha3Accel
	-rm Sha3Accel.daidir
	ln -s $(vcs_sim).daidir Sha3Accel.daidir

#--------------------------------------------------------------------
# Clean up
#--------------------------------------------------------------------

junk += simv* csrc *.vcd *.vpd *.key DVE* .vcs* timestamp *.out *.h *.a *.ucli sdfAnnotateInfo *.saif

clean :
	rm -rf $(junk) *~ \#* *.log *.cmd *.daidir
