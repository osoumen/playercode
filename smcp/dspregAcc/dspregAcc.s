.include "defines.i"

.org $200
.section "CODE" semifree

.define P0FLG_TORAM		$80
.define P0FLG_BLKTRAS	$40
.define P0FLG_P0RST		$20

main:
	mov SPC_CONTROL,#$00
	mov SPC_REGADDR,#DSP_FLG
	mov SPC_REGDATA,#$00
	mov y,#0
	mov a,#0
	mov $04,#$00
	mov $05,#$06
initloop:
	mov [$04]+y,a	; 7
	incw $04		; 6
	cmp $05,#$7e	; 5
	bne initloop	; 4
	mov a,SPC_PORT0
ack:
	mov SPC_PORT3,#$ee
loop:
	cmp a,SPC_PORT0		; 3
	beq loop			; 2
	mov a,SPC_PORT0		; 3
	bmi toram			; 2
	mov x,SPC_PORT2		; 3
	mov SPC_REGADDR,x	; 4
	mov SPC_REGDATA,SPC_PORT1
	mov SPC_PORT0,a		; 4
	; wait 64 - 32 cycle
	cmp x,#DSP_KON	; 3
	beq konWait	; 4
	cmp x,#DSP_KOF	; 3
	bne loop	; 4
koffWait:
	mov y,#10	; 2
-
	dbnz y,-	; 4/6
	mov SPC_REGDATA,#0	;5
	
	mov y,#5	; 2
-
	dbnz y,-	; 4/6
	nop			; 2
	bra loop	; 4
konWait:
	mov y,#5	; 2
-
	dbnz y,-	; 4/6
	nop			; 2
	bra loop	; 4
toram:
	mov x,a
	
	setc
	sbc a,#P0FLG_BLKTRAS
	bmi blockTrans
	and a,#P0FLG_P0RST
	bne resetP0
	
	mov y,#0
	mov a,SPC_PORT1
	mov [SPC_PORT2]+y,a
	mov a,x
	mov SPC_PORT0,a
	bra loop
blockTrans:
	mov SPC_PORT3,#$0
	mov $04,SPC_PORT2
	mov $05,SPC_PORT3
	mov a,x
	mov y,#0
	mov SPC_PORT0,a
loop2:
	cmp a,SPC_PORT0
	beq loop2
	mov a,SPC_PORT0
	bmi ack
	mov x,a
	mov a,SPC_PORT1
	mov [$04]+y,a
	incw $04
	mov a,SPC_PORT2
	mov [$04]+y,a
	incw $04
	mov a,SPC_PORT3
	mov [$04]+y,a
	incw $04
	mov a,x
	mov SPC_PORT0,a
	bra loop2
resetP0:
	mov SPC_CONTROL,#$b0
	clrp
	mov SPC_PORT0,x
	jmp !$ffc0

.ends
