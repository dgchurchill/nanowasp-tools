; simple talk program, characters received are shown on screen
; and then echoed back to sender


org 100h

	ld	(oldsp),sp
	ld	sp,stacktop


	call	clearrxbuf
	call	init_comms


	ld	hl,0F000h

mainloop:
	call	ringget
	jr	c,mainloop

	call	xmitbyte
	ld	(hl),a
	inc	hl
	cp	'X'
	jr	nz,mainloop

	call	deinit_comms

	ld	sp,(oldsp)
	ret


oldsp:	ds 2

ds	64
stacktop:

include comms.asm

end

