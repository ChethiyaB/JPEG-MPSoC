//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														//
//	 xtsc-run script for XTSC Verilog SystemC-on-top co-simulation of JPEG ENCODER using Modelsim		//
//														//
//	 Author : Isuru Nawinne											//
//	 Date   : 2012/09/20											//
//														//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#environment


#ifndef EXTRA_PLUS_ARGS
#define EXTRA_PLUS_ARGS
#endif

#ifdef XTSC_RUN_COSIM_64BIT
#define ARCH_FLAG -fPIC
#else
#define ARCH_FLAG -m32
#endif

-logging=$(XTSC_SCRIPT_FILE_PATH)/TextLogger.txt

-cosim=systemc,$(XTSC_RUN_COSIM_VENDOR),$(XTSC_RUN_COSIM_VENDOR_VERSION),$(XTSC_RUN_COSIM_GCC_VERSION)
-dir=xtsc-run

#ifeq ($(XTSC_RUN_COSIM_VENDOR),mentor)
-set_make_parm=modelsim_ini=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/modelsim.ini
-set_make_parm=vlog_args=-mfcu
-set_make_parm=vsim_args=+dumpvars  $(EXTRA_PLUS_ARGS)
#endif

-set_vcd_parm=file_name=waveforms
-create_vcd=waveforms

-set_clock_parm=vcd_handle=waveforms
-create_clock=CLK




//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 													//
//				Xtensa Cores with pin-level interfaces 	 				//
// 													//
//////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////  Core 0  //////////////////////////////////////////////////

// Edit the xtensa_system, xtensa_core, and xtensa_params assignments as required for the first core
-xtensa_system=/home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
-xtensa_core=LX4_uclib_1
-xtensa_params=$(XTSC_SCRIPT_FILE_PATH)/target/portqueue6o.tdk
-set_core_parm=SimPinLevelInterfaces=FIFO_OUT1,FIFO_OUT2,FIFO_OUT3,FIFO_OUT4,FIFO_OUT5,FIFO_OUT6
-set_core_parm=SimStaticVectorSelect=0
-set_core_parm=SimTargetOutput=core0_output.log
-set_core_parm=SimClients=profile --all core0_trace.log
-set_core_parm=SimTargetProgram=$(XTSC_SCRIPT_FILE_PATH)/target/jencoder1a/main.out
-create_core=core0


//////////////////////////////////////////////  Core 1  //////////////////////////////////////////////////

// Edit the xtensa_system, xtensa_core, and xtensa_params assignments as required for the second core
-xtensa_system=/home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
-xtensa_core=LX4_uclib_1
-xtensa_params=$(XTSC_SCRIPT_FILE_PATH)/target/portqueue3i1o.tdk
-set_core_parm=SimPinLevelInterfaces=FIFO_IN1,FIFO_IN2,FIFO_IN3,FIFO_OUT
-set_core_parm=SimTargetOutput=core1_output.log
-core_program=$(XTSC_SCRIPT_FILE_PATH)/target/jencoder1b/main.out
-create_core=core1


//////////////////////////////////////////////  Core 2  //////////////////////////////////////////////////

// Edit the xtensa_system, xtensa_core, and xtensa_params assignments as required for the third core
-xtensa_system=/home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
-xtensa_core=LX4_uclib_1
-xtensa_params=$(XTSC_SCRIPT_FILE_PATH)/target/portqueue1i1o.tdk
-set_core_parm=SimPinLevelInterfaces=FIFO_IN,FIFO_OUT
-set_core_parm=SimTargetOutput=core2_output.log
-core_program=$(XTSC_SCRIPT_FILE_PATH)/target/jencoder1c/dct.out
-create_core=core2


//////////////////////////////////////////////  Core 3  //////////////////////////////////////////////////

// Edit the xtensa_system, xtensa_core, and xtensa_params assignments as required for the fourth core
-xtensa_system=/home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
-xtensa_core=LX4_uclib_1
-xtensa_params=$(XTSC_SCRIPT_FILE_PATH)/target/portqueue2i1oA.tdk
-set_core_parm=SimPinLevelInterfaces=FIFO_IN1,FIFO_IN2,FIFO_OUT
-set_core_parm=SimTargetOutput=core3_output.log
-core_program=$(XTSC_SCRIPT_FILE_PATH)/target/jencoder1d/quant.out
-create_core=core3


//////////////////////////////////////////////  Core 4  //////////////////////////////////////////////////

// Edit the xtensa_system, xtensa_core, and xtensa_params assignments as required for the fifth core
-xtensa_system=/home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
-xtensa_core=LX4_uclib_1
-xtensa_params=$(XTSC_SCRIPT_FILE_PATH)/target/portqueue2i2o.tdk
-set_core_parm=SimPinLevelInterfaces=FIFO_IN1,FIFO_IN2,FIFO_OUT1,FIFO_OUT2
-set_core_parm=SimStaticVectorSelect=0
-set_core_parm=SimTargetOutput=core4_output.log
-core_program=$(XTSC_SCRIPT_FILE_PATH)/target/jencoder1e/huffman.out
-create_core=core4


