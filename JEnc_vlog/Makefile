# Customer ID=4869; Build=0x3007c; Copyright (c) 2007-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.


##############################################################################################################
# Makefile for XTSC Verilog SystemC-on-top co-simulation of JPEG ENCODER using Modelsim
#
# Author : Isuru Nawinne
# Date   : 2012/09/11
##############################################################################################################



# Put all required target object file names here
TARGET0 = target/jencoder1a/main.out
TARGET1 = target/jencoder1b/main.out
TARGET2 = target/jencoder1c/dct.out
TARGET3 = target/jencoder1d/quant.out
TARGET4 = target/jencoder1e/huffman.out
TARGET5 = target/jencoder1f/main.out


XTENSA_SYSTEM0 = /home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
XTENSA_CORE0 = LX4_uclib_1
TIE0_FILE = target/portqueue6o.tie
XTENSA_SYSTEM1 = /home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
XTENSA_CORE1 = LX4_uclib_1
TIE1_FILE = target/portqueue3i1o.tie
XTENSA_SYSTEM2 = /home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
XTENSA_CORE2 = LX4_uclib_1
TIE2_FILE = target/portqueue1i1o.tie
XTENSA_SYSTEM3 = /home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
XTENSA_CORE3 = LX4_uclib_1
TIE3_FILE = target/portqueue2i1oA.tie
XTENSA_SYSTEM4 = /home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
XTENSA_CORE4 = LX4_uclib_1
TIE4_FILE = target/portqueue2i2o.tie
XTENSA_SYSTEM5 = /home/isuru/xtensa/XtDevTools/install/builds/RD-2011.1-linux/LX4_uclib_1/config
XTENSA_CORE5 = LX4_uclib_1
TIE5_FILE = target/portqueue3i.tie


XTTOOLS = /home/isuru/xtensa/XtDevTools/install/tools/RD-2011.1-linux/XtensaTools


TARGET_CXX = $(XTTOOLS)/bin/xt-xcc
TARGET_CFLAGS  = -g -O2
TARGET_LIBS = -lm
LINK = $(CXX)
DEBUG_CXXFLAGS = -UNDEBUG -DDEBUG -Wall -O0 -g
#DEBUG_CXXFLAGS = -DNDEBUG -UDEBUG -Wall -O3
COMMON_CXXFLAGS = -m32 -c $(DEBUG_CXXFLAGS)
LDFLAGS = -m32
LDOUT = -o 
OBJ = o
EXE = 
RM = rm -f
RMDIR = rm -rf


INCLUDES = \
	$(XTSC_COMP_INC) \
	-I$(XTTOOLS)/include \
	-I$(SYSTEMC_INC) 
XTSC_LIBS = \
	$(XTSC_COMP_LIB) \
	-L$(XTTOOLS)/lib/$(XTSCLIBDIR) \
	-lxtsc \
	-lxtsc_comp \
	-lxtmp \
	-lsimxtcore \
	-llog4xtensa \
	-L$(XTTOOLS)/lib \
	-lxtparams
OTHER_LIBS = -Wl,-rpath,$(XTTOOLS)/lib/$(ISSLIBDIR):$(XTTOOLS)/lib
D = -D


LIBS = $(XTSC_LIBS) $(SYSTEMC_LIBS) $(OTHER_LIBS)
DEFINES = \
	$(D) CONFIG0_NAME=\"$(XTENSA_CORE0)\" \
	$(D) XTENSA0_REGISTRY=\"$(XTENSA_SYSTEM0)\" \
	$(D) TDK0_DIR=\"$(TDK0_DIR)\" \
	$(D) TARGET0_PROGRAM=\"$(TARGET0)\" \
	$(D) CONFIG1_NAME=\"$(XTENSA_CORE1)\" \
	$(D) XTENSA1_REGISTRY=\"$(XTENSA_SYSTEM1)\" \
	$(D) TDK1_DIR=\"$(TDK1_DIR)\" \
	$(D) TARGET1_PROGRAM=\"$(TARGET1)\" \
	$(D) CONFIG2_NAME=\"$(XTENSA_CORE2)\" \
	$(D) XTENSA2_REGISTRY=\"$(XTENSA_SYSTEM2)\" \
	$(D) TDK2_DIR=\"$(TDK2_DIR)\" \
	$(D) TARGET2_PROGRAM=\"$(TARGET2)\" \
	$(D) CONFIG3_NAME=\"$(XTENSA_CORE3)\" \
	$(D) XTENSA3_REGISTRY=\"$(XTENSA_SYSTEM3)\" \
	$(D) TDK3_DIR=\"$(TDK3_DIR)\" \
	$(D) TARGET3_PROGRAM=\"$(TARGET3)\" \
	$(D) CONFIG4_NAME=\"$(XTENSA_CORE4)\" \
	$(D) XTENSA4_REGISTRY=\"$(XTENSA_SYSTEM4)\" \
	$(D) TDK4_DIR=\"$(TDK4_DIR)\" \
	$(D) TARGET4_PROGRAM=\"$(TARGET4)\" \
	$(D) CONFIG5_NAME=\"$(XTENSA_CORE5)\" \
	$(D) XTENSA5_REGISTRY=\"$(XTENSA_SYSTEM5)\" \
	$(D) TDK5_DIR=\"$(TDK5_DIR)\" \
	$(D) TARGET5_PROGRAM=\"$(TARGET5)\" \
        $(ADDITIONAL_DEFINES)

