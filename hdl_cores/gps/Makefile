#************************************************************************
# Copyright (C) 2018 Massachusetts Institute of Technology
#
# File Name:      Makefile 
# Program:        Common Evaluation Platform (CEP)
# Description:    
# Notes:          
#************************************************************************
default:
	@echo "sys      : Create a VMEM file to be loaded onto a SoC"
	@echo "vcd_core : Verilator - Generate VCD for core"
	@echo "pcode    : iVerilog  - Verify the pcode core with a Verilog Testbench"
	@echo "clean    : Remove all generated files"

sys:
	or1k-elf-g++ -Wall -Wno-long-long -pedantic -O3 ../../software/bootloader/vc707_CEP.S tb_sys.cpp -o tb_sys
	or1k-elf-objdump -D tb_sys > tb_sys.S
	or1k-elf-objcopy -O binary tb_sys tb_sys.bin
	../../software/utils/bin2vmem tb_sys.bin > tb_sys.vmem
	cp tb_sys.vmem ../ram/sram.vmem

pcode:
	mkdir -p ./iverilog/
	iverilog -Wall tb_pcode.v pcode.v -o ./iverilog/pcode.sim
	./iverilog/pcode.sim
	gtkwave ./iverilog/tb_pcode.vcd

clean:
	rm -rf obj_dir iverilog *~ *.S *.bin *.vmem `find . -type f -executable`
