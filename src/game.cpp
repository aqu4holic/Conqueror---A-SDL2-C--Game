#include "game.h"
#include "world_map.h"

SDL_Renderer *game::renderer = nullptr;
SDL_Window *game::window = nullptr;
SDL_Texture *game::screen_texture = nullptr;
SDL_Surface *game::screen_surface = nullptr;

const Uint8 *game::keyboard_state = SDL_GetKeyboardState(NULL);

// append game map to a vector
void array2d_to_vector(int a[MAP_HEIGHT][MAP_WIDTH], vector <int> &res){
	res.clear();

	for (int i = 0; i < MAP_HEIGHT; ++i){
		for (int j = 0; j < MAP_WIDTH; ++j){
			res.push_back(a[i][j]);
		}
	}
}

// for sprites erase
bool needs_clean_up(const sprite &s){
	return s.cleanup;
}

// determine which sprite can be killed
bool is_killable_sprite(int texture_id){
	return (3 <= texture_id && texture_id <= 8);
}

game::game() {}
game::~game() {}

// init SDL
void game::init(){
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		printf("SDL Failed Initializing...");

		return;
	}

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());

		return;
	}

	if (TTF_Init() == -1){
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());

		return;
	}

	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1){
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());

		return;    
	}

	window = SDL_CreateWindow("Conqueror", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);
	screen_surface = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0, 0, 0, 0);

	frame_skip = 0;
	is_running = 0;
	
	draw_minimap_on = 1;

	ray_hits_count = 0;

	menu_init();

	reset();
}

// set menu variables' value
void game::menu_init(){
	des_play.w = 160;
	des_play.h = 100;
	des_play.x = (WINDOW_WIDTH - des_play.w) / 2;
	des_play.y = 250;

	des_help.w = 160;
	des_help.h = 100;
	des_help.x = (WINDOW_WIDTH - des_help.w) / 2;
	des_help.y = des_play.y + des_play.h + (WINDOW_HEIGHT / 15);

	des_exit.w = 160;
	des_exit.h = 100;
	des_exit.x = (WINDOW_WIDTH -des_exit.w) / 2;
	des_exit.y = des_help.y + des_help.h + (WINDOW_HEIGHT / 15);
}

// check if (x, y) is inside a button
bool game::inside_button(int x, int y, SDL_Rect src){
	return (src.x <= x && x <= src.x + src.w) && (src.y <= y && y <= src.y + src.h);
}

// update mouse event in menu
int game::update_mouse(){
	int mousex, mousey;

	SDL_PumpEvents();

	Uint32 button = SDL_GetMouseState(&mousex, &mousey);

	bool changed = 0;
	int play = inside_button(mousex, mousey, des_play);
	int help = inside_button(mousex, mousey, des_help);
	int exit = inside_button(mousex, mousey, des_exit);

	if (play != play_button_state || help != help_button_state || exit != exit_button_state){
		changed = 1;
	}

	play_button_state = play;
	help_button_state = help;
	exit_button_state = exit;

	if (play && (button & SDL_BUTTON_LMASK) != 0){
		return PLAY;
	}
	
	if (help && (button & SDL_BUTTON_LMASK) != 0){
		return HELP;
	}
	
	if (exit && (button & SDL_BUTTON_LMASK) != 0){
		return EXIT;
	}

	if (!changed){
		return -1;
	}

	return MENU;
}

// render guide menu
void game::render_guide(){
	Mix_HaltMusic();
	SDL_SetWindowTitle(window, "Conqueror");
	SDL_FillRect(screen_surface, NULL, 0);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	// load guide
	surface_texture title;
	title.load_image("img/menu/guide.png");
	SDL_Rect src, des;
	
	des.w = 660;
	des.h = 680;
	des.x = (WINDOW_WIDTH - des.w) / 2;
	des.y = 130;
	SDL_BlitScaled(title.get_surface(), NULL, screen_surface, &des);

	// load instructions title
	title.load_image("img/menu/instructions.png");

	src.x = 0;
	src.y = 0;
	src.w = 48;
	src.h = 6;
	des.w = 480;
	des.h = 60;
	des.x = (WINDOW_WIDTH - des.w) / 2;
	des.y = 30;
	SDL_BlitScaled(title.get_surface(), &src, screen_surface, &des);

	// load return to menu
	title.load_image("img/menu/return_to_menu.png");

	des.w = 250;
	des.h = 250;
	des.x = (WINDOW_WIDTH - des.w) / 2;
	des.y = 650;
	SDL_BlitScaled(title.get_surface(), NULL, screen_surface, &des);

	SDL_UpdateTexture(screen_texture, NULL, screen_surface -> pixels, screen_surface -> pitch);
	SDL_RenderCopy(renderer, screen_texture, NULL, NULL);

	SDL_RenderPresent(renderer);
}

// render main menu
void game::render_menu(){
	Mix_HaltMusic();
	SDL_SetWindowTitle(window, "Conqueror");
	SDL_FillRect(screen_surface, NULL, 0);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	// load game title
	surface_texture title;
	title.load_image("img/menu/game_title1.png");
	SDL_Rect src, des;

	des.w = 640;
	des.h = 150;
	des.x = (WINDOW_WIDTH - des.w) / 2;
	des.y = 50;
	SDL_BlitScaled(title.get_surface(), NULL, screen_surface, &des);

	// load play button
	title.load_image("img/menu/play_button.png");

	src.w = 150;
	src.h = 98;
	src.x = play_button_state * 150;
	src.y = 0;
	SDL_BlitScaled(title.get_surface(), &src, screen_surface, &des_play);

	// load help button
	title.load_image("img/menu/help_button.png");

	src.w = 150;
	src.h = 98;
	src.x = help_button_state * 150;
	src.y = 0;
	SDL_BlitScaled(title.get_surface(), &src, screen_surface, &des_help);

	// load exit button
	title.load_image("img/menu/exit_button.png");

	src.w = 150;
	src.h = 98;
	src.x = exit_button_state * 150;
	src.y = 0;
	SDL_BlitScaled(title.get_surface(), &src, screen_surface, &des_exit);

	SDL_UpdateTexture(screen_texture, NULL, screen_surface -> pixels, screen_surface -> pitch);
	SDL_RenderCopy(renderer, screen_texture, NULL, NULL);

	SDL_RenderPresent(renderer);
}

