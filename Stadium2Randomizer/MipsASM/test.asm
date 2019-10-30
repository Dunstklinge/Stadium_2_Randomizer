.n64

.create "iteminjection.bin", 0x80000000
	.include "iteminjection.asm", "UTF-8"
.close

.create "rentalinjection.bin", 0x80000000 
	.include "rentalinjection.asm", "UTF-8"
.close

.create "shortest.bin", 0x80000000 
	.include "shortest.asm", "UTF-8"
.close

.create "faceinjection.bin", 0x80000000 
	.include "faceinjection.asm", "UTF-8"
.close

.create "faceinjection2.bin", 0x80000000 
	.include "faceinjection2.asm", "UTF-8"
.close


.create "stringoverride_findtable.bin", 0x8000CCCC 		//so we can find it in the bin file
	.include "stringoverride_findtable.asm", "UTF-8"
.close

.create "stringoverride_init.bin", 0x80000000 
	.include "stringoverride_init.asm", "UTF-8"
.close

.create "stringoverride_getstring.bin", 0x80000000 
	.include "stringoverride_getstring.asm", "UTF-8"
.close

