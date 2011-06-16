;
;  receive byte delays:
;    start:
;       processing = 93 states
;       delay = 13*startdelay
;          + some states in sensing start bit and finish of instruction before interrupt
;
;       need to skip start bit, and end up somewhere in the middle of the first bit
;
;       total time should be about 70 nanosecs = 236 states
;           startdelay = (236-93)/13 = 11
;
;           uh, but let's make it 13 :)
;
;
;    receive bit:
;     processing time = 46 states
;     delay time = 63 + 13*delay states
;
;     19200bps = 52.1 nanosec / bit
;              = 175.8 states / bit  @ 3.375 MHz
;
;     receive delay = (175.8 - 46 - 63) / 13
;                   = 5.13
;     using a delay of 5 we have:
;           (5 * 13 + 63 + 46) * ns/state = 51.56 ns/bit
;     with 10 bits we have a drift of:
;           (52.1-51.56)*10 = 6.6 ns  all up
;
;
;  for 9600bps we have: 104.2 ns / bit = 351.7 states
;     receive delay = (351.7 - 109)/13 = 18.6
;     using 19: delay = 105.5 ns, which is fine.
;  start delay should be ? probably about 35 ish
;     
; in summary:
;  19200:  start = 13, inter = 5
;  9600:   start = ~35, inter = 19
;


RXBITDELAY	equ	5
TXBITDELAY	equ	6			; don't ask :P
STARTDELAY	equ	13




; initialise the pio
init_comms:
	ld	a,4
	rst	28h
	di
	ld	de,rxinterrupt
	ld	(hl),e
	inc	hl
	ld	(hl),d
	dec	hl
	ld	a,l
	ld	(piosetup),a
	ld	hl,piosetup
	ld	bc,0503h
	otir
	ei
	ret


; deinitialise the pio
deinit_comms:
	ld	a,07h		; disable pio interrupts
	out	(03h),a
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
		db 0EFh		; 1110 1111
				; monitor rxd only



; delay for time b/w bits
dorxbitdelay:
	push	bc
	ld	b,RXBITDELAY
dbd_lp:
	djnz	dbd_lp
	pop	bc
	ret

dotxbitdelay:
	push	bc
	ld	b,TXBITDELAY
	djnz	dbd_lp



; transmit a byte
; a = byte to transmit
xmitbyte:
	push    af
	push    bc
	push    de
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
	pop     de
	pop     bc
	pop     af

; NOTE! fall through to rxinterrupt, in case we missed it



rxinterrupt:
	push	af
	in	a, (02h)		; read port b data
	and	10h			; mask in rxd
	jp	z,rxi_fin		; can probably skip this test

	push	bc
	push	hl


	ld	b,STARTDELAY		; skip start and a half bit

rxi_lp1:
	djnz	rxi_lp1


	ld	b,8			; get 8 data bits
	jp	rxi_jp2

rxi_lp2:
	call	dorxbitdelay
rxi_jp2:
	in	a, (02h)		; read port b data
	and	10h
	sub	10h
	rr	c
	djnz	rxi_lp2


	push	de

	call	ringput


rxi_lp3:
	in	a, (02h)
	and	10h
	jr	nz, rxi_lp3


	pop	de
	pop	hl
	pop	bc

rxi_fin:
	pop	af
	ei
	reti




clearrxbuf:
	ld	a,0
	ld	(rxbufhead),a
	ld	(rxbuftail),a
	ret



; c = char to put
; carry set on buffer full
; destroys a,de,hl
ringput:
	ld	a,(rxbufhead)
	inc	a
	ld	e,a
	ld	a,(rxbuftail)
	cp	e
	jp	z,rp_full

	ld	a,e
	ld	(rxbufhead),a
	dec	e
	ld	d,0
	ld	hl,rxbuf
	add	hl,de
	ld	(hl),c
	or	a		; clear cf
	ret

rp_full:
	scf
	ret



; returns next char in a
; carry set on buffer empty
ringget:
	push	de
	push	hl

	ld	a,(rxbufhead)
	ld	e,a
	ld	a,(rxbuftail)
	cp	e
	jp	z,rg_empty
	
	ld	e,a
	inc	a
	ld	(rxbuftail),a
	ld	d,0
	ld	hl,rxbuf
	add	hl,de
	ld	a,(hl)
	or	a		; clear cf
	jr	rg_fin

rg_empty:
	scf
rg_fin:
	pop	hl
	pop	de
	ret




; transmit a byte and add to checksum
; byte to send in a
xmit_chksum:
	push	bc
	push	hl

	ld	hl,(checksum)
	ld	c,a
	ld	b,0
	add	hl,bc
	ld	(checksum),hl

	pop	hl
	pop	bc

	jp	xmitbyte


; reset the check sum
start_chksum:
	push	de
	ld	e,0
	ld	d,e
	ld	(checksum),de
	pop	de
	ret


; get a byte from the rx buffer and return it in A
; if a byte is not available, wait until one is
recvbyte:
	call	ringget
	jp	c,recvbyte
	ret


rxbufhead:	ds 1
rxbuftail:	ds 1

checksum:	ds 2


phase 8000h
rxbuf:
dephase