// cleanup game
void game::clean(){
	if (strip_angles){
		delete[] strip_angles;
	}

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_FreeSurface(screen_surface);
	SDL_DestroyTexture(screen_texture);
	
	SDL_Quit();
	IMG_Quit();
	TTF_Quit();
	Mix_CloseAudio();
	Mix_Quit();
}

// reset the game
void game::reset(){
	raycaster3d.create_grids(MAP_WIDTH, MAP_HEIGHT, 1, TILE_SIZE);
	array2d_to_vector(g_map1, raycaster3d.grids[0]);
	array2d_to_vector(g_ceilingmap1, ceiling_grid);

	// set player's variable
	player.x = 2 * TILE_SIZE;
	player.y = (MAP_HEIGHT - 2) * TILE_SIZE;
	player.rot = M_PI / 2;
	player.move_speed = TILE_SIZE / (DESIRED_FPS / 60.0 * 16.0);
	player.rot_speed = 1.0 * M_PI / 180;

	// set up sprites
	sprites.clear();
	for (int i = 0; i < MAP_HEIGHT; ++i){
		for (int j = 0; j < MAP_WIDTH; ++j){
			int sprite_id = g_spritemap1[i][j];

			if (sprite_id){
				add_sprite_at(sprite_id, j, i);
			}

			doors[j + i * MAP_WIDTH] = 0;
		}
	}
}

// add sprite at cell
void game::add_sprite_at(int sprite_id, int cell_x, int cell_y){
	sprite s;
	s.x = cell_x * TILE_SIZE + (TILE_SIZE / 2);
	s.y = cell_y * TILE_SIZE + (TILE_SIZE / 2);
	s.w = s.h = TILE_SIZE;
	s.texture_id = sprite_id;

	int sprite_wall_x = int(s.x / TILE_SIZE);
	int sprite_wall_y = int(s.y / TILE_SIZE);

	bool in_wall = raycaster3d.safe_cell_at(sprite_wall_x, sprite_wall_y);
	if (in_wall){
		return;
	}

	sprites.push_back(s);
}

// at projectile at cell, with the direction
void game::add_projectile(int texture_id, int x, int y, int size, double rotation){
	sprite s;
	s.x = x;
	s.y = y;
	s.rot = rotation;
	s.w = s.h = size;
	s.texture_id = texture_id;

	projectile_queue.push(s);
}

