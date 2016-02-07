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
	sbt "run-main sha3.Sha3AccelMain $(PROJ) $(MODULE) $(CONFIG) --genHarness --compile --configInstance $(PROJ).$(CONFIG) --backend c --targetDir build/emulator/generated-src"
	date > $(emulator_timestamp)

emulator: $(emulator_timestamp)

run-unit: $(src_files)
	sbt "run --test  --genHarness --compile --backend c --targetDir build/emulator/generated-src"

run-emulator: emulator
	sbt "run-main sha3.Sha3AccelMain $(PROJ) $(MODULE) $(CONFIG) --test --configInstance $(PROJ).$(CONFIG) --targetDir build/emulator/generated-src"

run-emulator-vpd: $(src_files)
	sbt "run-main sha3.Sha3AccelMain $(PROJ) $(MODULE) $(CONFIG) --test --vcd --genHarness --compile --configInstance $(PROJ).$(CONFIG) --backend c --targetDir build/emulator/generated-src"
	
run-emulator-report: emulator
	sbt "run-main sha3.Sha3AccelMain $(PROJ) $(MODULE) $(CONFIG) --test --configInstance $(PROJ).$(CONFIG) --targetDir build/emulator/generated-src" > build/emulator/cpp-report

# Rerun if scala files or Makefile is newer than last time make vlsi has run
$(vlsi_timestamp): $(src_files)
	sbt "run-main sha3.Sha3AccelMain $(PROJ) $(MODULE) $(CONFIG) --genHarness --debug --compile --backend v --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/generated-src"
	date > $(vlsi_timestamp)

vlsi: $(vlsi_timestamp)

run-vlsi-syn: $(src_files) vlsi
	cd build/vlsi/vcs-sim-gl-syn && make CONFIG=$(CONFIG)
	cd ../../..
	sbt "run-main sha3.Sha3AccelMain $(PROJ) $(MODULE) $(CONFIG) --test --backend null --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/vcs-sim-gl-syn --testCommand build/vlsi/vcs-sim-gl-syn/$(MODULE)"

run-vlsi-syn-report: $(src_files) vlsi
	cd build/vlsi/vcs-sim-gl-syn && make CONFIG=$(CONFIG)
	cd ../../..
	sbt "run-main sha3.Sha3AccelMain $(PROJ) $(MODULE) $(CONFIG) --test --backend null --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/vcs-sim-gl-syn --testCommand build/vlsi/vcs-sim-gl-syn/$(MODULE)" > build/vlsi/vcs-syn-report


run-vlsi: $(src_files) vlsi
	sbt "run-main sha3.Sha3AccelMain $(PROJ) $(MODULE) $(CONFIG) --debug --test --backend v --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/generated-src"

run-vlsi-vpd: $(src_files)
	sbt "run-main sha3.Sha3AccelMain $(PROJ) $(MODULE) $(CONFIG) --test --debug --genHarness --compile --backend v --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/generated-src"

run-vlsi-report: $(src_files)
	sbt "run-main sha3.Sha3AccelMain $(PROJ) $(MODULE) $(CONFIG) --test --genHarness --compile --backend v --configInstance $(PROJ).$(CONFIG) --targetDir build/vlsi/generated-src" > build/vlsi/vcs-report

config: $(src_files)
	run-main sha3.Sha3AccelConfigCollect $(PROJ) $(MODULE) $(CONFIG) src/main/scala/config --genHarness --compile --configInstance $(PROJ).$(CONFIG) --backend c --targetDir src/main/scala/config

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
	cd build/vlsi/vcs-sim-rtl && make clean
	cd build/vlsi/vcs-sim-gl-syn && make clean
	rm build/vlsi/vcs-sim-gl-syn/Sha3*

.PHONY: vlsi emulator config deepclean clean
