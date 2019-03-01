	//d9630, loaded in 8017B5A0 when executed
	//note: jal 0x80003F74 native void ipDma(a0: ramCopyTo, a1: ROMstart, a2: ROMend, a3: ? (0))x
	
	addiu 	sp, sp, -8-128
	//copy item list to stack
	or 		a0, sp, r0
	lui 	a1, 0x1258
	ori 	a2, a1, 0x9464 //saves a lui for a2
	ori 	a1, a1, 0x9400
	or 		a3, r0, r0
	jal 0x0003F74
	//save s2 and s3
	sw 		s2, 128+0(sp)
	sw 		s3, 128+4(sp)
	//use them for the loop
	or 		s3, sp, r0		//s3 = bufferPtr
	lbu 	s2, 0(s3)		//s2 = bufferValue
	
	//code duplication because delay slots
	or 		a0, s0, r0
	or 		a1, s2, r0
label:
	jal 0x16019C0
	or 		a2, s1, r0

	addiu 	s3, s3, 1
	lbu 	s2,0(s3)
	or 		a0, s0, r0
	bne 	s2, r0, label
	or 		a1, s2, r0

	//recover s2 and s3
	lw 		s3, 128+4(sp)
	lw 		s2, 128+0(sp)
	addiu 	sp, sp, 8+128