// start the game
int game::start(){
	strip_width = DEFAULT_STRIP_WIDTH;
	ray_count = WINDOW_WIDTH / strip_width;
	fov_degrees = DEFAULT_FOV_DEGRESS;
	fov_radians = double(fov_degrees) * M_PI / 180;
	view_dis = raycaster::screen_distance(WINDOW_WIDTH, fov_radians);

	// calculate the angles for each column strip once and save them
	this -> strip_angles = new double [ray_count];
	for (int strip = 0; strip < ray_count; ++strip){
		// the origin is the player
		double screen_x = (ray_count / 2 - strip) * strip_width;
		strip_angles[strip] = raycaster::strip_angle(screen_x, view_dis);
	}

	Uint32 pf = SDL_GetWindowPixelFormat(window);

	SDL_PixelFormat *tmp_pf = SDL_AllocFormat(pf);
	ceiling_color = SDL_MapRGB(tmp_pf, 139, 185, 249);
	SDL_FreeFormat(tmp_pf);

	// load textures
	if (!walls_image.load_bitmap("img/texture/walls4.bmp")){
		printf("Error loading walls4.bmp\n");
		return EXIT;
	}
	if (!walls_image_dark.load_bitmap("img/texture/walls4dark.bmp")){
		printf("Error loading walls4dark.bmp\n");
		return EXIT;
	}
	if (!gates_image.load_bitmap("img/texture/gates.bmp")){
		printf("Error loading gates.bmp\n");
		return EXIT;
	}
	if (!gates_open_image.load_bitmap("img/texture/gatesopen.bmp")){
		printf("Error loading gatesopen.bmp\n");
		return EXIT;
	}

	walls_image.create_texture(renderer);
	walls_image_dark.create_texture(renderer);
	gates_image.create_texture(renderer);
	gates_open_image.create_texture(renderer);

	if (!gun_image.load_bitmap("img/texture/gun.bmp")){
		printf("Error loading gun.bmp\n");
		return EXIT;
	}
	Uint32 color_key = SDL_MapRGB(gun_image.get_surface() -> format, 152, 0, 136);
	SDL_SetColorKey(gun_image.get_surface(), 1, color_key);
	gun_image.create_texture(renderer);

	if (!crosshair_image.load_image("img/texture/crosshair.png")){
		printf("Error loading crosshair.png\n");
		return EXIT;
	}
	crosshair_image.create_texture(renderer);

	SDL_SetColorKey(gates_image.get_surface(), 1, color_key);
	SDL_SetColorKey(gates_open_image.get_surface(), 1, color_key);

	// load sprite images
	map <int, string> sprite_file_names;
	sprite_file_names[SPRITE_TYPE_TREE_1] = "tree1.bmp";
	sprite_file_names[SPRITE_TYPE_TREE_2] = "tree2.bmp";
	sprite_file_names[SPRITE_TYPE_ZOMBIE] = "zombie.bmp";
	sprite_file_names[SPRITE_TYPE_SKELETON] = "skeleton.bmp";
	sprite_file_names[SPRITE_TYPE_ROBOT] = "robot.bmp";
	sprite_file_names[	SPRITE_TYPE_FROGMAN] = "frogman.bmp";
	sprite_file_names[SPRITE_TYPE_HEROINE] = "heroine.bmp";
	sprite_file_names[SPRITE_TYPE_DRUID] = "druid.bmp";
	sprite_file_names[SPRITE_TYPE_PROJECTILE] = "plasmball.bmp";
	sprite_file_names[SPRITE_TYPE_PROJECTILE_SPLASH] = "plasmball_hit.bmp";

	for (__typeof(sprite_file_names.begin()) it = sprite_file_names.begin(); it != sprite_file_names.end(); ++it){
		int texture_id = it -> first;
		string filename = "img/texture/" + it -> second;

		surface_texture &Surface_texture = sprite_textures[texture_id];
		if (!Surface_texture.load_bitmap(filename.c_str())){
			printf("Error loading %s\n", filename.c_str());
			return EXIT;
		}

		SDL_SetColorKey(Surface_texture.get_surface(), 1, color_key);
		Surface_texture.create_texture(renderer);
	}

	// load floor and ceiling images
	map <int, string> floor_ceiling_filenames;
	floor_ceiling_filenames[0] = "grass.bmp";
	floor_ceiling_filenames[1] = "texture1.bmp";
	floor_ceiling_filenames[2] = "texture2.bmp";
	floor_ceiling_filenames[3] = "texture3.bmp";
	floor_ceiling_filenames[4] = "texture4.bmp";
	floor_ceiling_filenames[5] = "default_brick.bmp";
	floor_ceiling_filenames[6] = "default_aspen_wood.bmp";
	floor_ceiling_filenames[7] = "water.bmp";
	floor_ceiling_filenames[8] = "mossycobble.bmp";

	floor_ceiling_bitmaps.resize(floor_ceiling_filenames.size());

	for (__typeof(floor_ceiling_filenames.begin()) it = floor_ceiling_filenames.begin(); it != floor_ceiling_filenames.end(); ++it){
		int texture_id = it -> first;
		string filename = "img/texture/" + it -> second;

		bitmap &Bitmap = floor_ceiling_bitmaps[texture_id];
		if (!Bitmap.load(filename.c_str(), renderer, pf)){
			printf("Error loading %s\n", filename.c_str());
			return EXIT;
		}
	}

	if (!ceiling_bitmap.load("img/texture/texture1.bmp", renderer, pf)){
		printf("Error loading texture1.bmp\n");
		return EXIT;
	}

	// load sound effect and bgm
	projectile_fire_sound = Mix_LoadWAV("sfx/shooting.wav");
	projectile_impact_sound = Mix_LoadWAV("sfx/explode.wav");
	door_open_sound = Mix_LoadWAV("sfx/door_open.wav");
	door_close_sound = Mix_LoadWAV("sfx/door_close.wav");
	bgm_music = Mix_LoadMUS("sfx/bgm.mp3");
	
	Mix_VolumeChunk(projectile_fire_sound, MIX_MAX_VOLUME / 2);
	Mix_VolumeChunk(projectile_impact_sound, MIX_MAX_VOLUME / 3);
	Mix_VolumeChunk(door_open_sound, MIX_MAX_VOLUME / 2);
	Mix_VolumeChunk(door_close_sound, MIX_MAX_VOLUME / 2);
	Mix_VolumeMusic(MIX_MAX_VOLUME / 5);

	sfx = 1;
	bgm = 0;

	this -> is_running = 1;

	return run();
}

// draw the game
void game::draw(){
	// clear screen
	SDL_FillRect(screen_surface, NULL, 0);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	vector <ray_hit> all_ray_hits;
	all_ray_hits.clear();
	
	raycast_world(all_ray_hits);
	
	draw_world(all_ray_hits);
	draw_weapon();

	SDL_UpdateTexture(screen_texture, NULL, screen_surface -> pixels, screen_surface -> pitch);
	SDL_RenderCopy(renderer, screen_texture, NULL, NULL);

	// draw the minimap
	if (draw_minimap_on){
		draw_minimap();
		draw_rays(all_ray_hits);
		draw_player();
		draw_minimap_sprites();
	}

	SDL_RenderPresent(renderer);
}

// fill a rectangle on the screen
void game::fill_rect(SDL_Rect *rc, int r, int g, int b){
	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, rc);
}

// draw a line with given coord on the screen
void game::draw_line(int start_x, int start_y, int end_x, int end_y, int r, int g, int b, int alpha){
	SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
	SDL_RenderDrawLine(renderer, start_x, start_y, end_x, end_y);
}

// draw gun and crosshair
void game::draw_weapon(){
	double gun_scale = WINDOW_WIDTH / 500;

	SDL_Rect des;
	SDL_Surface *gun_surface = gun_image.get_surface();
	des.w = gun_surface -> w * gun_scale;
	des.h = gun_surface -> h * gun_scale;
	des.x = (WINDOW_WIDTH - des.w) / 2;
	des.y = WINDOW_HEIGHT - des.h;

	SDL_BlitScaled(gun_surface, NULL, screen_surface, &des);

	SDL_Surface *crosshair_surface = crosshair_image.get_surface();
	des.w = crosshair_surface -> w * (gun_scale * 0.03);
	des.h = crosshair_surface -> h * (gun_scale * 0.03);
	des.x = (WINDOW_WIDTH - des.w) / 2;
	des.y = (WINDOW_HEIGHT - des.h) / 2;

	SDL_BlitScaled(crosshair_surface, NULL, screen_surface, &des);
}

