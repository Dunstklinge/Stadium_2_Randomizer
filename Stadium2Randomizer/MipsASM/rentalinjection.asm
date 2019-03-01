//we are hooking this part: 
//801883D0: jal 0x80071B20
//801883D4: or a1, v0, r0
//801883D8: addiu at, s0, 0x7FFF
//where a0 is start of rental table in ram, a1 is determined index to load, a2 is 0(unused?), a3 is unknown struct(leftover i guess)
//the function takes the tableptr and length at index a1, loads them into ram using pi and returns the address of the loaded part in v0 and v1.
//i guess the easiest course of action would be to manipulate the tableptrs in ram before delegating to the original function and then maybe restoring them.

//801451D0 contains trainer group to fight
//we can deduce the rental to be used from this

RNT_TRAINER_FIGHT_ADDR 	equ 0x801451D0
RNT_CUSTOM_TABLE_ADDR  	equ 0xDFFFFFFF
RNT_ORIGINAL_CALL	   	equ 0x80071B20
RNT_ORIGINAL_CALL_RAOFF equ (0x80071B20 - 0x801883E0)
RNT_INFO_ENTRY_SIZE	   	equ 0xC


	lui t1, RNT_TRAINER_FIGHT_ADDR 	   >> 16 & 0xFFFF
	ori t1, t1, RNT_TRAINER_FIGHT_ADDR & 0xFFFF
	lhu t1, (t1) 			 			//t1 is npc we fight
	lui t0, RNT_CUSTOM_TABLE_ADDR    >> 16 & 0xFFFF	//t0 is custom table iterator
	ori t0, t0, RNT_CUSTOM_TABLE_ADDR& 0xFFFF
loop:
	//loop through rental table until a fitting rule is found
	lw t2, (t0) 			//t2 is upper border (starts high, goes to 0)
	nop
	subu at, t1, t2
	bltz at, loop			//while toFight(t1) < border(t2)
	addiu t0, t0, RNT_INFO_ENTRY_SIZE
	subiu t0, t0, RNT_INFO_ENTRY_SIZE
	
	lw t1, 0x4(t0) 			//t1 = rentalOffest
	lw t2, 0x8(t0) 			//t2 = rentalLength
	//if rentalOffset == -1 then this rule should be ignored
	addiu t1, t1, 1
	beq t1, r0, nochange 
	addiu t1, t1, -1
	
	//calculate rental table entry to overwrite (t4)
	sll t3, a1, 4 			//a1*16 as every line is made of 4 words (+10 is in offset)
	addu t4, a0, t3
	
	//temporarily replace offest and length with the one from the table
	addiu sp, sp, -0x10		//save old s0/s1/ra and t4(rental table offset)
	sw s0, 0x0(sp)
	sw s1, 0x4(sp)
	sw t4, 0x8(sp)
	sw ra, 0xC(sp)
	
	lw s0, 0x10(t4) 		//save old values
	lw s1, 0x14(t4)
	sw t1, 0x10(t4) 		//overwrite with new values
	sw t2, 0x14(t4)
	
	li t0, RNT_ORIGINAL_CALL
	jalr t0					//call original function
	nop 
	
	lw t4, 0x08(sp)			//restore rental table pointer
	sw s0, 0x10(t4)			//restore save old rental table value
	sw s1, 0x14(t4)
	
	lw s0, 0x0(sp)			//restore old s0/s1/ra values
	lw s1, 0x4(sp)
	lw ra, 0x0C(sp)
	addiu sp, sp, 0x10
	
	b epiloge
	nop
nochange:
	
	addiu sp, sp, -4
	sw ra, 0x0(sp)
	
	li t0, RNT_ORIGINAL_CALL
	jalr t0					
	nop
	
	lw ra, 0x0(sp)
	addiu sp, sp, 4
	
epiloge: //overwritten ops
	addiu at, s0, 0x7FFF 
	sw v0, 0x2179(at)
	li t0, 0x80002B24					 //jal 0x80002B24
	jr t0							     //delegate cause we're done
	nop
	