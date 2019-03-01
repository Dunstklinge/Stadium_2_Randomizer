.n64

.create "iteminjection.bin", 0x8000F800 //dont actually know where
	.include "iteminjection.asm", "UTF-8"
.close

.create "rentalinjection.bin", 0x8000F800 //dont actually know where
	.include "rentalinjection.asm", "UTF-8"
.close

.create "shortest.bin", 0x8000F800 //dont actually know where
	.include "shortest.asm", "UTF-8"
.close