// draw the minimap
void game::draw_minimap(){
	SDL_Rect minimap_rect;
	minimap_rect.x = 0;
	minimap_rect.y = MINIMAP_Y;
	minimap_rect.w = MAP_WIDTH * MINIMAP_SCALE;
	minimap_rect.w = MAP_HEIGHT * MINIMAP_SCALE;
	fill_rect(&minimap_rect, 0, 0, 0);

	for (int i = 0; i < MAP_WIDTH; ++i){
		for (int j = 0; j < MAP_HEIGHT; ++j){
			if (g_map1[j][i]){
				SDL_Rect rc;
				rc.x = i * MINIMAP_SCALE;
				rc.y = j * MINIMAP_SCALE + MINIMAP_Y;
				rc.w = MINIMAP_SCALE;
				rc.h = MINIMAP_SCALE;

				if (g_map1[j][i] < 1000){
					fill_rect(&rc, 200, 200, 200);
				}
				else{
					fill_rect(&rc, 0, 255, 255);
				}
			}
			else{
				SDL_Rect rc;
				rc.x = i * MINIMAP_SCALE;
				rc.y = j * MINIMAP_SCALE + MINIMAP_Y;
				rc.w = MINIMAP_SCALE;
				rc.h = MINIMAP_SCALE;
				
				fill_rect(&rc, 0, 0, 0);
			}
		}
	}
}

// draw the player on the minimap
void game::draw_player(){
	SDL_Rect player_rect;

	double player_x = (double) player.x / (MAP_WIDTH * TILE_SIZE) * 100;
	player_x = player_x / 100 * MINIMAP_SCALE * MAP_WIDTH;
	double player_y = (double) player.y / (MAP_HEIGHT * TILE_SIZE) * 100;
	player_y = player_y / 100 * MINIMAP_SCALE * MAP_HEIGHT;

	player_rect.x = player_x - 2;
	player_rect.y = player_y - 2 + MINIMAP_Y;
	player_rect.w = player_rect.h = 5;
	fill_rect(&player_rect, 255, 0, 0);

	double line_end_x = player_x + cos(player.rot) * 3 * MINIMAP_SCALE;
	double line_end_y = player_y - sin(player.rot) * 3 * MINIMAP_SCALE;

	draw_line(player_x, player_y + MINIMAP_Y, line_end_x, line_end_y + MINIMAP_Y, 255, 0, 0);
}

// draw sprites on the minimap
void game::draw_minimap_sprites(){
	for (__typeof(sprites.begin()) it = sprites.begin(); it != sprites.end(); ++it){
		sprite &s = *it;

		if (s.hidden){
			continue;
		}

		double sprite_x = (double) s.x / (MAP_WIDTH * TILE_SIZE) * 100;
		sprite_x = sprite_x / 100 * MINIMAP_SCALE * MAP_WIDTH;
		double sprite_y = (double) s.y / (MAP_HEIGHT * TILE_SIZE) * 100;
		sprite_y = sprite_y / 100 * MINIMAP_SCALE * MAP_HEIGHT;

		SDL_Rect rc;
		rc.x = sprite_x - 2;
		rc.y = sprite_y - 2 + MINIMAP_Y;
		rc.w = rc.h = 5;

		fill_rect(&rc, 255, 0, 0);
	}
}

// draw 1 ray on the minimap
void game::draw_ray(double ray_x, double ray_y){
	double player_x = (double) player.x / (MAP_WIDTH * TILE_SIZE) * 100;
	player_x = player_x / 100 * MINIMAP_SCALE * MAP_WIDTH;
	double player_y = (double) player.y / (MAP_HEIGHT * TILE_SIZE) * 100;
	player_y = player_y / 100 * MINIMAP_SCALE * MAP_HEIGHT;

	ray_x = ray_x / (MAP_WIDTH * TILE_SIZE) * 100;
	ray_x = ray_x / 100 * MINIMAP_SCALE * MAP_WIDTH;
	ray_y = (double) ray_y / (MAP_HEIGHT * TILE_SIZE) * 100;
	ray_y = ray_y / 100 * MINIMAP_SCALE * MAP_HEIGHT;

	draw_line(player_x, player_y + MINIMAP_Y, ray_x, ray_y + MINIMAP_Y, 100, 0, 103, 0.3 * 255);
}

// draw rays of fov on the minimap
void game::draw_rays(vector <ray_hit> &ray_hits){
	for (int i = 0; i < (int) ray_hits.size(); ++i){
		ray_hit &r = ray_hits[i];

		if (r.wall_type){
			draw_ray(r.x, r.y);
		}
	}
}

// run the game
int game::run(){
	int past = SDL_GetTicks();
	int now = past, past_fps = past;
	int fps = 0, frame_skipped = 0;

	SDL_Event event;

	if (bgm){
		Mix_PlayMusic(bgm_music, 1);
	}

	while(is_running){
		int time_elapsed = 0;

		SDL_PumpEvents();

		if (keyboard_state[SDL_SCANCODE_RETURN]){
			return MENU;
		}

		if (keyboard_state[SDL_SCANCODE_ESCAPE]){
			return EXIT;
		}

		if (SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					return on_quit();
					break;
				case SDL_KEYUP:
					on_key_up(&event);
					break;
				default:
					break;
			}
		}

		// update and draw
		now = SDL_GetTicks();
		time_elapsed = now - past;
		if (time_elapsed >= UPDATE_INTERVAL){
			past = now;

			update(time_elapsed);

			if (frame_skipped++ >= frame_skip){
				draw();
				++fps;
				frame_skipped = 0;
			}

		}

		// fps
		if (now - past_fps >= 1000){
			past_fps = now;
			SDL_SetWindowTitle(window, ("Conqueror (" + to_string(fps) + " FPS)").c_str());
			fps = 0;
		}
	}

	return MENU;
}

