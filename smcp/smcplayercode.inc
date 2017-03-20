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

.define SEQ_PTR_LSB $00
.define SEQ_PTR_MSB $01
.define SEQ_PTR_BANK $02
.define WAIT_FRAMES $04
.define P0STATE $06
.define LOOP_PTR_LSB $0a
.define LOOP_PTR_MSB $0b
.define LOOP_PTR_BANK $0c
.define Y_TEMP $0e
.define BRR_END_BYTES $10
.define BRR_END_ADDR $12
.define BRR_REST_BYTES $14
.define A_TEMP $15

;.define WAITSYNC_LSB $20
;.define WAITSYNC_MSB $30

.define _INITIAL_SEQ_BANK $83
.define _INITIAL_WAIT_FRAMES 60

.macro incAddr
	ldx SEQ_PTR_LSB		;3
	inx					;2
	bmi ++				;2
	ldx #$8000			;2
	inc SEQ_PTR_BANK	;5
++
	stx SEQ_PTR_LSB		;3
.endm

;========================
; Start
;========================

.bank 0 slot 0
.org 0
.section "EmptyVectors" semifree
EmptyHandler:
	rti
.ends

.base $80
.bank 0 slot 0
.org 0
.section "MainCode" semifree

Start:
	jml	main
main:
	clc
	xce
	rep #$10
	sep #$20
	
	; PPUレジスタをクリア
	ldx #$33
@loop:
	stz $2100,x
	stz $4200,x
	dex
	bpl @loop
	
	; 背景色を設定
	lda #$ff
	sta $2122
	lda #$ff
	sta $2122
	
	lda #$0b
	sta $2100
	
	; FastRom用の設定
	lda $420d
	ora #$01
	sta $420d
	
	; サウンドの初期化
	jsr initSound
	
	; DSPアクセス用コードを転送
	ldy.w #$0010
	jsr spc_begin_upload
-
	lda spccode,y
	jsr spc_upload_byte
	cpy #(spccode_end - spccode)
	bne -
	tya
	inc a
	sta P0STATE
	
	; DIR領域を転送
	ldy.w dirregion
	jsr spc_begin_upload
-
	lda dirregion+4,y
	jsr spc_upload_byte
	cpy.w dirregion+2
	bne -
	tya
	inc a
	sta P0STATE
	
	; $0010に転送したプログラムを実行
	ldy.w #$0010
	jsr spc_execute

	; プログラム起動待ち
	lda #$ee
-
	cmp $2143
	bne -
	
	; ポート情報の初期化
	lda #$01
	sta P0STATE
	
	; 波形データの転送
	lda #$81
	pha
	plb
	ldy.w brrregion
	ldx.w brrregion+2
	jsr transBrrData

	lda #$82
	pha
	plb
	ldy.w brrregion
	ldx.w brrregion+2
	beq @noUpperBank
	jsr transBrrData
@noUpperBank:
	lda #$80
	pha
	plb
	
	; 画面を真っ暗にする
	lda #$00
	sta $2100
restart:
	; --再生処理--
	; 変数初期化
	stz SEQ_PTR_LSB
	stz SEQ_PTR_MSB
	ldx #_INITIAL_WAIT_FRAMES
	stx WAIT_FRAMES
	lda #_INITIAL_SEQ_BANK
	sta SEQ_PTR_BANK
	
	; ループポイントの読み込み
	ldy #$8000
	lda [SEQ_PTR_LSB], y
	sta LOOP_PTR_LSB
	iny
	lda [SEQ_PTR_LSB], y
	sta LOOP_PTR_MSB
	iny
	lda [SEQ_PTR_LSB], y
	clc
	adc #_INITIAL_SEQ_BANK
	sta LOOP_PTR_BANK
	iny
	sty SEQ_PTR_LSB
	
	; HLine割り込みタイミングを設定
	lda #0
	sta $4208
	lda #100
	sta $4207
	
	; 割り込みを開始
	ldy #0
	cli
	lda #$10
	sta $4200

	; メインループ開始
