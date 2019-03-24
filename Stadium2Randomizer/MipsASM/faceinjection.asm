FACE_CUSTOM_TABLE_ADDR  	equ 0xDFFFFFFF
FACE_ORIGINAL_CALL	   	equ 0x8004E0F0
FACE_ORIGINAL_N			equ 67

//ram table address is at 0x80097120 -> (dyn?0x801C2A70)+0x28 -> (dyn?0x801C7000) = ram table address (there is no register, it is taken just like that in the called original func)
//a1 is face index

//NOTE:
//the call site of this function replaced one call-op with 3 ops (2 loads and 1 call-op).
//however, the function jumps to the third instruction after the original call on occasion;
//this instruction becomes the delay slot of our modified call.
//thankfully, the instruction in question has no impact on our injected function and can thus
//be left intact to not break the loop; however, that means we have to execute the old delay slot 
//manually as the first instruction and only have to execute one of the two replaced instructions at the end.

	or a0, r0, r0			//this used to be the delay slot.
	
	

	lui t0, FACE_CUSTOM_TABLE_ADDR    >> 16 & 0xFFFF	//t0 is custom table iterator
	ori t0, t0, FACE_CUSTOM_TABLE_ADDR& 0xFFFF
	subiu t1, a1, FACE_ORIGINAL_N
	bltz t1, @originalCall
	
	sll t1, t1, 3 			// *8 as each entry has 2 words
	addu t1, t1, t0 		// t1 = address to copy in custom table
	or a1, r0, r0 			// set index to 0 so he loads the first table entry; we will modify that one
	
	lui t4, 0x8009
	lw t4, 0x7120(t4)
	nop 
	lw t4, 0x28(t4)			// t4 = ram table ptr
	
	addiu sp, sp, -0x10
		
	lw t2, 0x10(t4)			// t2 = original ptr
	lw t3, 0x14(t4)			// t3 = original length
	sw t4, 0x0C(sp)			// save original table ram ptr
	sw t2, 0x08(sp)			// save original ptr
	sw t3, 0x04(sp)			// save original size
	sw ra, 0x00(sp)			// save return address 
	
	lw t2, 0x00(t1)			// t2 = new ptr 
	lw t3, 0x04(t1)			// t3 = new length
	sw t2, 0x10(t4)
	sw t3, 0x14(t4)
	
	li t0, FACE_ORIGINAL_CALL
	jalr t0
	nop
	
	lw t4, 0x0C(sp)
	lw t1, 0x08(sp)
	lw t2, 0x04(sp)
	lw ra, 0x00(sp)
	
	sw t1, 0x10(t4)			// restore original ptr 
	sw t2, 0x14(t4)			// restore original size 
	
	addiu sp, sp, 0x10
	b @epiloge
	nop
@originalCall:
	
	addiu sp, sp, -4
	sw ra, 0x00(sp)
	
	li at, FACE_ORIGINAL_CALL
	jalr at
	nop
	
	lw ra, 0x00(sp)
	addiu sp, sp, 4
	
@epiloge:
	sw v0, 0x001C(s4)		//replaced op 1
	//addiu s1, s1, 0x0001	//replaced op 2, doesnt have to be replaced (look at top)
	
	jr ra 
	nop