// update the game (basic player movement) after 1 game cycle
void game::update(double time_elapsed){
	// move forward
	if (keyboard_state[SDL_SCANCODE_W]){
		player.speed = 1;
	}
	// move backward
	else if (keyboard_state[SDL_SCANCODE_S]){
		player.speed = -1;
	}
	// stop moving
	else{
		player.speed = 0;
	}

	// rotate left
	if (keyboard_state[SDL_SCANCODE_A]){
		player.dir = -1;
	}
	// rotate right
	else if (keyboard_state[SDL_SCANCODE_D]){
		player.dir = 1;
	}
	// stop rotating
	else{
		player.dir = 0;
	}

	update_player(time_elapsed);
	update_projectiles(time_elapsed);
}

// update player after 1 game cycle
void game::update_player(double time_elapsed){
	double time_based_factor = time_elapsed / UPDATE_INTERVAL;
	double move_step = player.speed * player.move_speed * time_based_factor;
	player.rot += -player.dir * player.rot_speed * time_based_factor;

	double new_x = player.x + cos(player.rot) * move_step;	
	double new_y = player.y + (-sin(player.rot)) * move_step;

	bool move_left = keyboard_state[SDL_SCANCODE_Q];
	bool move_right = keyboard_state[SDL_SCANCODE_E];
	double strafe_step = (move_left || move_right) * player.move_speed * time_based_factor;
	double strafe_rotation = 0;

	/*
		move left/right is just +/- 90 degrees to the player's rotation
	*/
	if (move_left){
		strafe_rotation = player.rot + M_PI / 2;
	}
	if (move_right){
		strafe_rotation = player.rot - M_PI / 2;
	}

	new_x += cos(strafe_rotation) * strafe_step;
	new_y += -sin(strafe_rotation) * strafe_step;
	
	// hit wall
	if (player_in_wall(new_x, new_y)){
		// try moving left/right only
		if (!player_in_wall(new_x, player.y)){
			player.x = new_x;
		}
		// try moving forward/backward only
		if (!player_in_wall(player.x, new_y)){
			player.y = new_y;
		}
	}
	else{
		player.x = new_x;
		player.y = new_y;
	}
}

// update the projectile after 1 game cycle
void game::update_projectiles(double time_elapsed){
	sprites.erase(remove_if(sprites.begin(), sprites.end(), needs_clean_up), sprites.end());

	double time_based_factor = time_elapsed / UPDATE_INTERVAL;
	double projectile_speed = 3 * player.move_speed * time_based_factor; // faster than player speed
	double move_step = 1 * projectile_speed; // only move forward

	// process all the projectile in the queue
	while(projectile_queue.size()){
		sprite new_projectile = projectile_queue.front();
		projectile_queue.pop();

		// if projectile hasn't explode
		if (new_projectile.texture_id != SPRITE_TYPE_PROJECTILE_SPLASH){
			double new_x = new_projectile.x + cos(new_projectile.rot) * player.move_speed;
			double new_y = new_projectile.y - sin(new_projectile.rot) * player.move_speed;

			new_projectile.x = new_x;
			new_projectile.y = new_y;
		}
		// projectile exploded
		else{
			new_projectile.frame = 0;
			// how long the explosion lasts
			new_projectile.frame_rate = 200;
		}

		sprites.push_back(new_projectile);
	}

	for (__typeof(sprites.begin()) it = sprites.begin(); it != sprites.end(); ++it){
		sprite &projectile = *it;

		// update the explosion
		if (projectile.texture_id == SPRITE_TYPE_PROJECTILE_SPLASH){
			projectile.frame_rate -= time_elapsed;

			if (projectile.frame_rate <= 0){
				projectile.cleanup = projectile.hidden = 1;
			}

			continue;
		}

		// kill the sprite that got hit
		if (is_killable_sprite(projectile.texture_id)){
			if (projectile.frame_rate){
				projectile.frame_rate -= time_elapsed;

				if (projectile.frame_rate <= 0){
					projectile.cleanup = projectile.hidden = 1;
				}

				continue;
			}
		}

		// only process the projectile 
		if (projectile.texture_id != SPRITE_TYPE_PROJECTILE){
			continue;
		}

		double new_x = projectile.x + cos(projectile.rot) * move_step;
		double new_y = projectile.y - sin(projectile.rot) * move_step;
		int wall_x = new_x / TILE_SIZE;
		int wall_y = new_y / TILE_SIZE;

		bool wall_hit = 0;
		bool out_of_bounds = 0;
		bool hit_other_sprites = 0;

		// if hit wall cell
		if (is_wall_cell(wall_x, wall_y)){
			new_x = projectile.x + cos(projectile.rot) * move_step / 4;
			new_y = projectile.y - sin(projectile.rot) * move_step / 4;
			wall_x = new_x / TILE_SIZE;
			wall_y = new_y / TILE_SIZE;

			// move the projectile a little bit
			if (is_wall_cell(wall_x, wall_y)){
				if (!projectile.cleanup){
					add_projectile(SPRITE_TYPE_PROJECTILE_SPLASH, projectile.x, projectile.y, TILE_SIZE, projectile.rot);
					if (sfx){
						Mix_PlayChannel(-1, projectile_impact_sound, 0);
					}
				}

				projectile.cleanup = 1;
				wall_hit = 1;
			}
		}

		// out of bound
		if (new_x < 0 || new_x > MAP_WIDTH * TILE_SIZE || new_y < 0 || new_y > MAP_HEIGHT * TILE_SIZE){
			out_of_bounds = 1;
			projectile.cleanup = 1;
		}

		if (!wall_hit && !out_of_bounds){
			vector <sprite*> sprites_hit = raycaster::find_sprites_in_cell(sprites, wall_x, wall_y, TILE_SIZE);

			for (__typeof(sprites_hit.begin()) it2 = sprites_hit.begin(); it2 != sprites_hit.end(); ++it2){
				sprite *s = *it2;

				// hit a sprite, add an explosion
				if (is_killable_sprite(s -> texture_id) && !s -> hidden){
					if (s -> frame_rate == 0){
						s -> frame_rate = 200;
						s -> cleanup = 1;

						hit_other_sprites = 1;

						if (!projectile.cleanup){
							projectile.cleanup = 1;

							add_projectile(SPRITE_TYPE_PROJECTILE_SPLASH, projectile.x, projectile.y, TILE_SIZE, projectile.rot);
							if (sfx){
								Mix_PlayChannel(-1, projectile_impact_sound, 0);
							}
						}
					}
				}
			}
		}

		// update the projectile pos
		if (!wall_hit && !out_of_bounds && !hit_other_sprites){
			projectile.x = new_x;
			projectile.y = new_y;
		}
	}
}

