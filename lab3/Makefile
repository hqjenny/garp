#=======================================================================
# UCB Chisel Flow: Makefile 
#-----------------------------------------------------------------------
# Brian Zimmer (bmzimmer@eecs.berkeley.edu)
#
# This makefile will generate verilog files or an emulator from chisel code

include Makefrag

target_dir := build/emulator/generated-src

src_files := Makefile src/main/scala/*.scala
vlsi_timestamp    := build/vlsi/generated-src/timestamp    
emulator_timestamp    := build/emulator/generated-src/timestamp    
chisel_timestamp := chisel/timestamp



$(emulator_timestamp): $(src_files)
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --genHarness --compile --configInstance $(PROJ).$(CONFIG) --backend c --targetDir build/emulator/generated-src"
	date > $(emulator_timestamp)

emulator: $(emulator_timestamp)

run-unit: $(src_files)
	sbt "run --test  --genHarness --compile --backend c --targetDir build/emulator/generated-src"

run-emulator: emulator
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --configInstance $(PROJ).$(CONFIG) --targetDir build/emulator/generated-src"

run-emulator-vpd: $(src_files)
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --vcd --genHarness --compile --configInstance $(PROJ).$(CONFIG) --backend c --targetDir build/emulator/generated-src"
	
run-emulator-report: emulator
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --configInstance $(PROJ).$(CONFIG) --targetDir build/emulator/generated-src" > build/emulator/cpp-report

# Rerun if scala files or Makefile is newer than last time make vlsi has run
$(vlsi_timestamp): $(src_files)
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --genHarness --noInlineMem --debug --backend v --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/generated-src"
	if [ -a build/vlsi/generated-src/$(MODULE).$(CONFIG).conf ]; then $(mem_gen) build/vlsi/generated-src/$(MODULE).$(CONFIG).conf >> build/vlsi/generated-src/$(MODULE).$(CONFIG).v; fi
	g++  -c -o build/vlsi/generated-src/vpi.o -I$$VCS_HOME/include -Ibuild/vlsi/generated-src/ -fPIC -std=c++11 build/vlsi/generated-src/vpi.cpp
	date > $(vlsi_timestamp)

vlsi: $(vlsi_timestamp)

run-vlsi-syn: $(src_files) vlsi
	cd build/vlsi/vcs-sim-gl-syn && make CONFIG=$(CONFIG)
	cd ../../..
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --backend null --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/vcs-sim-gl-syn --testCommand \"build/vlsi/vcs-sim-gl-syn/$(MODULE) -ucli -do build/vlsi/vcs-sim-gl-syn/+run_from_prjdir.tcl\""

run-vlsi-syn-vpd: $(src_files) vlsi
	cd build/vlsi/vcs-sim-gl-syn && make CONFIG=$(CONFIG)
	cd ../../..
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --backend null --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/vcs-sim-gl-syn --testCommand \"build/vlsi/vcs-sim-gl-syn/$(MODULE) -ucli -do build/vlsi/vcs-sim-gl-syn/+run_from_prjdir.tcl\" +vcdfile=build/vlsi/vcs-sim-gl-syn/GarpAccel.vcd"

run-vlsi-syn-report: $(src_files) vlsi
	cd build/vlsi/vcs-sim-gl-syn && make CONFIG=$(CONFIG)
	cd ../../..
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --backend null --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/vcs-sim-gl-syn --testCommand \"build/vlsi/vcs-sim-gl-syn/$(MODULE) -ucli -do build/vlsi/vcs-sim-gl-syn/+run_from_prjdir.tcl\"" > build/vlsi/vcs-syn-report

run-vlsi-par: $(src_files) vlsi
	cd build/vlsi/vcs-sim-gl-par && make SHELL=/bin/bash
	cd ../../..
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --backend null --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/vcs-sim-gl-par --testCommand \"build/vlsi/vcs-sim-gl-par/$(MODULE) -ucli -do build/vlsi/vcs-sim-gl-par/+run_from_prjdir.tcl\""

run-vlsi-par-report: $(src_files) vlsi
	cd build/vlsi/vcs-sim-gl-par && make SHELL=/bin/bash
	cd ../../..
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --backend null --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/vcs-sim-gl-par --testCommand \"build/vlsi/vcs-sim-gl-par/$(MODULE) -ucli -do build/vlsi/vcs-sim-gl-par/+run_from_prjdir.tcl\"" > build/vlsi/vcs-par-report

run-vlsi: $(src_files) vlsi
	cd build/vlsi/vcs-sim-rtl&& make CONFIG=$(CONFIG)
	cd ../../..
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --backend null --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/vcs-sim-rtl --testCommand \"build/vlsi/vcs-sim-rtl/$(MODULE)\""

run-vlsi-vpd: $(src_files) vlsi
	cd build/vlsi/vcs-sim-rtl&& make CONFIG=$(CONFIG)
	cd ../../..
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --backend null --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/vcs-sim-rtl --testCommand \"build/vlsi/vcs-sim-rtl/$(MODULE)\" +vcdfile=build/vlsi/vcs-sim-rtl/GarpAccel.vcd"

run-vlsi-report: $(src_files) vlsi
	cd build/vlsi/vcs-sim-rtl&& make CONFIG=$(CONFIG)
	cd ../../..
	sbt "run-main garp.GarpAccelMain $(PROJ) $(MODULE) $(CONFIG) --test --backend null --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/vcs-sim-rtl --testCommand \"build/vlsi/vcs-sim-rtl/$(MODULE)\"" > build/vlsi/vcs-report

config: $(src_files)
	run-main garp.GarpAccelConfigCollect $(PROJ) $(MODULE) $(CONFIG) src/main/scala/config --genHarness --compile --configInstance $(PROJ).$(CONFIG) --backend c --targetDir src/main/scala/config

chisel: $(chisel_timestamp)

$(chisel_timestamp):
	rm -rf ~/.sbt ~/.ivy2
	cd chisel && 	make publish-local
	date > $(chisel_timestamp)

clean:
	rm -rf build/vlsi/generated-src/* build/emulator/generated-src/* target csrc ucli.key

deepclean:
	#cd build/vlsi && make clean
	cd build/vlsi/dc-syn && make clean
	cd build/vlsi/icc-par && make clean
	cd build/vlsi/vcs-sim-rtl && make clean
	cd build/vlsi/vcs-sim-gl-syn && make clean
	rm build/vlsi/vcs-sim-gl-syn/Garp*
	cd build/vlsi/vcs-sim-gl-par && make clean
	rm build/vlsi/vcs-sim-gl-par/Garp*

.PHONY: vlsi emulator config deepclean clean
