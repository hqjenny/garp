#=======================================================================
# UCB VLSI FLOW: Makefile for vcs-sim-rtl
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

include $(base_dir)/Makefrag
include $(build_dir)/Makefrag
include $(build_dir)/vcs-sim-rtl/Makefrag
include $(build_dir)/Makefrag-sim

#--------------------------------------------------------------------
# Default make target
#--------------------------------------------------------------------

.PHONY: run

all : $(simv)
	cp $(simv) GarpAccel
	-rm GarpAccel.daidir
	ln -s $(simv).daidir GarpAccel.daidir


#--------------------------------------------------------------------
# Clean up
#--------------------------------------------------------------------

junk += simv* csrc *.vpd *.key DVE* .vcs* timestamp *.out *.h *.a *.ucli Garp*

clean :
	rm -rf $(junk) *~ \#* *.log *.cmd *.daidir
