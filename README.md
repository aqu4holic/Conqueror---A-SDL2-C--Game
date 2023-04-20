# Conqueror - A-SDL2-C-Game

## Introduction

A game project made with SDL2 C++.

Game's demo: https://youtu.be/oEzOYZ4L6Yg

## Project uses:
- SDL2 devel 2.0.20 mingw.
- SDL2 image 2.0.5 mingw.
- SDL2 mixer 2.0.4 mingw.
- SDL2 ttf devel 2.0.18 mingw.

## How to compile and play

- **Windows 64bit**
	- Requirements:
		- MinGW-64.
	- Clone this repository to your computer.
	- Open command line inside the cloned folder.
	- Build `main.exe` wtih this command:
	```
	g++ -DBlackWhite -O2 -I src/include -L src/lib src/*.cpp -o main.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
	```

## Contents

### How to play

Your kingdom you once ruled has got taken by monsters. You have to flee, with only a gun in your hand.
Can you fight back to conquer your kingdom once more?

Try to eliminate all enemies and prepare for the fight with the MEGA WITCH. Demolish him to take back your kingdom!

**Controls:**

- W, S, Q, E : Move around

- A, D : Look around

- F : Open door

- Space : Shoot

- M : Toggle mini map

- T : Toggle background music

- Y : Toggle sound effect
