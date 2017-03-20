.memorymap
  slotsize $8000
  defaultslot 0
  slot 0 $8000
.endme

.rombanksize $8000
.rombanks 4

.snesheader
  id "SNES"
  name "c700play             "
  ;    "123456789012345678901"
  fastrom
  lorom
  cartridgetype $00
  romsize $07
  sramsize $00
  country $00
  licenseecode $00
  version $00
.endsnes

.snesnativevector
  cop EmptyHandler
  brk EmptyHandler
  abort EmptyHandler
  nmi EmptyHandler
  irq irqmain
.endnativevector

.snesemuvector
  cop EmptyHandler
  abort EmptyHandler
  nmi EmptyHandler
  reset Start
  irqbrk EmptyHandler
.endemuvector

.emptyfill $00

codestart:
.include "smcplayercode.inc"

.section "SpcCode" semifree
spccode:
.incbin "dspregAcc/dspregAcc.bin"
spccode_end:
.ends
codeend:

.org $7a00
.section "WaitTable" semifree
waittable:
.incbin "../sample/waittable.dat"
.ends

.org $7b00
.section "DirRegion" semifree

dirregion:
; 先頭に、アドレス、サイズが２バイトずつ
.incbin "../sample/dirregion.dat"

.ends

.base $81
.bank 1 slot 0
.org 0
.section "BrrRegion"
brrregion:
; 先頭に、アドレス、サイズが２バイトずつ
.incbin "../sample/brrregion.dat" skip 0; read 32768
.ends

;.base $82
;.bank 2 slot 0
;.org 0
;.section "BrrRegion2"
;brrregion2:
;.incbin "../sample/brrregion.dat" skip 32768
;.ends


.base $83
.bank 3 slot 0
.org 0
.section "RegLogRegion"
reglogregion:
.incbin "../sample/spclog.dat" skip 0; read 32768
.ends

;.base $84
;.bank 4 slot 0
;.org 0
;.section "RegLogRegion2"
;.incbin "../sample/spclog.dat" skip 32768 ;read 32768
;.ends

;
;.base $85
;.bank 5 slot 0
;.org 0
;.section "RegLogRegion3"
;.incbin "../sample/spclog.dat" skip 65536; read 32768
;.ends

;.base $86
;.bank 6 slot 0
;.org 0
;.section "RegLogRegion6"
;.incbin "../sample/spclog.dat" skip 163840 read 32768
;.ends
;
;.base $87
;.bank 7 slot 0
;.org 0
;.section "RegLogRegion7"
;.incbin "../sample/spclog.dat" skip 196608 read 32768
;.ends