CXXFLAGS = $(INCLUDES) $(DEFINES) $(COMMON_CXXFLAGS)


ifndef XTSC_RUN_COSIM_VENDOR
XTSC_RUN_COSIM_VENDOR=mentor
endif

ifndef XTSC_RUN_COSIM_VENDOR_VERSION
XTSC_RUN_COSIM_VENDOR_VERSION=6.6a
endif

ifndef XTSC_RUN_COSIM_GCC
XTSC_RUN_COSIM_GCC=g++
endif

ifndef XTSC_RUN_COSIM_GCC_VERSION
XTSC_RUN_COSIM_GCC_VERSION=4.1.2
endif


XTENSA_XMP_ROOT    = $(CURDIR)/../../../XMP
XTENSA_XMP_XSYS    = $(XTENSA_XMP_ROOT)/$(SYSTEM).xsys
XTENSA_XMP_SYSTEM  = $(XTENSA_XMP_ROOT)/sys-x86-linux/$(SYSTEM)
XTENSA_XMP_PACKAGE = $(XTENSA_XMP_SYSTEM)/package
XTENSA_XMP_DEFS    = $(XTENSA_XMP_PACKAGE)/sys-defs.mk
XTENSA_XMP_CORES   = $(XTENSA_XMP_PACKAGE)/cores
XMP_XTENSA_SYSTEM  = $(XTENSA_XMP_PACKAGE)/config


BASE_NAME := JEnc_vlog

.PHONY: all custom_target run clean target tie 

.PRECIOUS: target/%.c

.SUFFIXES :
.SUFFIXES : .c .cpp .cxx .o

all: target xtsc-run
	cd xtsc-run && make all

xtsc-run: xtsc-run.inc
	@echo "===================================================="
	@echo "=  Using xtsc-run elaboration for code generation  ="
	@echo "===================================================="
	rm -rf xtsc-run && xtsc-run -include=xtsc-run.inc
	cp param.txt xtsc-run/.
	cp 227flowg1.raw xtsc-run/.

run:
	cd xtsc-run && make run




custom_target: 


target:  cleant $(TARGET0) $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) custom_target


$(EXAMPLE_NAME): $(HOST_OBJECTS)
	$(LINK) $(LDFLAGS) $(LDOUT)$@ $(HOST_OBJECTS) $(LIBS)

$(TARGET0): $(TARGET0:.out=.c)
	$(TARGET_CXX) --xtensa-system=$(XTENSA_SYSTEM0) --xtensa-core=$(XTENSA_CORE0) --xtensa-params=$(TDK0_DIR) \
                      $(TARGET_CFLAGS) -o $@ $< $(TARGET_LIBS) 

$(TARGET1): $(TARGET1:.out=.c)
	$(TARGET_CXX) --xtensa-system=$(XTENSA_SYSTEM1) --xtensa-core=$(XTENSA_CORE1) --xtensa-params=$(TDK1_DIR) \
                      $(TARGET_CFLAGS) -o $@ $< $(TARGET_LIBS)

$(TARGET2): $(TARGET2:.out=.c)
	$(TARGET_CXX) --xtensa-system=$(XTENSA_SYSTEM2) --xtensa-core=$(XTENSA_CORE2) --xtensa-params=$(TDK2_DIR) \
                      $(TARGET_CFLAGS) -o $@ $< $(TARGET_LIBS)

$(TARGET3): $(TARGET3:.out=.c)
	$(TARGET_CXX) --xtensa-system=$(XTENSA_SYSTEM3) --xtensa-core=$(XTENSA_CORE3) --xtensa-params=$(TDK3_DIR) \
                      $(TARGET_CFLAGS) -o $@ $< $(TARGET_LIBS)

$(TARGET4): $(TARGET4:.out=.c)
	$(TARGET_CXX) --xtensa-system=$(XTENSA_SYSTEM3) --xtensa-core=$(XTENSA_CORE3) --xtensa-params=$(TDK4_DIR) \
                      $(TARGET_CFLAGS) -o $@ $< $(TARGET_LIBS)