loop:
	cpy #0			;2
	beq loop		;2
	dey				;2
	
	rep #$20		;3
	dec WAIT_FRAMES	;5
	sep #$20		;3
	bne loop		;2
next:
	lda [SEQ_PTR_LSB]	;6
	
	; 負数ならwaitコマンド
	bmi cntlcmd		;2
	
regset00:
	;DSPアドレス
	sta $2142
	
	; pitchだけは2バイト連続
	and #$0f
	cmp #$03
	beq pitchSet
	
	; DSPデータ
	incAddr
	lda [SEQ_PTR_LSB]
	sta $2141
	
	lda P0STATE
	sta $2140
	incAddr
-
	cmp $2140
	bne -
	eor #$01
	sta P0STATE
	bra next
	
cntlcmd:
; $80 xx xx yy: spc側xxxxにyyを書き込み
; $90: 1シンク
; $92 xx: 8bitシンク
; $94 xx xx: 16bitシンク
; $9e: END/LOOP
; $a0-df: デフォルト値シンク
	rep #$20			;3
	and #$007e			;2
	tax					;2
	sep #$20			;3
	jmp (cmdTable,x)	;6
	
pitchSet:
	; LSBアドレスを退避
	lda [SEQ_PTR_LSB]
	dec a
	pha

	; DSPデータ
	incAddr
	lda [SEQ_PTR_LSB]
	sta $2141

	lda P0STATE
	sta $2140
	incAddr
-
	cmp $2140
	bne -
	eor #$01
	sta P0STATE

	; PitchのLSBが続く
	pla
	sta $2142
	lda [SEQ_PTR_LSB]
	sta $2141

	lda P0STATE
	sta $2140
	incAddr
-
	cmp $2140
	bne -
	eor #$01
	sta P0STATE
	jmp next
	
ramsetcmd:
	; RAMアドレス
	incAddr				;17
	lda [SEQ_PTR_LSB]	;6
	sta $2142			;4
	incAddr				;17
	lda [SEQ_PTR_LSB]	;6
	sta $2143			;4
	
	; RAMデータ
	incAddr				;17
	lda [SEQ_PTR_LSB]	;6
	sta $2141			;4

	lda P0STATE			;3
	sta $2140			;4
	incAddr				;17
-
	cmp $2140			;4
	bne -				;2
	eor #$01			;2
	sta P0STATE			;3
	jmp next			;3
	
sync8bit:
	incAddr
	lda [SEQ_PTR_LSB]
	sta WAIT_FRAMES
	incAddr
	stz WAIT_FRAMES+1
	jmp loop

sync8bitx2:
	incAddr
	lda [SEQ_PTR_LSB]
	rep #$20
	rol
	and #$01fe
	sta WAIT_FRAMES
	sep #$20
	incAddr
	jmp loop
	
sync8bitx4:
	incAddr
	lda [SEQ_PTR_LSB]
	rep #$20
	rol
	rol
	and #$03fc
	sta WAIT_FRAMES
	sep #$20
	incAddr
	jmp loop
	
sync8bitx8:
	incAddr
	lda [SEQ_PTR_LSB]
	rep #$20
	rol
	rol
	rol
	and #$07f8
	sta WAIT_FRAMES
	sep #$20
	incAddr
	jmp loop

sync8bitx16:
	incAddr
	lda [SEQ_PTR_LSB]
	rep #$20
	rol
	rol
	rol
	rol
	and #$0ff0
	sta WAIT_FRAMES
	sep #$20
	incAddr
	jmp loop

sync16bit:
	incAddr
	lda [SEQ_PTR_LSB]
	sta WAIT_FRAMES
	incAddr
	lda [SEQ_PTR_LSB]
	sta WAIT_FRAMES+1
	incAddr
	jmp loop

dataend:
	lda LOOP_PTR_BANK
	sta SEQ_PTR_BANK
	ldx LOOP_PTR_LSB
	stx SEQ_PTR_LSB
	jmp next