// draw the floor in 3d (psuedo)
void game::draw_floor(vector <ray_hit> &ray_hits){
	Uint32 *screen_pixels = (Uint32*) screen_surface -> pixels;
	for (int i = 0; i < (int) ray_hits.size(); ++i){
		ray_hit &r = ray_hits[i];

		// must be a wall, not a sprite
		if (!r.wall_type){
			continue;
		}

		int wall_screen_height = raycaster::strip_screen_height(view_dis, r.correct_distance, TILE_SIZE);
		double center_plane = WINDOW_HEIGHT / 2;
		double eye_height = TILE_SIZE / 2;
		int screen_x = r.strip * strip_width;
		int screen_y = (WINDOW_HEIGHT - wall_screen_height) / 2 + wall_screen_height;
		if (screen_y < center_plane){
			screen_y = center_plane;
		}

		/*
			how many times a texture repeat inside itself
			numbers of repeat = texture_repeat ^ 2
		*/
		int texture_repeat = 2;

		const double cos_factor = 1 / cos(player.rot - r.ray_angle);

		for (; screen_y < WINDOW_HEIGHT; ++screen_y){
			double ratio = eye_height / (screen_y - center_plane);
			double straight_distance = view_dis * ratio;
			double diagonal_distance = straight_distance * cos_factor;

			double x_end = (diagonal_distance * cos(r.ray_angle));
			double y_end = (diagonal_distance * (-sin(r.ray_angle)));
			x_end += player.x;
			y_end += player.y;

			int x = (int) (x_end * texture_repeat) % TILE_SIZE;
			int y = (int) (y_end * texture_repeat) % TILE_SIZE;
			int tile_x = x_end / TILE_SIZE;
			int tile_y = y_end / TILE_SIZE;

			bool out_of_bounds = (x < 0 || tile_x >= MAP_WIDTH || y < 0 || tile_y >= MAP_HEIGHT);

			int floor_type = g_floormap1[tile_y][tile_x];
			if (floor_type < 0 || floor_type >= (int) floor_ceiling_bitmaps.size() || out_of_bounds){
				continue;
			}

			bitmap &b = floor_ceiling_bitmaps[floor_type];
			Uint32 *px = (Uint32*) b.get_pixels();

			int tex_x = (double) x / TILE_SIZE * TEXTURE_SIZE;
			int tex_y = (double) y / TILE_SIZE * TEXTURE_SIZE;
			int des_pixel = screen_x + screen_y * WINDOW_WIDTH;
			int src_pixel = tex_x + tex_y * b.get_width();
			bool pixel_ok = (src_pixel >= 0 && des_pixel >= 0 &&
							src_pixel <= b.get_width() * b.get_height() &&
							des_pixel <= WINDOW_WIDTH * WINDOW_HEIGHT);
			
			for (int j = 0; j < strip_width; ++j){
				screen_pixels[des_pixel + j] = px[src_pixel];
			}
		}
	}
}

// draw the ceiling in 3d (psuedo)
void game::draw_ceiling(vector <ray_hit> &ray_hits){
	Uint32 *screen_pixels = (Uint32*) screen_surface -> pixels;
	for (int i = 0; i < (int) ray_hits.size(); ++i){
		ray_hit &r = ray_hits[i];

		// must be a wall, not a sprite
		if (!r.wall_type){
			continue;
		}

		int wall_screen_height = raycaster::strip_screen_height(view_dis, r.correct_distance, TILE_SIZE);
		double center_plane = WINDOW_HEIGHT / 2;
		double eye_height = TILE_SIZE / 2;
		int screen_x = r.strip * strip_width;
		int screen_y = (WINDOW_HEIGHT - wall_screen_height) / 2;
		if (screen_y >= WINDOW_HEIGHT / 2){
			screen_y = WINDOW_HEIGHT / 2 - 1;
		}

		const double cos_factor = 1 / cos(player.rot - r.ray_angle);

		for (; screen_y >= 0; --screen_y){
			double ratio = (TILE_SIZE - eye_height) / (center_plane - screen_y);
			double straight_distance = view_dis * ratio;
			double diagonal_distance = straight_distance * cos_factor;

			double x_end = (diagonal_distance * cos(r.ray_angle));
			double y_end = (diagonal_distance * (-sin(r.ray_angle)));
			x_end += player.x;
			y_end += player.y;
			int x = (int) (x_end) % TILE_SIZE;
			int y = (int) (y_end) % TILE_SIZE;
			int tile_x = x_end / TILE_SIZE;
			int tile_y = y_end / TILE_SIZE;
			bool out_of_bounds = (x_end < 0 || x_end >= MAP_WIDTH * TILE_SIZE || y_end < 0 || y_end >= MAP_HEIGHT * TILE_SIZE);

			int ceiling_type = (out_of_bounds ? -1 : g_ceilingmap1[tile_y][tile_x]);
			if (ceiling_type < 0 || ceiling_type >= (int) floor_ceiling_bitmaps.size() || out_of_bounds){
				continue;
			}

			bitmap &b = floor_ceiling_bitmaps[ceiling_type];
			Uint32 *px = (Uint32*) b.get_pixels();
			
			int tex_x = (double) x / TILE_SIZE * TEXTURE_SIZE;
			int tex_y = (double) y / TILE_SIZE * TEXTURE_SIZE;
			int src_pixel = tex_x + tex_y * b.get_width();
			int des_pixel = screen_x + screen_y * WINDOW_WIDTH;

			if (des_pixel >= WINDOW_WIDTH * WINDOW_HEIGHT){
				continue;
			}

			for (int j = 0; j < strip_width; ++j){
				screen_pixels[des_pixel + j] = px[src_pixel];
			}
		}
	}
}

