#pragma once

#include "bits/stdc++.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_ttf.h"
#include "world_map.h"
#include "raycasting.h"
#include "sdl2utils.h"

using namespace std;

// length of a wall or cell in game units
const int TILE_SIZE = 128;

const int TEXTURE_SIZE = 128; // length of wall textures in pixels
const int MINIMAP_SCALE = 8;
const int MINIMAP_Y = 0; // position of minimap from top of the screen
const int DESIRED_FPS = 120;
const int UPDATE_INTERVAL = 1000 / DESIRED_FPS;
const double TWO_PI = 2 * M_PI;

enum sprite_type{
	SPRITE_TYPE_DEFAULT,
	SPRITE_TYPE_TREE_1,
	SPRITE_TYPE_TREE_2,
	SPRITE_TYPE_ZOMBIE,
	SPRITE_TYPE_SKELETON,
	SPRITE_TYPE_ROBOT,
	SPRITE_TYPE_FROGMAN,
	SPRITE_TYPE_HEROINE,
	SPRITE_TYPE_DRUID,
	SPRITE_TYPE_PROJECTILE,
	SPRITE_TYPE_PROJECTILE_SPLASH,
};

enum{
	MENU,
	PLAY,
	HELP,
	EXIT,
};

class game{
public:
	game();
	~game();

	void init();
	void menu_init();

	void reset();
	int on_quit(){
		is_running = 0;

		return EXIT;
	}
	bool running(){
		return is_running;
	}

	void clean();
	
	/*
		mouse in menu manager
	*/

	int play_button_state = 0;
	int help_button_state = 0;
	int exit_button_state = 0;

	SDL_Rect des_play, des_help, des_exit;

	bool inside_button(int x, int y, SDL_Rect src);

	int update_mouse();

	void render_menu();
	void render_guide();

	static SDL_Window *window;
	static SDL_Renderer *renderer;
	static SDL_Texture *screen_texture;
	static SDL_Surface *screen_surface;
	
	static const Uint8 *keyboard_state;

	int start();

	void draw();
	void fill_rect(SDL_Rect *rec, int r, int g, int b);
	void draw_line(int start_x, int start_y, int end_x, int end_y, int r, int g, int b, int alpha = SDL_ALPHA_OPAQUE);

	void on_key_up(SDL_Event *event);
	int run();
	void update(double time_elapsed);
	
	void draw_floor(vector <ray_hit> &ray_hits);
	void draw_ceiling(vector <ray_hit> &ray_hits);
	void draw_weapon();
	void draw_minimap();
	void draw_minimap_sprites();
	void draw_player();
	void draw_ray(double ray_x, double ray_y);
	void draw_rays(vector <ray_hit> &ray_hits);
	void draw_world();
	
	void update_player(double time_elapsed);
	void update_projectiles(double time_elapsed);

	double wall_screen_y(ray_hit &Ray_hit, double wall_height);
	SDL_Rect strip_screen_rect(ray_hit &Ray_hit, double wall_height);
	void draw_wall_strip(ray_hit &Ray_hit, surface_texture &img, double tex_x, double tex_y, int wall_screen_height);

	void draw_world(vector <ray_hit> &ray_hits);
	void raycast_world(vector <ray_hit> &ray_hits);

	bool is_wall_cell(int wall_x, int wall_y);
	bool player_in_wall(double player_x, double player_y);
	
	SDL_Rect find_sprite_screen_position(sprite &s);
	void add_sprite_at(int sprite_id, int cell_x, int cell_y);
	void add_projectile(int texture_id, int x, int y, int size, double rotation);

	void toggle_door_pressed();
	void toggle_door(int cell_x, int cell_y);

	Uint32 fog_pixel(Uint32 pixel, double distance);
	void fog_wall_strip(SDL_Rect *dstrect, double distance);
private:
	bool is_running;
	int strip_width, ray_count;
	int fov_degrees;
	double fov_radians, view_dis;
	double *strip_angles;

	raycaster raycaster3d;
	vector <int> ceiling_grid;
	vector <int> ground_walls;

	int frame_skip;

	sprite player;
	surface_texture walls_image, walls_image_dark, gates_image, gates_open_image;
	surface_texture gun_image, crosshair_image;

	vector <sprite> sprites;
	queue <sprite> projectile_queue;

	bool draw_minimap_on = 1;
	bool sfx = 1;
	bool bgm = 0;

	bitmap ceiling_bitmap;
	Uint32 ceiling_color;

	Mix_Chunk *projectile_fire_sound;
	Mix_Chunk *projectile_impact_sound;
	Mix_Chunk *door_open_sound;
	Mix_Chunk *door_close_sound;
	Mix_Music *bgm_music;

	map <int, surface_texture> sprite_textures;
	vector <bitmap> floor_ceiling_bitmaps;

	int highest_ceiling_level = 1;

	int ray_hits_count;
	int doors[MAP_WIDTH * MAP_HEIGHT];
	vector <thin_wall*> thin_walls;
};