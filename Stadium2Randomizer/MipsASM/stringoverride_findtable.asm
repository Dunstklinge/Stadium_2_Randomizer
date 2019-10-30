SORH_TABLE_TABLE_ADDR  	equ 0xDFFFFFFF
	
	
//takes:	a0 = text table
//touches: 	t0, t1, t2, t3, t4, v0, v1
//returns:	v0 = pointer to matching table table row, or null if not found, v1 = pointer to target table
//			also leaves the offset in t1
string_find_alt_table:
	lui 	t4, SORH_TABLE_TABLE_ADDR 	   >> 16 & 0xFFFF
	ori 	t4, t4, SORH_TABLE_TABLE_ADDR & 0xFFFF
	or		v0, t4, r0
	and		t1, r0, r0
	ori		t1, t1, 0xFF
string_fat_loop:
	//v0 = table it, t1 = end condition
	lw		t2, 0(v0) 			//load table and offsets (hiword = table, lo = offset)
	nop
	srl		t3, t2, 16			//table table that this entry applies to
	beq		t3, a0, string_fat_found	//text table entry found
	nop
	bne		t3, t1, string_fat_loop    	//loop again, unless end marker was found
	addiu	v0, v0, 8			//it++
//not found:
	jr		ra
	or	 	v0, r0, r0
string_fat_found:
	andi	t1, t2, 0xFFFF		//v1 = offset of table from table table start
	jr		ra
	addu	v1, t4, t1			//v1 = pointer to table