$(TARGET5): $(TARGET5:.out=.c)
	$(TARGET_CXX) --xtensa-system=$(XTENSA_SYSTEM4) --xtensa-core=$(XTENSA_CORE4) --xtensa-params=$(TDK5_DIR) \
                      $(TARGET_CFLAGS) -o $@ $< $(TARGET_LIBS)


%.$(OBJ): %.cpp
	$(CXX) $(CXXFLAGS) $<


clean:
	-$(RM) target/jencoder1a/*.out *.$(OBJ)
	-$(RM) target/jencoder1b/*.out *.$(OBJ)
	-$(RM) target/jencoder1c/*.out *.$(OBJ)
	-$(RM) target/jencoder1d/*.out *.$(OBJ)
	-$(RM) target/jencoder1e/*.out *.$(OBJ)
	-$(RM) target/jencoder1f/*.out *.$(OBJ)
	-$(RM) target/*.out *.$(OBJ)
	-$(RM) *.log *.vcd
	-$(RMDIR) xtsc-run

cleant:
	-$(RM) target/jencoder1a/*.out *.$(OBJ)
	-$(RM) target/jencoder1b/*.out *.$(OBJ)
	-$(RM) target/jencoder1c/*.out *.$(OBJ)
	-$(RM) target/jencoder1d/*.out *.$(OBJ)
	-$(RM) target/jencoder1e/*.out *.$(OBJ)
	-$(RM) target/jencoder1f/*.out *.$(OBJ)
	-$(RM) target/*.out *.$(OBJ)



TDK0_DIR = $(TIE0_FILE:.tie=.tdk)
TDK1_DIR = $(TIE1_FILE:.tie=.tdk)
TDK2_DIR = $(TIE2_FILE:.tie=.tdk)
TDK3_DIR = $(TIE3_FILE:.tie=.tdk)
TDK4_DIR = $(TIE4_FILE:.tie=.tdk)
TDK5_DIR = $(TIE5_FILE:.tie=.tdk)


$(TARGET0): $(TDK0_DIR)
$(TARGET1): $(TDK1_DIR)
$(TARGET2): $(TDK2_DIR)
$(TARGET3): $(TDK3_DIR)
$(TARGET4): $(TDK4_DIR)
$(TARGET5): $(TDK5_DIR)


ifneq ($(TDK0_DIR),)
$(TDK0_DIR): $(TIE0_FILE)
	XTENSA_SYSTEM=$(XTENSA_SYSTEM0) XTENSA_CORE=$(XTENSA_CORE0) XTENSA_PARAMS=$(TDK0_DIR) \
	$(XTTOOLS)/bin/tc $(TCFLAGS)  -d $@ $<       
endif

ifneq ($(TDK1_DIR),)
$(TDK1_DIR): $(TIE1_FILE)
	XTENSA_SYSTEM=$(XTENSA_SYSTEM1) XTENSA_CORE=$(XTENSA_CORE1) XTENSA_PARAMS=$(TDK1_DIR) \
	$(XTTOOLS)/bin/tc $(TCFLAGS)  -d $@ $<       
endif

ifneq ($(TDK2_DIR),)
$(TDK2_DIR): $(TIE2_FILE)
	XTENSA_SYSTEM=$(XTENSA_SYSTEM2) XTENSA_CORE=$(XTENSA_CORE2) XTENSA_PARAMS=$(TDK2_DIR) \
	$(XTTOOLS)/bin/tc $(TCFLAGS)  -d $@ $<       
endif

ifneq ($(TDK3_DIR),)
$(TDK3_DIR): $(TIE3_FILE)
	XTENSA_SYSTEM=$(XTENSA_SYSTEM3) XTENSA_CORE=$(XTENSA_CORE3) XTENSA_PARAMS=$(TDK3_DIR) \
	$(XTTOOLS)/bin/tc $(TCFLAGS)  -d $@ $<       
endif

ifneq ($(TDK4_DIR),)
$(TDK4_DIR): $(TIE4_FILE)
	XTENSA_SYSTEM=$(XTENSA_SYSTEM4) XTENSA_CORE=$(XTENSA_CORE4) XTENSA_PARAMS=$(TDK4_DIR) \
	$(XTTOOLS)/bin/tc $(TCFLAGS)  -d $@ $<       
endif

ifneq ($(TDK5_DIR),)
$(TDK5_DIR): $(TIE5_FILE)
	XTENSA_SYSTEM=$(XTENSA_SYSTEM5) XTENSA_CORE=$(XTENSA_CORE5) XTENSA_PARAMS=$(TDK5_DIR) \
	$(XTTOOLS)/bin/tc $(TCFLAGS)  -d $@ $<       
endif








