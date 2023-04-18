#include "bits/stdc++.h"
#include "game.h"

using namespace std;

game *g = nullptr;

int menu();
int play();
int help();
int exit();

int main(int argc, char *argv[]){
	g = new game();
	
	g -> init();

	int type = MENU;

	while(type != EXIT){
		switch(type){
			case MENU:
				type = menu();
				
				break;
			case PLAY:
				type = play();

				break;
			case HELP:
				type = help();

				break;
			case EXIT:
				type = exit();

				break;
			default:
				break;
		}
	}

	g -> clean();

	delete g;

	return 0;
}

int menu(){
	g -> render_menu();

	while(1){
		int state = g -> update_mouse();

		switch(state){
			case MENU:
				g -> render_menu();
				break;
			case PLAY:
				return PLAY;
			case HELP:
				return HELP;
			case EXIT:
				return EXIT;
			default:
				break;			
		}
	}

	return -1;
}

int play(){
	return g -> start();
}

int help(){
	g -> render_guide();

	while(1){
		SDL_PumpEvents();

		if (game::keyboard_state[SDL_SCANCODE_RETURN]){
			break;
		}
	}

	return MENU;
}

int exit(){
	printf("exit\n");

	return EXIT;
}