//////////////////////////////////////////////  Core 5  //////////////////////////////////////////////////

// Edit the xtensa_system, xtensa_core, and xtensa_params assignments as required for the sixth core
-xtensa_system=/home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
-xtensa_core=LX4_uclib_1
-xtensa_params=$(XTSC_SCRIPT_FILE_PATH)/target/portqueue3i.tdk
-set_core_parm=SimPinLevelInterfaces=FIFO_IN1,FIFO_IN2,FIFO_IN3
-set_core_parm=SimTargetOutput=core5_output.log
-core_program=$(XTSC_SCRIPT_FILE_PATH)/target/jencoder1f/main.out
-create_core=core5








//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 													//
//				SystemC proxies for connecting verilog queues 	 			//
// 													//
//////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////  Queue Proxy A2Ba  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueA2Ba
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-A2Ba.v
-create_proxy=A2Ba


/////////////////////////////////////////  Queue Proxy A2Bb  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueA2Bb
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-A2Bb.v
-create_proxy=A2Bb


/////////////////////////////////////////  Queue Proxy A2Bc  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueA2Bc
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-A2Bc.v
-create_proxy=A2Bc


/////////////////////////////////////////  Queue Proxy A2D  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueA2D
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-A2D.v
-create_proxy=A2D


/////////////////////////////////////////  Queue Proxy A2E  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueA2E
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-A2E.v
-create_proxy=A2E


/////////////////////////////////////////  Queue Proxy A2F  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueA2F
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-A2F.v
-create_proxy=A2F


/////////////////////////////////////////  Queue Proxy B2C  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueB2C
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-B2C.v
-create_proxy=B2C


/////////////////////////////////////////  Queue Proxy C2D  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueC2D
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-C2D.v
-create_proxy=C2D


/////////////////////////////////////////  Queue Proxy D2E  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueD2E
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-D2E.v
-create_proxy=D2E


/////////////////////////////////////////  Queue Proxy E2Fa  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueE2Fa
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-E2Fa.v
-create_proxy=E2Fa


/////////////////////////////////////////  Queue Proxy E2Fb  /////////////////////////////////////////////

-set_proxy_parm=module_name=queueE2Fb
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/verilog.sources/queue-E2Fb.v
-create_proxy=E2Fb









//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 													//
//					Connecting the Cores & Queues 	 				//
// 													//
//////////////////////////////////////////////////////////////////////////////////////////////////////////


-connect_clock_proxy=CLK,A2Ba
-connect_core_proxy=core0,FIFO_OUT1,A2Ba
-connect_proxy_core=A2Ba,FIFO_IN1,core1

-connect_clock_proxy=CLK,A2Bb
-connect_core_proxy=core0,FIFO_OUT2,A2Bb
-connect_proxy_core=A2Bb,FIFO_IN2,core1

-connect_clock_proxy=CLK,A2Bc
-connect_core_proxy=core0,FIFO_OUT3,A2Bc
-connect_proxy_core=A2Bc,FIFO_IN3,core1

-connect_clock_proxy=CLK,A2D
-connect_core_proxy=core0,FIFO_OUT4,A2D
-connect_proxy_core=A2D,FIFO_IN1,core3

-connect_clock_proxy=CLK,A2E
-connect_core_proxy=core0,FIFO_OUT5,A2E
-connect_proxy_core=A2E,FIFO_IN1,core4

-connect_clock_proxy=CLK,A2F
-connect_core_proxy=core0,FIFO_OUT6,A2F
-connect_proxy_core=A2F,FIFO_IN1,core5

-connect_clock_proxy=CLK,B2C
-connect_core_proxy=core1,FIFO_OUT,B2C
-connect_proxy_core=B2C,FIFO_IN,core2

-connect_clock_proxy=CLK,C2D
-connect_core_proxy=core2,FIFO_OUT,C2D
-connect_proxy_core=C2D,FIFO_IN2,core3

-connect_clock_proxy=CLK,D2E
-connect_core_proxy=core3,FIFO_OUT,D2E
-connect_proxy_core=D2E,FIFO_IN2,core4

-connect_clock_proxy=CLK,E2Fa
-connect_core_proxy=core4,FIFO_OUT1,E2Fa
-connect_proxy_core=E2Fa,FIFO_IN2,core5

-connect_clock_proxy=CLK,E2Fb
-connect_core_proxy=core4,FIFO_OUT2,E2Fb
-connect_proxy_core=E2Fb,FIFO_IN3,core5








