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
	@echo "top      : Verilator - verify the core with a Wishbone bus interface"
	@echo "core     : Verilator - Verify the core"
	@echo "vcd      : Verilator - Generate VCD for top"
	@echo "vcd_core : Verilator - Generate VCD for core"
	@echo "iver     : iVerilog  - Verify the core with a Verilog Testbench"
	@echo "clean    : Remove all generated files"

sys:
	or1k-elf-g++ -Wall -Wno-long-long -pedantic -O3 ../../../software/bootloader/vc707_CEP.S tb_sys.cpp -o tb_sys
	or1k-elf-objdump -D tb_sys > tb_sys.S
	or1k-elf-objcopy -O binary tb_sys tb_sys.bin
	../../../software/utils/bin2vmem tb_sys.bin > tb_sys.vmem
	cp tb_sys.vmem ../../ram/sram.vmem

top:
	verilator -Wno-fatal --cc --clk clk --default-language 1364-2001 -I. filter_idft.v idft_top_top.v --top-module idft_top_top -exe tb_top.cpp
	make -j -C obj_dir -f Vidft_top_top.mk Vidft_top_top
	./obj_dir/Vidft_top_top

core:
	verilator -Wno-fatal --cc --clk clk --default-language 1364-2001 -I. filter_idft.v --top-module idft_top -exe tb_core.cpp
	make -j -C obj_dir -f Vidft_top.mk Vidft_top
	./obj_dir/Vidft_top

vcd:
	verilator -Wno-fatal --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. filter_idft.v idft_top_top.v --top-module idft_top_top -exe tb_top.cpp
	make -j -C obj_dir -f Vidft_top_top.mk Vidft_top_top
	./obj_dir/Vidft_top_top
	gtkwave ./obj_dir/Vidft_top_top.vcd

vcd_core:
	verilator -Wno-fatal --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. filter_idft.v --top-module idft_top -exe tb_core.cpp
	make -j -C obj_dir -f Vidft_top.mk Vidft_top
	./obj_dir/Vidft_top
	gtkwave ./obj_dir/Vidft_top.vcd

iver:
	mkdir -p ./iverilog/
	iverilog -Wall tb_filter_idft.v filter_idft.v -o ./iverilog/idft.sim
	./iverilog/idft.sim

clean:
	rm -rf obj_dir iverilog *~ *.S *.bin *.vmem `find . -type f -executable`