;syncVariable:
;	rep #$20
;	and #$000f
;	tax
;	sep #$20
;	lda WAITSYNC_LSB,x
;	sta WAIT_FRAMES
;	lda WAITSYNC_MSB,x
;	sta WAIT_FRAMES+1
;	incAddr
;	jmp loop
syncVariable0:
	lda.w waittable
	sta WAIT_FRAMES
	lda.w waittable+1
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable1:
	lda.w waittable+2
	sta WAIT_FRAMES
	lda.w waittable+3
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable2:
	lda.w waittable+4
	sta WAIT_FRAMES
	lda.w waittable+5
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable3:
	lda.w waittable+6
	sta WAIT_FRAMES
	lda.w waittable+7
	sta WAIT_FRAMES+1
	incAddr
	jmp loop	
syncVariable4:
	lda.w waittable+8
	sta WAIT_FRAMES
	lda.w waittable+9
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable5:
	lda.w waittable+10
	sta WAIT_FRAMES
	lda.w waittable+11
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable6:
	lda.w waittable+12
	sta WAIT_FRAMES
	lda.w waittable+13
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable7:
	lda.w waittable+14
	sta WAIT_FRAMES
	lda.w waittable+15
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable8:
	lda.w waittable+16
	sta WAIT_FRAMES
	lda.w waittable+17
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable9:
	lda.w waittable+18
	sta WAIT_FRAMES
	lda.w waittable+19
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable10:
	lda.w waittable+20
	sta WAIT_FRAMES
	lda.w waittable+21
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable11:
	lda.w waittable+22
	sta WAIT_FRAMES
	lda.w waittable+23
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable12:
	lda.w waittable+24
	sta WAIT_FRAMES
	lda.w waittable+25
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable13:
	lda.w waittable+26
	sta WAIT_FRAMES
	lda.w waittable+27
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable14:
	lda.w waittable+28
	sta WAIT_FRAMES
	lda.w waittable+29
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable15:
	lda.w waittable+30
	sta WAIT_FRAMES
	lda.w waittable+31
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable16:
	lda.w waittable+32
	sta WAIT_FRAMES
	lda.w waittable+33
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable17:
	lda.w waittable+34
	sta WAIT_FRAMES
	lda.w waittable+35
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable18:
	lda.w waittable+36
	sta WAIT_FRAMES
	lda.w waittable+37
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable19:
	lda.w waittable+38
	sta WAIT_FRAMES
	lda.w waittable+39
	sta WAIT_FRAMES+1
	incAddr
	jmp loop	
syncVariable20:
	lda.w waittable+40
	sta WAIT_FRAMES
	lda.w waittable+41
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable21:
	lda.w waittable+42
	sta WAIT_FRAMES
	lda.w waittable+43
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable22:
	lda.w waittable+44
	sta WAIT_FRAMES
	lda.w waittable+45
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable23:
	lda.w waittable+46
	sta WAIT_FRAMES
	lda.w waittable+47
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable24:
	lda.w waittable+48
	sta WAIT_FRAMES
	lda.w waittable+49
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable25:
	lda.w waittable+50
	sta WAIT_FRAMES
	lda.w waittable+51
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable26:
	lda.w waittable+52
	sta WAIT_FRAMES
	lda.w waittable+53
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable27:
	lda.w waittable+54
	sta WAIT_FRAMES
	lda.w waittable+55
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable28:
	lda.w waittable+56
	sta WAIT_FRAMES
	lda.w waittable+57
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable29:
	lda.w waittable+58
	sta WAIT_FRAMES
	lda.w waittable+59
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable30:
	lda.w waittable+60
	sta WAIT_FRAMES
	lda.w waittable+61
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
syncVariable31:
	lda.w waittable+62
	sta WAIT_FRAMES
	lda.w waittable+63
	sta WAIT_FRAMES+1
	incAddr
	jmp loop
	
