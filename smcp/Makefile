.SUFFIXES :

TARGET		:=	$(shell basename $(CURDIR))
AS			= wla-65816
ASFLAGS		= -voi
LD			= wlalink
LFLAGS		= -vsi
SOURCES		:=	.
export OUTPUT	:=	$(CURDIR)/$(TARGET)

IFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.inc)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
export OFILES	:=	$(SFILES:.s=.o)
LSTFILES	= $(SFILES:.s=.lst)

SUBDIRS = dspregAcc

.PHONY: all $(SUBDIRS)

all: $(SUBDIRS) $(TARGET)

$(SUBDIRS):
	$(MAKE) -C $@
	
$(TARGET):	$(OFILES) Makefile
	echo [objects] > linkfile
	$(foreach ofile,$(OFILES),echo $(ofile) >> linkfile;)
	$(LD) $(LFLAGS) linkfile $(TARGET).smc
	rm -f linkfile
	rm -f $(OFILES)

$(OFILES):	$(SFILES) $(IFILES) Makefile
	$(foreach src,$(SFILES),$(AS) $(ASFLAGS) $(src);)

clean:
	rm -f $(OFILES) *.lst linkfile $(TARGET).sym $(TARGET).smc
	(for i in $(SUBDIRS) ; do $(MAKE) -C $$i clean || true ; done)