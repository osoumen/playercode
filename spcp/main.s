.include "defines.i"

.section "BANKHEADER" size 256

.db "SNES-SPC700 Sound File Data v0.30"
.db $26, $26
.db $26		; 26 = header contains ID666 information, 27 = header contains no ID666 tag
.db 30		; Version minor (i.e. 30)
.dw main	; PC
.db 0		; A
.db 0		; X
.db 0		; Y
.db $02		; PSW
.db $ff		; SP
.db 0,0		; reserved

.db "C700 spctest                    "
.db "Game title                      "
.db "dumper          "
.db "Comments                        "
.db "03/20/2017",0		; Date SPC was dumped (YYYY/MM/DD)
.db "120"				; Number of seconds to play song before fading out
.db "20000"				; Length of fade in milliseconds
.db "Artist of song                  "
.db 0					; Default channel disables (0 = enable, 1 = disable)
.db 0					; Emulator used to dump SPC: 0 = unknown, 1 = ZSNES, 2 = Snes9x
.dsb 45 0

.ends

.define _INITIAL_WAIT_FRAMES $ff

.fopen "../sample/spclog.dat" fp_spclog
.fread fp_spclog looppoint_l
.fread fp_spclog looppoint_m
.fclose fp_spclog

.org $00
.bank 0 slot 0
.section "VARIABLEAREA" force
.dsb 2,0
.db <spclog
.db >spclog
.db looppoint_l-$03
.db looppoint_m-$80
.db _INITIAL_WAIT_FRAMES
.db 0
.db 0
.db 0
.ends

.org $30
.bank 0 slot 0
.section "WAITTABLEAREA" force
waittable:
.incbin "../sample/waittable.dat"
.ends

.include "spcplayercode.s"

.org $200
.bank 0 slot 0
.section "SRCDIR" force
srcdir:
.incbin "../sample/dirregion.dat" skip 4
.ends

.fopen "../sample/regdump.dat" fp_reg_dump
.repeat DSP_EDL+1
.fread fp_reg_dump edldata
.endr
.fclose fp_reg_dump

.org $600
.bank 0 slot 0
.section "ECHOREGION" force
.dsb edldata << 11,0
.ends

.bank 0 slot 0
.section "SPCLOG"
spclog:
.incbin "../sample/spclog.dat" skip 3
.ends

.fopen "../sample/brrregion.dat" fp_brrregion
.fread fp_brrregion brrregion_l
.fread fp_brrregion brrregion_m
.fclose fp_brrregion

.org brrregion_l + brrregion_m*256
.bank 0 slot 0
.section "SAMPLEDATA" force
wavearea:
.incbin "../sample/brrregion.dat" skip 4
.ends
