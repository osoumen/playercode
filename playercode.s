.MEMORYMAP
  SLOTSIZE $8000
  DEFAULTSLOT 0
  SLOT 0 $8000
.ENDME

.ROMBANKSIZE $8000
.ROMBANKS 1

;-------------------------------------------------
; spc dsp define
;-------------------------------------------------
.define DSP_VOL		$00
.define DSP_P		$02
.define DSP_SRCN	$04
.define DSP_ADSR	$05
.define DSP_GAIN	$07
.define DSP_ENVX	$08	; *
.define DSP_OUTX	$09	; *
.define DSP_MVOLL	$0c
.define DSP_MVOLR	$1c
.define DSP_EVOLL	$2c
.define DSP_EVOLR	$3c
.define DSP_KON		$4c
.define DSP_KOF		$5c
.define DSP_FLG		$6c
.define DSP_ENDX	$7c	; *
.define DSP_EFB		$0d
.define DSP_PMON	$2d
.define DSP_NON		$3d
.define DSP_EON		$4d
.define DSP_DIR		$5d
.define DSP_ESA		$6d
.define DSP_EDL		$7d
.define DSP_FIR		$0F

.define waittable	$fa00
.define dirregion	$fb00
.define brrregion	$8000

.section "BANKHEADER"

.db "head"
.dw 16,0
.db "ppse song player"

.db "vers"
.dw 4,0
.dw $0319
.dw $2017

.fopen "spcp.bin" fp
.fsize fp spccode_size
.fclose fp
.db "spcp"
.dw spccode_size,0
.incbin "spcp.bin"

.db "ntvv"
.dw 12,0
.dw EmptyHandler
.dw EmptyHandler
.dw EmptyHandler
.dw EmptyHandler
.dw 0
.dw irqmain

.db "emuv"
.dw 12,0
.dw EmptyHandler
.dw 0
.dw EmptyHandler
.dw EmptyHandler
.dw Start
.dw EmptyHandler

.db "smcp"
.dw endcode,0	; smcコードの容量を取得する方法
.ends

.include "smcp/smcplayercode.inc"

.section "SpcCode" semifree
spccode:
.incbin "smcp/dspregAcc/dspregAcc.bin"
spccode_end:
.ends

.section "EndCode" semifree
endcode:
.db 0	; dummy
.ends
