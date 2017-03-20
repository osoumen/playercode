.SUFFIXES : .s .o

PROGNAME = dspregAcc
EXTENS  = .bin
TARGET  = $(PROGNAME)$(EXTENS)
AS = wla-spc700
ASFLAGS = -voi
LD = wlalink
LFLAGS = -vbsi
INCDIR = .

IFILES = defines.i
SFILES = dspregAcc.s
OFILES = $(SFILES:.s=.o)
LSTFILES = $(SFILES:.s=.lst)

all: $(TARGET)

$(TARGET):		$(OFILES) Makefile
				echo [objects] > linkfile
				$(foreach ofile,$(OFILES),echo $(ofile) >> linkfile;)
				$(LD) $(LFLAGS) linkfile $@
				rm -f linkfile
				rm -f $(OFILES)

$(OFILES):	$(SFILES) $(IFILES) Makefile

.s.o : $(SFILES) $(IFILES)
	$(AS) $(ASFLAGS) $*.s

clean:
	rm -f $(OFILES) $(LSTFILES) $(PROGNAME)$(EXTENS) $(PROGNAME).sym