;syncValSet:
;	sta A_TEMP
;	incAddr
;	lda [SEQ_PTR_LSB]
;	pha
;	incAddr
;	lda [SEQ_PTR_LSB]
;	pha
;	lda A_TEMP
;	rep #$20
;	ror a
;	and #$000f
;	tax
;	sep #$20
;	pla
;	sta WAITSYNC_MSB,x
;	pla
;	sta WAITSYNC_LSB,x
;	incAddr
;	jmp next

wait1sync:
	incAddr
	lda #1
	sta WAIT_FRAMES
	stz WAIT_FRAMES+1
	jmp loop

unknownCmd:
	jmp next

cmdTable:
.dw ramsetcmd
.dsw 7 unknownCmd
.dw wait1sync,sync8bit,sync16bit,sync8bitx2,sync8bitx4,sync8bitx8,sync8bitx16,dataend
.dw syncVariable0,syncVariable1,syncVariable2,syncVariable3,syncVariable4,syncVariable5,syncVariable6,syncVariable7,syncVariable8,syncVariable9,syncVariable10,syncVariable11,syncVariable12,syncVariable13,syncVariable14,syncVariable15
.dw syncVariable16,syncVariable17,syncVariable18,syncVariable19,syncVariable20,syncVariable21,syncVariable22,syncVariable23,syncVariable24,syncVariable25,syncVariable26,syncVariable27,syncVariable28,syncVariable29,syncVariable30,syncVariable31
;.dsw 16 syncValSet
.dsw 16 unknownCmd


irqmain:
	cmp $4211
	iny
	rti


; IPLを利用してDSPレジスタに値を書き込む
; X レジスタの上位に値を、下位にアドレスを入れておく
write_dsp:
	phx
	; $00F2-$00F3 へ 続けて２バイトの転送を行うことで
	; DSPアドレス、値を順に書き込む
	ldy.w #$00F2
	jsr spc_begin_upload
	pla
	jsr spc_upload_byte     ; X の下位バイトを $F2 へ
	pla
	jsr spc_upload_byte     ; X の上位バイトを $F3 へ
	rts

; SPC側へ転送を行うには
; 1. spc_wait_boot を呼ぶ
; 2. データを送る
; 	A. spc_begin_upload を呼び出す
; 	B. spc_upload_byte を転送したいバイト数分呼び出す
; 	C. 別のアドレスへ書き込む場合は、Aに戻る
; 3. 転送したコードを実行するには spc_executeを呼ぶ
; Yは16bitモードであること

; SPCの起動待ち 起動後の最初に一度呼ぶ
; X, Y : 使用しません
spc_wait_boot:
	lda #$AA
-	cmp $2140
	bne -

	sta $2140

	lda #$BB
-	cmp $2141
	bne -

	rts

; Yレジスタに格納されているアドレスへ転送を開始します。
; Yレジスタは、その後のspc_upload_byteでインデックスとして使用するため、0に初期化されます。
; X : 使用しません
spc_begin_upload:
	sty $2142

	; 開始コマンド
	lda #$01
	sta $2141
	lda P0STATE
	sta $2140

	; 受理されると$2140が0になる
-	cmp $2140
	bne -

	; Yを初期化
	ldy.w #0

	rts

; Aレジスタの値を1バイト転送し、Yレジスタをインクリメントします。
; これを呼び続けている間は、Yレジスタの下位バイトを変更しないこと。
; X : 使用しません
spc_upload_byte:
	sta $2141

	tya
	sta $2140

-	cmp $2140
	bne -

	iny
	rts

; PCをYレジスタのアドレスにジャンプします
; X, Y : 使用しません
spc_execute:
	sty $2142

	stz $2141

	lda P0STATE
	sta $2140

	; 受理されるのを待つ
-	cmp $2140
	bne -

	rts

