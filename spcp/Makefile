.SUFFIXES : .s .o

PROGNAME = $(shell basename $(CURDIR))
EXTENS  = .spc
TARGET  = $(PROGNAME)$(EXTENS)
AS = wla-spc700
ASFLAGS = -voi
LD = wlalink
LFLAGS = -vSi
INCDIR = .

IFILES = defines.i
SFILES = main.s
OFILES = $(SFILES:.s=.o)
LSTFILES = $(SFILES:.s=.lst)

all: $(TARGET)

$(TARGET):		$(OFILES) linkfile Makefile
				$(LD) $(LFLAGS) linkfile $@

$(OFILES):	$(SFILES) $(IFILES) Makefile

.s.o : $(SFILES) $(IFILES)
	$(AS) $(ASFLAGS) $*.s

clean:
	rm -f $(OFILES) $(LSTFILES) $(PROGNAME)$(EXTENS) $(PROGNAME).sym
