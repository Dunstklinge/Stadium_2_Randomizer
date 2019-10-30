## A Randomizer for Pokemon Stadium 2.

Uses MFC, so probably windows only.

Heres a screenshot of the UI to see what it does:

![Main Dialog](https://github.com/Dunstklinge/Stadium2Randomizer/blob/master/UISnapshot.jpg)
![Distribtion Dialog](https://github.com/Dunstklinge/Stadium2Randomizer/blob/master/UISnapshot2.jpg)

Current limitations:

- does not support roms other than US versions ([U] in title, region code 'E'). The custom asm-hacks are missing a couple offsets to work with eu.
	
- rental items must be battle items; other items are not visible in the list.
	
- specific trainers do not get specific pokemon (like misty getting water pokemon).
	
- i actually have no idea if this would work on an actual cardrige. 
	
- only 2 custom trainers exist. i wanted to make more, but it turns out coming up with 40 lines of dialog isnt that easy.

Big Thanks to the guy who made [this repo](https://github.com/pret/pokestadium/tree/master/stadiumgs). Half of the Information i needed came from there.


Note: this was written with VC++17 and requires the appropriate redistributable because of that.