initSound:
	jsr spc_wait_boot
	; ノイズ回避のため音量を0に
	lda	#$cc
	sta P0STATE
	ldx.w #($00 << 8 | DSP_MVOLL)
	jsr write_dsp
	tya
	inc a
	sta P0STATE

	ldx.w #($00 << 8 | DSP_MVOLR)
	jsr write_dsp
	tya
	inc a
	sta P0STATE

	ldx.w #($00 << 8 | DSP_EVOLL)
	jsr write_dsp
	tya
	inc a
	sta P0STATE

	ldx.w #($00 << 8 | DSP_EVOLR)
	jsr write_dsp
	tya
	inc a
	sta P0STATE

	; EDL,ESAを初期化
	ldx.w #($20 << 8 | DSP_FLG)
	jsr write_dsp
	tya
	inc a
	sta P0STATE

	ldx.w #($00 << 8 | DSP_EDL)
	jsr write_dsp
	tya
	inc a
	sta P0STATE

	ldx.w #($06 << 8 | DSP_ESA)
	jsr write_dsp
	tya
	inc a
	sta P0STATE

	; EDL,ESAを変更したので240ms(859080 clock)待ち
	ldy #(65536 - 53504)
-
	nop			; 2
	nop			; 2
	iny			; 2
	bne -		; 2
	
	rts
	
	
transBrrData:
	; 転送済みプログラムを用いてRAMを転送
	sty BRR_END_ADDR	;後でバイト数を足す
	
	sty $2142
	lda P0STATE
	ora #$c0
	sta $2140
-
	cmp $2140
	bne -
	and #$01
	eor #$01
	sta P0STATE
	; バイト数3をで割る
	txy
	sty $4204
	lda #3
	sta $4206
	; 割り算が終わるまで16サイクルのウェイト
	iny	;2
	iny	;2
	iny	;2
	iny	;2
	nop	;2
	nop	;2
	nop	;2
	nop	;2
	
	; 3で割った余りをBRR_REST_BLOCKSに入れる
	lda $4216
	sta BRR_REST_BYTES
	; 3で割った余りをyから引く
	cmp #1
	bne +
	dey
+
	cmp #2
	bne +
	dey
	dey
+
	sty BRR_END_BYTES	;3
	
	; ブロック転送終了apuアドレスを計算
	dey
	dey
	dey
	dey
	sty Y_TEMP
	rep #$20
	lda BRR_END_ADDR
	clc
	adc Y_TEMP
	sta BRR_END_ADDR
	sep #$20

	; ３バイトずつ転送
	ldy #4	; アドレス、サイズの次
@loadBank:
	lda brrregion,y
	sta $2141
	lda brrregion+1,y
	sta $2142
	lda brrregion+2,y
	sta $2143

	lda #2
	sta $2140
	iny
	iny
	iny
-
	cmp $2140
	bne -
	; 終了判定
	cpy BRR_END_BYTES
	beq @loadBank_end

	lda brrregion,y
	sta $2141
	lda brrregion+1,y
	sta $2142
	lda brrregion+2,y
	sta $2143

	lda #3
	sta $2140
	iny
	iny
	iny
-
	cmp $2140
	bne -
	; 終了判定
	cpy BRR_END_BYTES
	bmi @loadBank
@loadBank_end:

	; ブロック転送を終了させる
	lda P0STATE
	ora #$80
	sta $2140
	eor #$01
	sta P0STATE
	lda #$ee
-
	cmp $2143
	bne -

	;余り(1か2)バイトがあれば転送する
	lda BRR_REST_BYTES
	beq @noRestBytes
	cmp #01
	beq +
	lda brrregion,y
	iny
	sta $2141
	lda BRR_END_ADDR
	sta $2142
	lda BRR_END_ADDR+1
	sta $2143
	lda P0STATE
	sta $2140
-
	cmp $2140
	bne -
	eor #$01
	sta P0STATE
+
	lda brrregion,y
	iny
	sta $2141
	lda BRR_END_ADDR
	ina
	sta $2142
	lda BRR_END_ADDR+1
	sta $2143
	lda P0STATE
	sta $2140
-
	cmp $2140
	bne -
	eor #$01
	sta P0STATE

@noRestBytes:
	; brr転送終わり
	lda P0STATE
	and #$01
	sta P0STATE
	
	rts
.ends
