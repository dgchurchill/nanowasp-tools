
TXBITDELAY	equ	6			; don't ask :P


org 100h


; init pio
	ld	hl,piosetup
	ld	bc,0503h
	otir

	ld	a,1
	out	(0Bh),a

	ld	b,0
	call	dump

	ld	b,20h
	call	dump

	xor	a
	out	(0Bh),a


; deinit pio
	ld	a,07h		; disable pio interrupts
	out	(03h),a

	ret


dump:
	ld	a,0Ch
	out	(0Ch),a
	ld	c,0Dh
	out	(c),b
	ld	a,50
delay:
	ld	b,0FFh
	djnz	$
	dec	a
	jr	nz,delay
	

	ld	hl,0F000h
	ld	bc,800h
send:
	ld	a,(hl)
	call	xmitbyte
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jr	nz,send

	ret



piosetup:	db 00h		; interrupt vector
		db 0FFh		; set mode = bit control
		db 99h		; i/o direction
				; b6 = output, speaker
				; b5 = output, txd
				; b4 = input,  rxd
				; b3 = output, cts
				; b2 = output, clk
				; b1 = output, tape out
				; b0 = input,  tape in
		db 0B7h		; set interrupt control
				; interrupt on any bit high
				; mask follows
		db 0FFh		; 1111 1111
				; monitor nothing



dotxbitdelay:
	push	bc
	ld	b,TXBITDELAY
dbd_lp:
	djnz	dbd_lp
	pop	bc
	ret


; transmit a byte
; a = byte to transmit
xmitbyte:
	push    bc
	push    hl

	ld      l, 0		; start bit is b0 here

	ld      b, 8		; data bits
xb_lp1:
	rra
	adc     hl, hl
	djnz    xb_lp1		; l[7->0] = a[0->7]


	scf			; stop bit
	adc     hl, hl


	ld      b, 4		; push the bits left to line up with port B data reg
xb_lp2:
	add     hl, hl
	inc     l
	djnz    xb_lp2


	in      a, (2)
	and     5Eh		; mask in
				; speaker, rxd, cts, clk,
				; tape out
	ld      d, a

	di
	ld      b, 10
	jr      xb_jp3

xb_sendlp:
	call    dotxbitdelay

xb_jp3:
	ld	a, h
	add	hl, hl
	and	20h
	or      d
	out     (2), a
	djnz	xb_sendlp

	ei

	call    dotxbitdelay      ; for stop bit

	pop     hl
	pop     bc
	ret


