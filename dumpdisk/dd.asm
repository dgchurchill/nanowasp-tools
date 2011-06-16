
BDOS		equ	5
PRSTR		equ	9

NUMCMDS		equ	6			; number of defined commands


org 100h

	ld	(oldsp),sp
	ld	sp,stacktop


	call	clearrxbuf
	call	init_comms


mainloop:
	call	clearrxbuf			; FIXME !!! this is here as a workaround!!

	ld	de,waiting
	ld	c,PRSTR
	call	BDOS
	
waitforDC:
	call	recvbyte
	cp	'D'
	jp	nz,waitforDC
	call	recvbyte
	cp	'C'
	jp	nz,waitforDC

	ld	de,gotDC
	ld	c,PRSTR
	call	BDOS

; get commannd byte
	call	recvbyte
	cp	NUMCMDS
	jp	nc,mainloop			; jmp if invalid command number

	ld	hl,mainloop			; setup return address
	push	hl

	ld	l,a				; jmp to command's routine
	ld	h,0
	add	hl,hl
	ld	de,cmdtable
	add	hl,de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ex	de,hl
	jp	(hl)


finish:
	ld	de,exiting
	ld	c,PRSTR
	call	BDOS

	call	deinit_comms

	ld	sp,(oldsp)
	ret




; delay approx DE milliseconds
delay:
	push	af
	push	bc
del_lp1:
	ld	b,125
del_lp2:
	ld	c,0
	ld	c,0
	djnz	del_lp2

	dec	de
	ld	a,e
	or	d
	jp	nz,del_lp1

	pop	bc
	pop	af
	ret



; set the active drive register (also does side, double density enable)
setdrive:
	call	recvbyte
	out	(48h),a
	ld	de,100
	call	delay
	ld	a,0			; success
	call	xmitbyte
	ret


; move r/w head to track 0
seek_track0:
	ld	a,03h
	call	fdc_sendcmd
	call	fdc_busywait
	in	a,(40h)			; return status
	call	xmitbyte
	ret


; seek to track x
seek:
	ld	de,seeking
	ld	c,PRSTR
	call	BDOS

	call	recvbyte		; = track number
	out	(43h),a
	ld	a,13h
	call	fdc_sendcmd
	call	fdc_busywait
	in	a,(40h)			; return status
	call	xmitbyte
	ret


; perform a raw dump of the current track
dump_track:
	ld	de,dumping
	ld	c,PRSTR
	call	BDOS

	ld	hl,buffer

	ld	a,0E4h
	out	(40h),a
dt_wait:
	in	a,(48h)
	or	a
	jp	p,dt_wait

	in	a,(43h)
	ld	(hl),a
	inc	hl

	in	a,(40h)
	and	01h
	jp	nz,dt_wait


	in	a,(40h)			; return status
	call	xmitbyte

	ld	de,buffer
	or	a			; clear carry
	sbc	hl,de			; hl = bytes read from track
	ld	a,l
	call	xmitbyte
	ld	a,h
	call	xmitbyte

	call	start_chksum

dt_send:
	ld	a,(de)
	call	xmit_chksum
	inc	de

	dec	hl
	ld	a,l
	or	h
	jp	nz,dt_send

	ld	a,(checksum)
	call	xmitbyte
	ld	a,(checksum+1)
	call	xmitbyte

	ret



read_sector:
        ld      de,reading
	ld	c,PRSTR
	call	BDOS

        call    recvbyte                ; = sector number
        out     (42h),a

	ld	hl,buffer

        ld      a,084h
	out	(40h),a

;!!!bug, last byte of sector is read twice!!!!
rs_wait:
	in	a,(48h)
	or	a
        jp      p,rs_wait

	in	a,(43h)
	ld	(hl),a
	inc	hl

	in	a,(40h)
	and	01h
        jp      nz,rs_wait


	in	a,(40h)			; return status
	call	xmitbyte

	ld	de,buffer
	or	a			; clear carry
	sbc	hl,de			; hl = bytes read from track
	ld	a,l
	call	xmitbyte
	ld	a,h
	call	xmitbyte

	call	start_chksum

rs_send:
	ld	a,(de)
	call	xmit_chksum
	inc	de

	dec	hl
	ld	a,l
	or	h
        jp      nz,rs_send

	ld	a,(checksum)
	call	xmitbyte
	ld	a,(checksum+1)
	call	xmitbyte

	ret



; send a command to the fdc, and delay as required
fdc_sendcmd:
	push	de
	out	(40h),a
	ld	de,32
	call	delay
	pop	de
	ret


; wait for the fdc's busy flag to clear
fdc_busywait:
	push	af
fdc_bwlp:
	in	a,(40h)
	and	01h
	jp	nz,fdc_bwlp
	pop	af
	ret



waiting:	db	"waiting...",0Dh,0Ah,"$"
gotDC:		db	"got DC",0Dh,0Ah,"$"
seeking:	db	"seeking",0Dh,0Ah,"$"
dumping:	db	"dumping",0Dh,0Ah,"$"
reading:        db	"reading",0Dh,0Ah,"$"
exiting:	db	"exiting.",0Dh,0Ah,"$"


cmdtable:	dw	finish
		dw	setdrive
		dw	seek_track0
		dw	seek
		dw	dump_track
                dw	read_sector


oldsp:	ds 2

ds	64
stacktop:


include ../comms.asm


phase 8100h
buffer:
dephase

end
