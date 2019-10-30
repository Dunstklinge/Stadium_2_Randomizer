

//original function:
//a0 = text table index
//a1 = table entry nr
//8004C874		ADDIU	SP, SP, -0x18
//8004C878		SW		RA, 0x0014(SP)
//8004C87C		OR		A2, A0, R0
//8004C880		OR		A3, A1, R0 
//8004C884		OR		A1, R0, R0
//8004C888		JAL		0x8004C7A0
//8004C88C		OR		A0, R0, R0
//8004C890		LW		RA, 0x0014(SP)
//8004C894		ADDIU	SP, SP, 0x18
//8004C898		JR		RA
//8004C89C		NOP

//we replace the entire function (including the sp instruction) using a non-linking jump
//this means ra is still the original caller, so please no touching

	addiu	sp, sp, -0x10
	sw		ra, 0x4(sp)
//first, find string table based on a0
	jal		string_find_alt_table
	nop
	lw		ra, 0x4(sp)
	addiu	sp, sp, 0x10
	bnez	v0, tt_found
	nop

nothing_found:
	//nothing found, delegate to original
	or		a2, a0, r0
	or		a3, a1, r0
	or		a1, r0, r0
	li		t0, 0x8004C7A0
	jr		t0					
	or 		a0, r0, r0


tt_found:
//find our loaded buffer using this rule: header = [[80097120] + 8] + (nr << 4) + 0x10, [header + 4] + [header + 8] - tableSize
	lui		t0, 0x8009
	lw		t0, 0x7120(t0)
	lw		t4, 0x4(v0)			//custom table size
	beqz	t0, nothing_found
	nop
	lw		t0, 0x8(t0)
	sll		t2, a0, 4
	beqz	t0, nothing_found
	addu	t0, t0, t2
	lw		t3, 0x18(t0)
	lw		t2, 0x14(t0)
	beqz	t3, nothing_found
	nop
	addu	t0, t2, t3
	subu	t0, t0, t4
	
	or		v1, t0, r0
	lw 		t1, 0(v1)			//load min max (and 16 bits after that)
	nop
	srl		t1, t1, 16			//min/max (hi/lo byte)
	srl		t2, t1, 8			//start
	subu	t3, a1, t2			//t3 = id - start (nth element)
	bltz	t3, nothing_found	//abort if id < start
	andi	t1, t1, 0x00FF		//end
	subu	at, a1, t1			
	bgez	at, nothing_found	//abort if id >= end
	
	
//t3-th element (16 bit offset) starting from v1 + 4, ergo v1 + t3*4 + 4
	sll		t1, t3, 2			
	addu	t1, v1, t1			//t2 = v1 + 4*t1
	lw		t1, 4(t1)			//load offset to string
	nop
	jr		ra
	addu 	v0, v1, t1			//string address
	