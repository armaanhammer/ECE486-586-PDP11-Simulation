; rotates all bits of destination to the right by 1
; place. LSB gets loaded into the C bit and C bits gets
; shifted to the MSB. 

A:	.WORD 000001.

START:
		MOV A, R1
		MOV B, R2
		CLC
		ROR	R1 ; C = 1
		HALT
		.END START