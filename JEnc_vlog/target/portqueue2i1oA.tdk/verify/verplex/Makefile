#
#Makefile for running formal equivalence check
#You must have VERPLEX installed and in your path before you run this
#
default: verplex

.SECONDARY: $(patsubst %.do,%.do.log,$(shell ls *.do 2>/dev/null))

verplex: log warnings $(patsubst %.do,%.check,$(shell ls *.do 2>/dev/null)) report

PERL = /opt/xtensa/XtDevTools/install/tools/RD-2011.1-linux/XtensaTools/Tools/bin/perl
XTENSA_TOOLS = /opt/xtensa/XtDevTools/install/tools/RD-2011.1-linux/XtensaTools/Tools
%.check: %.do ../src/verify_ref.v ../src/verify_sem.v
	@echo "Running verplex on $*" 
	XTENSA_TOOLS=$(XTENSA_TOOLS) $(PERL) -w tie_verplex -check $* | tee -a verplex.log

log:
	@rm -rf verplex.log
	@touch verplex.log

report:
	@echo "============= Report Summary ============="
	@cat verplex.log
	@rm -f *.vector.do *.fpt

clean:
	rm -rf *.log *.lck *.fpt


warnings:
	@echo "The following opcodes are not checked:RECV1, RECV2, SEND, DUMMY \n" | tee -a verplex.log

