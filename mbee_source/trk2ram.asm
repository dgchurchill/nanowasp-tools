;
; DGC 2011-06-16:  This program seems to read raw tracks from the disk in 
;                  drive B and store them in RAM at 300H.  I think the
;                  idea was to run this and then run the CP/M command that
;                  saved a chunk of memory to disk.
;
;
	ORG	100H
;
;	SET UP STACK
	LXI	H,0
	DAD	SP
;	ENTRY STACK POINTER IN HL FROM THE CCP
	SHLD	OLDSP
;	SET SP TO LOCAL STACK AREA (RESTORED AT FINIS)
	LXI	SP,STKTOP

; set drive b:
	MVI	A,9
	CALL	SETDRIVE

	CALL	FDCTRK0
	MVI	A,'S'
	JNZ	ERROR

	LXI	D,300H
	MVI	A,5

LOOP:
	PUSH	PSW
	CALL	DMPTRK
	ORA	A
	JNZ	ERROR
;	CALL	DELAY
	POP	PSW
	INR	A
	CPI	10
	JZ	EXIT
	JMP	LOOP

ERROR:
	LXI	D,0F1B8H
	STAX	D	
	INX	D	
	POP	PSW
	ADI	'0'
	STAX	D

EXIT:
; set drive a:
	MVI	A,0
	CALL	SETDRIVE

	LHLD	OLDSP
	SPHL
;	STACK POINTER CONTAINS CCP'S STACK LOCATION
	RET		;TO THE CCP



; DE -> buffer
; A = track number 
; returns: A = -1 on any error
DMPTRK:
	PUSH	PSW
; wait for any previous command to finish
	CALL	FDCWAIT
		
; seek to desired track
	POP	PSW
	OUT	43H
	MVI	A,12H
	CALL	FDCCMD
	CALL	FDCWAIT

; dump track
	MVI	A,0E4H
	OUT	40H

DTWAIT:
	IN	48H
	ORA	A	
	JP	DTWAIT
	IN	43H
	STAX	D
	INX	D
	IN	40H
	ANI	01H
	JNZ	DTWAIT

; check for lost data
	CALL	DELAY
	IN	40H
	ANI	04H
	JZ	DTOK

	MVI	A,'D'
	JMP	DTFIN

DTOK:	MVI	A,0
DTFIN:
	RET


; seek to track zero
; NZ on seek error 
FDCTRK0:
	MVI	A,07H
	CALL	FDCCMD
	CALL	FDCWAIT
	IN	40H
	ANI	10H	; mask in seek error
	RET


; send a command to the FDC
; command in A, destroyed on exit
; does appropriate delays
FDCCMD:
	OUT	40H
	MVI	A,30	; 3.5us
FDCDEL: DCR	A	; 3us
	JNZ	FDCDEL  ; 6us
	RET
	

; wait for the busy flag to clear
FDCWAIT:
	PUSH	PSW
WAITLP:
	IN	40H
	ANI	01H
	JNZ	WAITLP
	POP	PSW
	RET


; select the drive, side, density according to A
; destroys BC
SETDRIVE:
	OUT	48H
	CALL	DELAY
	RET


DELAY:
	MVI	C,40H
DL2:
	MVI	B,0
DL1:
	XTHL
	XTHL
	XTHL
	XTHL
	XTHL
	XTHL
	XTHL
	XTHL
	DCR	B
	JNZ	DL1
	DCR	C
	JNZ	DL2
	RET	 

OLDSP:	DS	2	;ENTRY SP VALUE FROM CCP
;
;	STACK AREA
	DS	64	;RESERVE 32 LEVEL STACK
STKTOP:
;
	END
