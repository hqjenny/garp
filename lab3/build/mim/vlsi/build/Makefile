#=======================================================================
# UCB VLSI FLOW: Toplevel Makefile
#-----------------------------------------------------------------------
# Yunsup Lee (yunsup@cs.berkeley.edu)
#
# The toplevel makefile connects together all the tools in the
# toolflow, by including Makefrags from subdirectories.
#
# to synthesize:
#  make dc
# to check rtl vs. gate-level netlist (implicitly runs dc):
#  make fm
# to place-and-route (implicitly runs dc)
#  make icc
# to measure energy (implicitly runs dc/icc):
#  make pt
#

base_dir = $(abspath ../..)
vlsi_dir = $(abspath ..)
build_dir = $(abspath .)
sim_dir = $(abspath ./vcs-sim-gl-par)

include $(build_dir)/Makefrag
include $(build_dir)/dc-syn/Makefrag
include $(build_dir)/icc-par/Makefrag
include $(build_dir)/vcs-sim-gl-par/Makefrag
include $(build_dir)/Makefrag-sim
include $(build_dir)/pt-pwr/Makefrag

tools += dc-syn.tile
tools += dc-syn.top
tools += icc-dp
tools += icc-par.tile
tools += dc-syn.top.hier
tools += icc-par.top

tools += dc-syn
tools += icc-par
tools += pt-pwr

clean:
	rm -rf generated-src 

clean-all:
	for tool in $(tools); \
	  do \
        cd $$tool; make clean; cd ..; \
	  done; \
	  rm -rf *~ \#*; \
	  rm -rf generated-src

.PHONY: dc fm icc pt 

