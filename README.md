A Randomizer for Pokemon Stadium 2.

![alt text](https://github.com/Dunstklinge/Stadium2Randomizer/blob/master/UISnapshot.jpg)

Current limitations:
	- does not support roms other than US versions ([U] in title, region code 'E'). The custom asm-hacks are missing a couple offsets to work with eu.
	- custom trainers replace every instance of a trainer. That is because all instances of a trainer share the same name string,
	  so replacing individual instances of trainers would change the name of all other instances of that trainer. Need to hack in a new name table to fix this.
	- rental items must be battle items; other items are not visible in the list.
	- specific trainers do not get specific pokemon (like misty getting water pokemon).
	- i actually have no idea if this would work on an actual cardrige. 
	- only 3 custom trainers exist. i wanted to make more, but it turns out coming up with 40 lines of dialog isnt that easy.

Note: this was written with VC++17 and requires the appropriate redistributable because of that.