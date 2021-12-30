## Pokemon Stadium 2 Randomizer with so many options that it becomes inconvenient again.

### IMPORTANT NOTE FOR PROJECT64 USERS:
Some Settings cause project 64 with default settings to freeze randomly.
Those freezes usually happen during the intro screen, when selecting a cup, when viewing rentals, or on certain moves.
Appears to still be happening with PJ64 3.0
To fix these crashes, do the following:

1. Launch a pokemon stadium 2 rom
2. Go to Options -> Settings
3. Expand Config: POKEMON STADIUM 2 (this option only appears after you launched the ROM)
4. Go to Recompiler
5. change "CPU Core Style" from "Recompiler" to "Interpreter".
6. click apply
7. close pj64 and reopen it

That should fix the freezes.

Mupen didnt seem to have this problem last year, didnt check on the newer versions yet.
But im going to go out on a whim and guess that they will have the same problem one day.

### Description

Heres a screenshot of the UI to see what it does:

![Main Dialog](https://github.com/Dunstklinge/Stadium2Randomizer/blob/master/UISnapshot.jpg)
![Distribtion Dialog](https://github.com/Dunstklinge/Stadium2Randomizer/blob/master/UISnapshot2.jpg)

As you can see, there are *way* too many bloddy settings at this point, so if you dont want to study this randomizer like
a bloody university course, i'd recommend just loading one of the templates.
That being said, a word of warning: turns out, if you dont give the npcs a decent advantage, their teams of 
3 pokemon picked out of 6 tend to be a lot weaker than your handcrafted team of 3 picked out of 250, so try to let them have 
at least *some* advantage, or the game is gonna become kinda easy.


Uses MFC, so probably windows only. Or maybe it works in Wine, i don't know.


Current limitations:

- does not support roms other than US versions ([U] in title, region code 'E'). The custom asm-hacks are missing a couple offsets to work with eu.
	
- rental items must be battle items; other items are not visible in the list.

- i wish i could change the game rules to bring all 6 pokemon at one point
	
- specific trainers do not get specific pokemon (like misty getting water pokemon).
	
- i actually have no idea if this would work on an actual cardrige. 
	
- only 2 custom trainers exist. i wanted to make more, but it turns out coming up with 40 lines of dialog isnt that easy.

Big Thanks to the guy who made [this repo](https://github.com/pret/pokestadium/tree/master/stadiumgs). Half of the Information i needed came from there.


Note: this was written with VC++19 and requires the appropriate redistributable because of that. (yes, i updated a little)
