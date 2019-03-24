FACE2_CUSTOM_TABLE_ADDR  	equ 0xDFFFFFFF
FACE2_ORIGINAL_N			equ 67

FACE2_RESTORE_PTR			equ 0x0440
FACE2_RESTORE_LENGTH		equ 0x6030
//FACE2_RESTORE_PTR			equ 0x00136df0
//FACE2_RESTORE_LENGTH		equ 0x6030

//ram table address is at 0x80097120 -> (dyn?0x801C2A70)+0x28 -> (dyn?0x801C7000) = ram table address (there is no register, it is taken just like that in the called original func)
//a1 is face index



//NOTE:
//this is attempt two and is INLINE at the start of the function 8004E0F0.
//we saved original ra in t9

	or t5, ra, r0 	//t5 is return address
	or ra, t9, r0	//restore ra
	
	lui t4, 0x8009
	lw t4, 0x7120(t4)
	nop 
	lw t4, 0x28(t4)			// t4 = ram table ptr
	nop
	
	subiu t1, a1, FACE2_ORIGINAL_N
	bltz t1, @restore_original
	
	lui t0, FACE2_CUSTOM_TABLE_ADDR    >> 16 & 0xFFFF	//t0 is custom table iterator
	ori t0, t0, FACE2_CUSTOM_TABLE_ADDR& 0xFFFF
	
	sll t1, t1, 3 			// *8 as each entry has 2 words
	addu t1, t1, t0 		// t1 = address to copy in custom table
	or a1, r0, r0			 // set index to 0 so he loads the first table entry; we will modify that one
	
	lw t2, 0x00(t1)			// t2 = new ptr 
	lw t3, 0x04(t1)			// t3 = new length
	sw t2, 0x10(t4)
	sw t3, 0x14(t4)
	
	b @exit_to_orig
	nop
	
@restore_original:
	or t0, r0, r0
	li t0, FACE2_RESTORE_PTR
	sw t0, 0x10(t4)
	li t1, FACE2_RESTORE_LENGTH
	sw t1, 0x14(t4)
	
@exit_to_orig:
	
	addiu sp, sp, -0x40		//original ops that we replaced
	lui v0, 0x8009
	lw v0, 0x7120(v0)
	sw s0, 0x0018(sp)
	or s0,a0,r0
	//sw ra, 0x1C(sp)
	//sw a2, 0x48(sp)
	//or a3, a1, r0
	//sw r0, 0x3C(sp)
	//beqz v0, 0x8004E230
	
	
	jr t5
	nop