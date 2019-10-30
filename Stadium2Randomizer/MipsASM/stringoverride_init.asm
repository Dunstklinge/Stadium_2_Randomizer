
//we replace:
//JAL 		0x08004C3EC
//SW		R0, 0x001C(SP)		<--- this one goes into the new delay slot, so we dont need to repeat it
//BEQZ		V0, 0x8004C4F0
//LW		V1, 0x001C(SP)
//(db 0c 01 30 FB 	AF A0 00 1C		10 40 00 02		8F A3 00 1C)
//our regs here are: a0 = some struct, a1 = table index (so that a0 + 0x10 + (a1 << 4) is table entry for that text table)

	addiu	sp, sp, -0x20
								//18 will be pointer to ram table
								//14 will be v0 to return
	sw		s0, 0x10(sp)		//10 is old s0
	sw		s1, 0xC(sp)			//C is old s1
	or		s0, r0, r0
	or		s1, r0, r0
	sw		ra, 0x8(sp)			//8 is ra
	sw		a0, 0x4(sp)			//4 is a0
								
	jal 	string_find_alt_table
	or		a0, a1, r0
	beqz	v0, original
	lw		a0, 0x4(sp)
	sw		v1, 0x18(sp)
//increase size so he overallocates
	lw		t0, 0x4(v0)			//t0 = table size
	sll		t1, a1, 4
	addu	s1, a0, t1
	lw		s0, 0x14(s1)		//+10 to get pointer, then second element is table size
	nop
	addu	t0, s0, t0
	sw		t0, 0x14(s1)
	
original:
	lui		at, 0x8004
	ori		at, 0xC3EC
	jalr	at 
	nop
	beqz	s0, main_end
	nop
//restore original size and do DMA
	sw		v0, 0x14(sp)		//save v0 for later
	lw		t0, 0x14(s1)		//t0 = new size
	//sw		s0, 0x14(s1)		//s0 = old size (restore original)
	addu	a0, v0, s0			//a0 = ram buffer after target
	lw		a1, 0x18(sp)		//a1 = rom table pointer
	lui		t1, 0x03FF
	ori		t1, 0xFFFF
	and		a1, a1, t1			//mask out upper byte since a2 is a B0000000 address
	subu	a2, t0, s0			//a2 = rom table size
	addu	a2, a1, a2			//a2 = rom table end
	or		a3, r0, r0			//a3 is usually 0 here, dont know what it is
	lui		at, 0x8000			//call stadium DMA function
	ori		at, at, 0x3F74
	jalr	at
	nop
//restore original return value
	lw		v0, 0x14(sp)
	
main_end:
	lw		s0, 0x10(sp)
	lw		s1, 0xC(sp)
	lw		ra, 0x8(sp)
	addiu	sp, sp, 0x20 
//we have to emulate the beqz; we do this by modifying ra
	beqz	v0, main_end_skip_mod
	nop
	lui		ra, 0x8004
	ori		ra, 0xC4F0
main_end_skip_mod:
	jr		ra
	lw		v1, 0x001C(SP)
	