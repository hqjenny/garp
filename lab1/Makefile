#=======================================================================
# UCB Chisel Flow: Makefile 
#-----------------------------------------------------------------------
# Brian Zimmer (bmzimmer@eecs.berkeley.edu)
#
# This makefile will generate verilog files or an emulator from chisel code


src_files := Makefile src/main/scala/*.scala
vlsi_timestamp    := build/vlsi/generated-src/timestamp    
emulator_timestamp    := build/emulator/generated-src/timestamp    

$(emulator_timestamp): $(src_files)
	sbt "run-main sha3.Sha3AccelMain --genHarness --compile --backend c --targetDir build/emulator/generated-src"
	date > $(emulator_timestamp)

emulator: $(emulator_timestamp)

run-unit: $(src_files)
	sbt "run --test --genHarness --compile --backend c --targetDir build/emulator/generated-src"

run-emulator: emulator
	sbt "run-main sha3.Sha3AccelMain --test --targetDir build/emulator/generated-src"
	
run-emulator-report: emulator
	sbt "run-main sha3.Sha3AccelMain --test --targetDir build/emulator/generated-src" > build/emulator/cpp-report

# Rerun if scala files or Makefile is newer than last time make vlsi has run
$(vlsi_timestamp): $(src_files)
	sbt "run-main sha3.Sha3AccelMain --genHarness --compile --backend v --targetDir build/vlsi/generated-src"
	date > $(vlsi_timestamp)

vlsi: $(vlsi_timestamp)

run-vlsi: $(src_files)
	sbt "run-main sha3.Sha3AccelMain --test --genHarness --compile --backend v --targetDir build/vlsi/generated-src"

run-vlsi-vpd: $(src_files)
	sbt "run-main sha3.Sha3AccelMain --test --debug --genHarness --compile --backend v --targetDir build/vlsi/generated-src"

run-vlsi-report: $(src_files)
	sbt "run-main sha3.Sha3AccelMain --test --genHarness --compile --backend v --targetDir build/vlsi/generated-src" > build/vlsi/vcs-report

clean:
	rm -rf build/vlsi/generated-src/* build/emulator/generated-src/* target project csrc ucli.key

.PHONY: vlsi emulator
