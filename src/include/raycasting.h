#pragma once

#include "bits/stdc++.h"
#include "shape.h"

using namespace std;

#define THICK_WALL_TYPE_NONE 0
#define THICK_WALL_TYPE_RECT 1

// a wall is made up of 2 points
struct wall{
	// start and end of line
	double x1, y1, x2, y2;
	double z;
	int wall_type;
	bool horizontal;
	double height;

	wall();
	wall(double x1, double y1, double x2, double y2, int wall_type, double wall_height);

	double distance_to_origin(double ix, double iy);
};

// handle texture of walls, ceilings and floors
struct sprite{
	double x, y;
	int w, h;
	int dir; // -1 for left, 1 for right
	double rot; // rotation (counter clockwise is positive)
	int speed; // 1 for forward, -1 for backward
	int move_speed; // how far (in map units) to move each step/update
	double rot_speed; // rotation speed (in radian)
	double distance; // used for z-buffer calculation (https://www.geeksforgeeks.org/z-buffer-depth-buffer-method/)
	int texture_id;
	bool cleanup;
	int frame_rate;
	int frame;
	bool hidden;
	bool Ray_hit;

	sprite(){
		x = y = 0;
		w = h = 0;
		dir = 0;
		rot = 0;
		speed = 0;

		move_speed = 0;
		rot_speed = 0;
		distance = 0;
		texture_id = 0;
		cleanup = 0;
		frame_rate = 0;
		frame = 0;
		hidden = 0;
		Ray_hit = 0;
	}
};

// handle info about a wall hit from a single ray
struct ray_hit{
	double x, y; // wall pos in game units
	int wall_x, wall_y; // wall pos in map col, row units
	int wall_type; // type of wall hit
	int strip; // strip of screen for this wall
	double tile_x; // x - coord within tile, used for calculating texture x
	double squared_distance; // distance squared duh
	double distance; // distance to wall
	double correct_distance; // fisheye correction distance
	bool horizontal; // true if wall was hit on the bottom or top
	double ray_angle; // angle used for calculation
	sprite *Sprite; // a sprite was hit
	bool right; // if ray angle is in right unit circle half
	bool up; // if ray angle is in upper unit circle half
	wall *Wall;
	double wall_height;

	/*
		sort_distance is used to sort which objects are drawn first
		further objects are drawn first
	*/
	double sort_distance;

	ray_hit(int world_x = 0, int world_y = 0, double angle = 0){
		x = world_x;
		y = world_y;
		ray_angle = angle;

		wall_type = strip = wall_x = wall_y = tile_x = squared_distance = distance = 0;
		correct_distance = 0;
		horizontal = 0;
		Sprite = 0;
		sort_distance = 0;
		Wall = 0;
		wall_height = 0;
	}
};

/*
	contains static utility functions for raycasting

	a value holds info for 1 or more 2d grids with the same dimensions
	each grid is stored as a single vector in row-major order
	the element offset for each grid is calculated using x + y * width

	if there are 2 or more grids, it can be used as a 3d grid
*/
struct raycaster{
	vector <vector <int>> grids;
	int grid_width;
	int grid_height;
	int grid_count;
	int tile_size;

	raycaster(){
		grid_width = grid_height = grid_count = tile_size = 0;
	}

	void create_grids(int _grid_width, int _grid_height, int _tile_size){
		grid_width = _grid_width;
		grid_height = _grid_height;
		tile_size = _tile_size;

		create_grids(_grid_width, _grid_height, grid_count, _tile_size);
	}

	void create_grids(int _grid_width, int _grid_height, int _grid_count, int _tile_size);

	// distance between a player to screen/projection plane
	static double screen_distance(double screen_width, double fov_radians);

	// relative agnle between player and a ray column strip
	static double strip_angle(double screen_x, double screen_distance);

	// calculate the screen height of a wall strip
	static double strip_screen_height(double screen_distance, double correct_distance, double tile_size);

	static bool is_wall_in_ray_hits(vector <ray_hit> &ray_hits, int cell_x, int cell_y);

	static vector <sprite*> find_sprites_in_cell(vector <sprite> &sprites, int cell_x, int cell_y, int tile_size);

	/*
		these functions look for collisions with walls and sprites, not the render function
		the collisions are stored in ray_hits
	*/
	void raycast(vector <ray_hit> &ray_hits, int player_x, int player_y, double player_rot, double strip_angle, int strip_idx);

	static void raycast(vector <ray_hit> &ray_hits, vector <vector <int>> &grids, int grid_width, int grid_height, int tile_size,
						int player_x, int player_y,
						double player_rot, double strip_angle,
						int strip_idx);

	static bool needs_next_wall(vector <vector <int>> &grids, int grid_width, int x, int y);

	int safe_cell_at(int x, int y){
		const int offset = x + y * grid_width;

		return grids[0][offset];
	}

	// reverse wall types (above 1000 for door detection)
	static bool is_horizontal_door(int wall_type){
		return (wall_type >= 1500);
	}
	static bool is_vertical_door(int wall_type){
		return (wall_type >= 1000 && wall_type < 1500);
	}
	static bool is_door(int wall_type){
		return (is_horizontal_door(wall_type) || is_vertical_door(wall_type));
	}

	static void find_intersecting_walls(vector <ray_hit> &ray_hits, vector <wall*> &walls,
												double player_x, double player_y, double ray_end_x, double ray_end_y);

	static void raycast_sprites(vector <ray_hit> &ray_hits, vector <vector <int>> &grids,
								int grid_width, int grid_height, int tile_size,
								double player_x, double player_y, double player_rot,
								double strip_angle, double strip_idx,
								vector <sprite> *sprites_to_look_for);
};

/*
	use to sort ray hits from furthest to nearest
	raycaster: the raycaster distance
	eye: distance between player's eye and ground
*/
struct ray_hit_sorter{
	raycaster *_raycaster;
	double eye;

	ray_hit_sorter(raycaster *__raycaster, double _eye){
		_raycaster = __raycaster;
		eye = _eye;
	}

	bool operator()(const ray_hit &x, const ray_hit &y) const;
};