// draw the world in 3d (psuedo)
void game::draw_world(vector <ray_hit> &ray_hits){
	// sort all the rays because we need to draw the furthest ray first
	ray_hit_sorter Ray_hit_sorter(&raycaster3d, TILE_SIZE / 2);
	sort(ray_hits.begin(), ray_hits.end(), Ray_hit_sorter);

	draw_ceiling(ray_hits);
	draw_floor(ray_hits);

	// erase all sprites that need cleanup to free up memory in case player's spamming
	sprites.erase(remove_if(sprites.begin(), sprites.end(), needs_clean_up), sprites.end());

	// draw walls and sprite
	for (int i = 0; i < (int) ray_hits.size(); ++i){
		ray_hit &r = ray_hits[i];

		// wall
		if (r.wall_type){
			// calculate the height of that ray
			int wall_screen_height = raycaster::strip_screen_height(view_dis, r.correct_distance, TILE_SIZE);
			double sx = r.tile_x / TILE_SIZE * TEXTURE_SIZE;
			if (sx >= TEXTURE_SIZE){
				sx = TEXTURE_SIZE - 1;
			}
			double sy = TEXTURE_SIZE * (r.wall_type - 1);

			// if the wall is horizontal, the wall will be darker to distinct from the vertical wall (horizontal and vertical in 2d)
			surface_texture *img = (r.horizontal ? &walls_image_dark : &walls_image);

			// if wall is open
			bool wall_is_door = raycaster::is_door(r.wall_type);
			if (wall_is_door){
				sy = 0;
				img = (doors[r.wall_x + r.wall_y * MAP_WIDTH] ? &gates_open_image : &gates_image);
			}

			// draw the wall on the screen
			draw_wall_strip(r, *img, sx, sy, wall_screen_height);
		}
		// sprite
		else if (r.Sprite && !r.Sprite -> hidden){
			SDL_Rect des_rect;
			surface_texture *sprite_st = nullptr;
			map <int, surface_texture>::iterator it = sprite_textures.find(r.Sprite -> texture_id);
			if (it == sprite_textures.end()){
				continue;
			}
			sprite_st = &sprite_textures[r.Sprite -> texture_id];
			des_rect = find_sprite_screen_position(*r.Sprite);
			SDL_BlitScaled(sprite_st -> get_surface(), NULL, screen_surface, &des_rect);
		}
	}
}

// calculate the srip's rectangle
SDL_Rect game::strip_screen_rect(ray_hit &Ray_hit, double wall_height){
	// height of 1 tile
	double default_wall_screen_height = raycaster::strip_screen_height(view_dis, Ray_hit.correct_distance, TILE_SIZE);
	// height of the wall
	double wall_screen_height = ((wall_height == TILE_SIZE) ? default_wall_screen_height : 
								raycaster::strip_screen_height(view_dis, Ray_hit.correct_distance, wall_height));

	// clamp height because SDL_Rect uses short int
	static const double MAX_WALL_HEIGHT = SDL_MAX_SINT16;
	if (default_wall_screen_height > MAX_WALL_HEIGHT){
		default_wall_screen_height = MAX_WALL_HEIGHT;
	}
	if (wall_screen_height > MAX_WALL_HEIGHT){
		wall_screen_height = MAX_WALL_HEIGHT;
	}

	SDL_Rect rc;
	rc.x = Ray_hit.strip * strip_width;
	rc.y = (WINDOW_HEIGHT - default_wall_screen_height) / 2 + (default_wall_screen_height - wall_screen_height);
	rc.w = strip_width;
	rc.h = wall_screen_height;

	return rc;
}

// draw a wall strip on the screen
void game::draw_wall_strip(ray_hit &Ray_hit, surface_texture &img, double tex_x, double tex_y, int wall_screen_height){
	/*
		clamp wall screen height
	*/
	static const double MAX_WALL_HEIGHT = SDL_MAX_SINT16;
	if (wall_screen_height > MAX_WALL_HEIGHT){
		wall_screen_height = MAX_WALL_HEIGHT;
	}

	SDL_Rect src_rect, des_rect;
	src_rect.x = tex_x;
	src_rect.y = tex_y;
	src_rect.w = strip_width;
	src_rect.h = TEXTURE_SIZE;

	des_rect.x = Ray_hit.strip * strip_width;
	des_rect.y = (WINDOW_HEIGHT - wall_screen_height) / 2;
	des_rect.w = strip_width;
	des_rect.h = wall_screen_height;

	SDL_BlitScaled(img.get_surface(), &src_rect, screen_surface, &des_rect);
}

// https://dev.opera.com/articles/3d-games-with-canvas-and-raycasting-part-2/
SDL_Rect game::find_sprite_screen_position(sprite &s){
	// translate position to viewer space
	double dx = s.x - player.x;
	double dy = s.y - player.y;

	// distance to sprite
	double dist = sqrt(dx * dx + dy * dy);

	double sprite_angle = atan2(dy, dx) + player.rot;
	double sprite_distance = cos(sprite_angle) * dist;
	double sprite_screen_width = TILE_SIZE * view_dis / sprite_distance;

	// x pos on screen
	double x = tan(sprite_angle) * view_dis;

	SDL_Rect rc;
	rc.x = (WINDOW_WIDTH - sprite_screen_width) / 2 + x;
	rc.y = (WINDOW_HEIGHT - sprite_screen_width) / 2.0;
	rc.w = rc.h = sprite_screen_width;

	return rc;
}

