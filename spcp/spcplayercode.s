.define SEQ_PTR_LSB $02
.define SEQ_PTR_MSB $03
.define LOOP_PTR_LSB $04
.define LOOP_PTR_MSB $05
.define WAIT_FRAMES $06
.define COUNT_FRAMES $08
.define RAMWR_ADDR_LSB $0a
.define RAMWR_ADDR_MSB $0b

.org $70
.bank 0 slot 0
.section "CMDTABLEAREA" force
cmdTable:
.dw ramsetcmd
.dsw 7 unknownCmd
.dw wait1sync,sync8bit,sync16bit,sync8bitx2,sync8bitx4,sync8bitx8,sync8bitx16,dataend
.dsw 32 syncVariable
.dsw 16 unknownCmd
cmdTable_end:
.ends

.org $f0
.bank 0 slot 0
.section "REGISTERAREA" force
.dsb 16,$00
.ends

.org $100
.bank 0 slot 0
.section "CODE"

main:
	; 変数の初期化
;	mov SEQ_PTR_LSB, #<spclog
;	mov SEQ_PTR_MSB, #>spclog
;	mov WAIT_FRAMES, #_INITIAL_WAIT_FRAMES
;	mov WAIT_FRAMES+1, #$0
;	mov COUNT_FRAMES, #0
	
	; ループポイントの読み込み
	mov y, #0
;	mov a, [SEQ_PTR_LSB]+y
;	mov LOOP_PTR_LSB, a
;	incw SEQ_PTR_LSB
;	mov a, [SEQ_PTR_LSB]+y
;	mov LOOP_PTR_MSB, a
;	incw SEQ_PTR_LSB
;	setc
;	sbc LOOP_PTR_MSB, #$80
	clrc
	adc LOOP_PTR_LSB, SEQ_PTR_LSB
	adc LOOP_PTR_MSB, SEQ_PTR_MSB
	
	; エコーの有効化
	mov SPC_REGADDR,#DSP_FLG
	mov SPC_REGDATA,#$00
	
	; タイマーの開始
	mov SPC_TIMER2, #4
	mov SPC_CONTROL, #CNT_TIMER2_ON
	
loop:
	mov a, SPC_COUNTER2
	clrc
	adc a, COUNT_FRAMES
	beq loop
	dec a
	mov COUNT_FRAMES, a
	
	decw WAIT_FRAMES
	bne loop
next:
	mov a, [SEQ_PTR_LSB]+y
	
	bmi cntlcmd
	
regset00:
	mov SPC_REGADDR, a
	mov x, a
	
	and a, #$0f
	cmp a, #$03
	beq pitchSet
	
	incw SEQ_PTR_LSB
	mov a, [SEQ_PTR_LSB]+y
	mov SPC_REGDATA, a
	incw SEQ_PTR_LSB
	; kon,koff時にwait
	cmp x,#DSP_KON	; 3
	beq konWait		; 4
	cmp x,#DSP_KOF	; 3
	bne next		; 4
	
koffWait:
	mov y,#10	; 2
-
	dbnz y,-	; 4/6
	mov SPC_REGDATA,#0	;5

	mov y,#5	; 2
-
	dbnz y,-	; 4/6
	mov y, #0	; 2
	bra next	; 4
konWait:
	mov y,#5	; 2
-
	dbnz y,-	; 4/6
	mov y, #0	; 2
	bra next	; 4
	
cntlcmd:
; $80 xx xx yy: spc側xxxxにyyを書き込み
; $90: 1シンク
; $92 xx: 8bitシンク
; $94 xx xx: 16bitシンク
; $9e: END/LOOP
; $a0-df: デフォルト値シンク
	and a, #$7e
	mov x, a
	jmp [!cmdTable+x]
	
pitchSet:
	mov a, [SEQ_PTR_LSB]+y
	dec a
	mov x, a
	
	incw SEQ_PTR_LSB
	mov a, [SEQ_PTR_LSB]+y
	mov SPC_REGDATA, a
	
	incw SEQ_PTR_LSB
	mov SPC_REGADDR, x
	mov a, [SEQ_PTR_LSB]+y
	mov SPC_REGDATA, a
	
	incw SEQ_PTR_LSB
	bra next

ramsetcmd:
	incw SEQ_PTR_LSB
	mov a, [SEQ_PTR_LSB]+y
	mov RAMWR_ADDR_LSB, a
	incw SEQ_PTR_LSB
	mov a, [SEQ_PTR_LSB]+y
	mov RAMWR_ADDR_MSB, a
	incw SEQ_PTR_LSB
	mov a, [SEQ_PTR_LSB]+y
	mov [RAMWR_ADDR_LSB]+y, a
	bra next
	
sync8bit:
	incw SEQ_PTR_LSB
	mov a, [SEQ_PTR_LSB]+y
	mov WAIT_FRAMES, a
	incw SEQ_PTR_LSB
	mov WAIT_FRAMES+1, #0
	bra loop

	
sync16bit:
	incw SEQ_PTR_LSB
	mov a, [SEQ_PTR_LSB]+y
	mov WAIT_FRAMES, a
	incw SEQ_PTR_LSB
	mov a, [SEQ_PTR_LSB]+y
	mov WAIT_FRAMES+1, a
	incw SEQ_PTR_LSB
	jmp loop

sync8bitx2:
	mov x, #2
	bra sync8bitxX
	
sync8bitx4:
	mov x, #4
	bra sync8bitxX

sync8bitx8:
	mov x, #8
	bra sync8bitxX
		
sync8bitx16:
	mov x, #16
	bra sync8bitxX
	
sync8bitxX:
	incw SEQ_PTR_LSB
	mov a, [SEQ_PTR_LSB]+y
	mov y, a
	mov a, x
	mul ya
	mov WAIT_FRAMES, a
	mov WAIT_FRAMES+1, y
	mov y, #0
	incw SEQ_PTR_LSB
	jmp loop

dataend:
	mov SEQ_PTR_LSB, LOOP_PTR_LSB
	mov SEQ_PTR_MSB, LOOP_PTR_MSB
	jmp next

syncVariable:
	setc
	sbc a, #$20
	mov x, a
	mov a, waittable+x
	mov WAIT_FRAMES, a
	mov a, waittable+1+x
	mov WAIT_FRAMES+1, a
	incw SEQ_PTR_LSB
	jmp loop
	
wait1sync:
	incw SEQ_PTR_LSB
	mov WAIT_FRAMES, #1
	mov WAIT_FRAMES+1, #0
	jmp loop
	
unknownCmd:
	jmp next

code_end:
.ends
