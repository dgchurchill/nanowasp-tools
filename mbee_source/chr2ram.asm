;
; DGC 2011-06-16:  This program appears to copy the character ROM data
;                  to memory location 300H.  I think the idea was to
;                  run this program and then run the CP/M command that
;                  saves a chunk of memory to disk.
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

	MVI	A,1
	OUT	0BH

	MVI	A,0CH
	OUT	0CH
	MVI	A,0
	OUT	0DH 

	MVI	A,50
DEL1:
	LXI	B,0FFFFH
DB 10H, 0FEH
	DCR	A
	JNZ	DEL1
	
	LXI     H,0F000H
	LXI     D,300H
	LXI     B,800H
DB 0EDH, 0B0H		

	MVI	A,0CH
	OUT	0CH
	MVI	A,20H
	OUT	0DH

	MVI	A,50
DEL2:
	LXI	B,0FFFFH
DB 10H, 0FEH
	DCR	A
	JNZ	DEL2

	LXI	H,0F000H
	LXI	B,800H
DB 0EDH, 0B0H

	MVI	A,0
	OUT 	0BH

	LHLD	OLDSP
	SPHL
;	STACK POINTER CONTAINS CCP'S STACK LOCATION
	RET		;TO THE CCP

OLDSP:	DS	2	;ENTRY SP VALUE FROM CCP
;
;	STACK AREA
	DS	64	;RESERVE 32 LEVEL STACK
STKTOP:
;
	END