void game::raycast_world(vector <ray_hit> &ray_hits){
	vector <sprite*> sprites_found;
	ray_hits_count = 0;

	for (int i = 0; i < (int) sprites.size(); ++i){
		sprites[i].Ray_hit = 0;
	}

	// loop through and raycast each angle
	for (int strip = 0; strip < ray_count; ++strip){
		const double strip_angle = strip_angles[strip];

		raycaster3d.raycast(ray_hits, player.x, player.y, player.rot, strip_angle, strip);
		raycaster3d.raycast_sprites(ray_hits, raycaster3d.grids, raycaster3d.grid_width, raycaster3d.grid_height, TILE_SIZE,
									player.x, player.y, player.rot, strip_angle, strip, &sprites);
	}

	ray_hits_count = ray_hits.size();
}

bool game::is_wall_cell(int x, int y){
	// clamp first
	if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT){
		return 1;
	}

	if (raycaster::is_door(g_map1[y][x])){
		return (!doors[x + y * MAP_WIDTH]);
	}

	return raycaster3d.safe_cell_at(x, y);
}

bool game::player_in_wall(double player_x, double player_y){
	// clamp first
	if (player_x < 0 || player_y < 0 || player_x >= MAP_WIDTH * TILE_SIZE || player_y >= MAP_HEIGHT * TILE_SIZE){
		return 1;
	}

	double player_width = TILE_SIZE / 5;
	int player_tile_x = player_x / TILE_SIZE;	
	int player_tile_y = player_y / TILE_SIZE;
	int player_tile_left = (int) (player_x - player_width) / TILE_SIZE;
	int player_tile_right = (int) (player_x + player_width) / TILE_SIZE;
	int player_tile_top = (int) (player_y - player_width) / TILE_SIZE;
	int player_tile_bottom = (int) (player_y + player_width) / TILE_SIZE;

	// current
	if (raycaster3d.safe_cell_at(player_tile_x, player_tile_y)){
		if (raycaster::is_door(g_map1[player_tile_y][player_tile_x])){
			if (!doors[player_tile_x + player_tile_y * MAP_WIDTH]){
				return 1;
			}
		}
	}
	// top left
	if (raycaster3d.safe_cell_at(player_tile_left, player_tile_top) && !raycaster::is_door(g_map1[player_tile_top][player_tile_left])){
		return 1;
	}
	// top right
	if (raycaster3d.safe_cell_at(player_tile_right, player_tile_top) && !raycaster::is_door(g_map1[player_tile_top][player_tile_right])){
		return 1;
	}
	// bottom left
	if (raycaster3d.safe_cell_at(player_tile_left, player_tile_bottom) && !raycaster::is_door(g_map1[player_tile_bottom][player_tile_left])){
		return 1;
	}
	// bottom right
	if (raycaster3d.safe_cell_at(player_tile_right, player_tile_bottom) && !raycaster::is_door(g_map1[player_tile_bottom][player_tile_right])){
		return 1;
	}

	return 0;
}

void game::on_key_up(SDL_Event *event){
	int sym = event -> key.keysym.sym;
	switch(sym){
		case SDLK_r: {
			reset();
			break;
		}
		case SDLK_0: {
			printf("%d %d\n", (int) player.x / TILE_SIZE, (int) player.y / TILE_SIZE);
			break;
		}
		case SDLK_m: {
			draw_minimap_on = !draw_minimap_on;
			printf("draw_minimap = %d\n", draw_minimap_on);
			break;
		}
		case SDLK_t: {
			bgm = !bgm;
			if (bgm){
				Mix_PlayMusic(bgm_music, 1);
			}
			else{
				Mix_HaltMusic();
			}
			printf("bgm = %d\n", bgm);
			break;
		}
		case SDLK_y: {
			sfx = !sfx;
			printf("sfx = %d\n", sfx);
			break;
		}
		case SDLK_f: {
			toggle_door_pressed();
			break;
		}
		case SDLK_SPACE: {
			add_projectile(SPRITE_TYPE_PROJECTILE, player.x, player.y, TILE_SIZE, player.rot);
			if (sfx){
				Mix_PlayChannel(-1, projectile_fire_sound, 0);
			}
			break;
		}
	}
}

void game::toggle_door(int x, int y){
	int offset = x + y * MAP_WIDTH;
	doors[offset] = !doors[offset];

	if (sfx){
		if (doors[offset]){
			Mix_PlayChannel(-1, door_open_sound, 0);
		}
		else{
			Mix_PlayChannel(-1, door_close_sound, 0);
		}
	}
}

void game::toggle_door_pressed(){
	const int wall_x = player.x / TILE_SIZE;
	const int wall_y = player.y / TILE_SIZE;
	
	int right_wall = raycaster3d.safe_cell_at(wall_x + 1, wall_y);
	if (raycaster::is_door(right_wall)){
		toggle_door(wall_x + 1, wall_y);
	}
	int left_wall = raycaster3d.safe_cell_at(wall_x - 1, wall_y);
	if (raycaster::is_door(left_wall)){
		toggle_door(wall_x - 1, wall_y);
	}
	int top_wall = raycaster3d.safe_cell_at(wall_x, wall_y - 1);
	if (raycaster::is_door(top_wall)){
		toggle_door(wall_x, wall_y - 1);
	}
	int bottom_wall = raycaster3d.safe_cell_at(wall_x, wall_y + 1);
	if (raycaster::is_door(bottom_wall)){
		toggle_door(wall_x, wall_y + 1);
	}
}