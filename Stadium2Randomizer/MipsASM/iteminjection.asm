ITM_TRAINER_FIGHT_ADDR equ 0x801451D0
ITM_CUSTOM_TABLE_ADDR  equ 0xDFFFFFFF
ITM_INFO_ENTRY_SIZE	   equ 0x8
ITM_ITEMADD_CALL_ADDR  equ 0x8017B4D0	//varies, so cant be used
ITM_ITEMADD_CALL_RAOFF equ (0x8017B4D0 - 0x8017B5B0)

	//s0 and s1 are set from the caller and contain parameters for a0 and a2 respectively
	//a1 must become our item
	addiu 	sp, sp, -0x10
	sw		ra, 0xC(sp)
	sw 		s2, 0x8(sp)
	sw 		s3, 0x4(sp)
	sw 		s4, 0x0(sp)

	lui 	t1, ITM_TRAINER_FIGHT_ADDR 	   >> 16 & 0xFFFF
	ori 	t1, t1, ITM_TRAINER_FIGHT_ADDR & 0xFFFF
	lhu 	t1, (t1) 			 			  //t1 is npc we fight
	lui 	t0, ITM_CUSTOM_TABLE_ADDR    >> 16 & 0xFFFF	
	ori 	t0, t0, ITM_CUSTOM_TABLE_ADDR& 0xFFFF //t0 is custom table iterator
headerLoop:
	//loop through rental table until a fitting rule is found
	lw 		t2, (t0) 					//t2 is upper border (starts high, goes to 0)
	nop
	subu 	at, t1, t2
	bltz 	at, headerLoop				//while toFight(t1) < border(t2)
	addiu 	t0, t0, ITM_INFO_ENTRY_SIZE
	subiu 	t0, t0, ITM_INFO_ENTRY_SIZE
	
	lw 		s2, 0x4(t0) 				//s2 = itemPtr
	nop
	
	//note that we have to load words cause lhu and lb dont work on rom
loadLoop:
	lw 		s3, 0(s2)					//s3 = 4 items from list 
	addiu 	s2, s2, 4					//s2 (itemPtr) to next
	ori 	s4, r0, 24					//s4 = right shifts to get to current byte in s3

shiftLoop:	
	srlv 	t1, s3, s4					
	andi 	t1, t1, 0xFF				//t1 = current item
	beq 	t1, r0, end					//exitwhen t1 == 0
	
	//do original call
	lw		at, 0xC(sp)					//load ra to find orig function
	or 		a0, s0, r0
	or 		a1, t1, r0
	addiu	at, at, ITM_ITEMADD_CALL_RAOFF
	jalr	at
	or 		a2, s1, r0
	
	beq		s4, 0, loadLoop				//was last byte
	addiu 	s4, s4, -8
	b		shiftLoop					//next byte
	nop
	
end:
	
	lw 		s4, 0x0(sp)
	lw 		s3, 0x4(sp)
	lw 		s2, 0x8(sp)
	lw		ra, 0xC(sp)
	addiu 	sp, sp, 0x10
	
	jr ra
	nop