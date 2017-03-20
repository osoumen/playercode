.SUFFIXES :

TARGET		:=	$(shell basename $(CURDIR))
AS			= wla-65816
ASFLAGS		= -voi
LD			= wlalink
LFLAGS		= -vbsi
SOURCES		:=	.
export OUTPUT	:=	$(CURDIR)/$(TARGET)

IFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.inc)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
export OFILES	:=	$(SFILES:.s=.o)
LSTFILES	= $(SFILES:.s=.lst)

SUBDIRS = smcp/dspregAcc

.PHONY: all $(SUBDIRS)

all: $(SUBDIRS) $(TARGET)

$(SUBDIRS):
	$(MAKE) -C $@

spcp.bin:
	echo [objects] > linkfile
	echo spcp.o >> linkfile
	wla-spc700 -vo spcp.asm spcp.o
	wlalink -vb linkfile spcp.bin
	
$(TARGET): spcp.bin $(OFILES) Makefile
	echo [objects] > linkfile
	$(foreach ofile,$(OFILES),echo $(ofile) >> linkfile;)
	$(LD) $(LFLAGS) linkfile $(TARGET).bin
	rm -f linkfile
	rm -f $(OFILES) spcp.o spcp.bin

$(OFILES):	$(SFILES) $(IFILES) Makefile
	$(foreach src,$(SFILES),$(AS) $(ASFLAGS) $(src);)

clean:
	rm -f $(OFILES) *.lst linkfile $(TARGET).sym $(TARGET).bin
	(for i in $(SUBDIRS) ; do $(MAKE) -C $$i clean || true ; done)