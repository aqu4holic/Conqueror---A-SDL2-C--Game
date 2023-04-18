#include "bits/stdc++.h"
#include "raycasting.h"

using namespace std;

const double TWO_PI = 2 * M_PI;

thin_wall::thin_wall(){
	x1 = y1 = x2 = y2 = 0;
	wall_type = 0;
	horizontal = 0;
	height = 0;
	hidden = 0;
	Thick_wall = 0;
}

thin_wall::thin_wall(double x1, double y1, double x2, double y2, int wall_type, thick_wall *Thick_wall, double wall_height){
	this -> x1 = x1;
	this -> y1 = y1;
	this -> x2 = x2;
	this -> y2 = y2;
	this -> wall_type = wall_type;
	horizontal = 0;
	height = 0;
	z = 0;
	hidden = 0;
	this -> Thick_wall = Thick_wall;
}

double thin_wall::distance_to_origin(double ix, double iy){
	return sqrt((x1 - ix) * (x1 - ix) + (y1 - iy) * (y1 - iy));
}

thick_wall::thick_wall(){
	type = 0;
	height = 0;
	x = y = w = h = 0;
	ceiling_texture_id = 0;
	floor_texture_id = 0;
}

void thick_wall::set_height(double height){
	this -> height = height;

	for (int i = 0; i < (int) thin_walls.size(); ++i){
		thin_walls[i].height = height;
	}
}

void thick_wall::set_thin_walls_type(int wall_type){
	for (int i = 0; i < (int) thin_walls.size(); ++i){
		thin_walls[i].wall_type = wall_type;
	}
}

bool thick_wall::contains_point(double x, double y){
	return shape::point_in_rect(x, y, this -> x, this -> y, this -> w, this -> h);
}

bool ray_hit::same_ray_hit(const ray_hit &Ray_hit2){
	const ray_hit &Ray_hit = *this;

	return (Ray_hit.x == Ray_hit2.x && Ray_hit.y == Ray_hit2.y && Ray_hit.wall_type == Ray_hit2.wall_type
			&& Ray_hit.strip == Ray_hit2.strip && Ray_hit.Thin_wall == Ray_hit2.Thin_wall && Ray_hit.ray_angle == Ray_hit2.ray_angle);
}

bool ray_hit_sorter::operator()(const ray_hit &a, const ray_hit &b) const{
	// if either wall is a thin_wall, just use the distance of it
	if (a.Thin_wall || b.Thin_wall){
		return (a.distance > b.distance);
	}

	/*
		sort by distance between player's eye to the wall below
		furthest wall drawn first
	*/
	double v_distance_to_eye_a = eye;
	double v_distance_to_eye_b = eye;

	double dis_a = (a.sort_distance ? a.sort_distance : a.distance);
	double dis_b = (b.sort_distance ? b.sort_distance : b.distance);

	double distance_to_wall_base_a = v_distance_to_eye_a * v_distance_to_eye_a + dis_a * dis_a;
	double distance_to_wall_base_b = v_distance_to_eye_b * v_distance_to_eye_b + dis_b * dis_b;

	return distance_to_wall_base_a > distance_to_wall_base_b;
}

void raycaster::create_grids(int grid_width, int grid_height, int grid_count, int tile_size){
	this -> grid_width = grid_width;
	this -> grid_height = grid_height;
	this -> grid_count = grid_count;
	this -> tile_size = tile_size;

	grids.clear();

	for (int i = 0; i < grid_count; ++i){
		vector <int> grid;
		grid.resize(grid_width * grid_height, 0);

		grids.push_back(grid);
	}
}

// demo pic of 2 functions below in img/demo
double raycaster::screen_distance(double screen_width, double fov_radians){
	return (screen_width / 2) / tan(fov_radians / 2);
}

double raycaster::strip_angle(double screen_x, double screen_distance){
	return atan(screen_x / screen_distance);
}

double raycaster::strip_screen_height(double screen_distance, double correct_distance, double tile_size){
	// use +0.5 to round
	return int(screen_distance / correct_distance * tile_size + 0.5);
}

bool raycaster::is_wall_in_ray_hits(vector <ray_hit> &ray_hits, int cell_x, int cell_y){
	for (__typeof(ray_hits.begin()) it = ray_hits.begin(); it != ray_hits.end(); ++it){
		if ((*it).wall_type && (*it).wall_x == cell_x && (*it).wall_y == cell_y){
			return 1;
		}
	}

	return 0;
}

vector <sprite*> raycaster::find_sprites_in_cell(vector <sprite> &sprites, int cell_x, int cell_y, int tile_size){
	vector <sprite*> sprites_found = {};

	for (__typeof(sprites.begin()) it = sprites.begin(); it != sprites.end(); ++it){
		sprite *s = &*it;

		if (cell_x == int(s -> x / tile_size) && (cell_y == int(s -> y / tile_size))){
			sprites_found.push_back(s);
		}
	}

	return sprites_found;
}

bool raycaster::needs_next_wall(vector <vector <int>> &grids, int tile_size, int grid_width, int x, int y){
	vector <int> &grid = grids[0];
	if (x && y){
		if (is_door(grid[x + y * grid_width])){
			return 1;
		}
	}

	return 0;
}

void raycaster::raycast(vector <ray_hit> &ray_hits, int player_x, int player_y,
						double player_rot, double strip_angle, int strip_idx,
						vector <sprite> *sprites_to_look_for){
	raycaster::raycast(ray_hits, this -> grids, this -> grid_width, this -> grid_height, this -> tile_size,
						player_x, player_y, player_rot,
						strip_angle, strip_idx,
						sprites_to_look_for);
}

void raycaster::raycast(vector <ray_hit> &ray_hits, vector <vector <int>> &grids, int grid_width, int grid_height, int tile_size,
						int player_x, int player_y,
						double player_rot, double strip_angle,
						int strip_idx,
						vector <sprite> *sprites_to_look_for){
	if (!grids.size()){
		return;
	}

	double ray_angle = strip_angle + player_rot;
	while(ray_angle < 0 || ray_angle >= TWO_PI){
		ray_angle += (ray_angle < 0 ? TWO_PI : (ray_angle >= TWO_PI ? -TWO_PI : 0));
	}

	bool right = (ray_angle < 0.25 * TWO_PI && ray_angle >= 0) || // quad 1
				(ray_angle > 0.75 * TWO_PI); // quad 4
	bool up = (ray_angle < TWO_PI * 0.5 && ray_angle >= 0); // quad 1 and 2

	vector <int> &ground_grid = grids[0];

	int current_tile_x = player_x / tile_size;
	int current_tile_y = player_y / tile_size;

	for (int level = 0; level < (int) grids.size(); ++level){
		vector <int> &grid = grids[level];
		
		// check player's current tile for sprites
		vector <sprite*> sprites_hit;
		if (sprites_to_look_for){
			vector <sprite*> sprites_found = find_sprites_in_cell(*sprites_to_look_for, current_tile_x, current_tile_y, tile_size);

			for (int i = 0; i < (int) sprites_found.size(); ++i){
				sprite *s = sprites_found[i];

				if (!s -> Ray_hit){
					const double dis_x = player_x - s -> x;
					const double dis_y = player_y - s -> y;
					const double block_dis = dis_x * dis_x + dis_y * dis_y;

					if (block_dis){
						s -> Ray_hit = 1;
						s -> distance = sqrt(block_dis);

						sprites_hit.push_back(s);
					}
				}
			}
		}

		// vertical lines check
		double vertical_line_distance = 0;
		ray_hit verical_wall_hit;

		// find x coord of vertical lines on the right and the left
		double vx = int(player_x / tile_size) * tile_size + (right ? tile_size : -1);
		// find y coord of those lines
		// line_y = player_y + (player_x - line_x) * tan(alpha)
		double vy = player_y + (player_x - vx) * tan(ray_angle);

		// calculate stepping vector for each line
		double step_x = (right ? tile_size : -tile_size);
		double step_y = tile_size * tan(ray_angle) * (right ? -1.0 : 1.0);

		bool prev_gaps = 0;
		while(vx >= 0 && vx < grid_width * tile_size && vy >= 0 && vy < grid_height * tile_size){
			int wall_x = int(vx / tile_size);
			int wall_y = int(vy / tile_size);
			int wall_offset = wall_x + wall_y * grid_width;

			// look for sprites in current cell
			if (sprites_to_look_for){
				vector <sprite*> sprites_found = find_sprites_in_cell(*sprites_to_look_for, wall_x, wall_y, tile_size);

				for (int i = 0; i < (int) sprites_found.size(); ++i){
					sprite *s = sprites_found[i];

					if (!s -> Ray_hit){
						const double dis_x = player_x - s -> x;
						const double dis_y = player_y - s -> y;
						const double block_dis = dis_x * dis_x + dis_y * dis_y;
						s -> distance = sqrt(block_dis);
						sprites_hit.push_back(s);

						ray_hit sprite_ray_hit(vx, vy, ray_angle);
						sprite_ray_hit.strip = strip_idx;
						if (s -> distance){
							sprite_ray_hit.distance = s -> distance;
							sprite_ray_hit.correct_distance = sprite_ray_hit.distance * cos(strip_angle);
						}
						sprite_ray_hit.wall_type = 0;
						sprite_ray_hit.Sprite = s;
						sprite_ray_hit.distance = s -> distance;
						s -> Ray_hit = 1;

						ray_hits.push_back(sprite_ray_hit);
					}
				}
			}

			// check if current cell is a wall
			if (grid[wall_offset] && !is_horizontal_door(grid[wall_offset])){
				double dis_x = player_x - vx;
				double dis_y = player_y - vy;
				double block_dis = dis_x * dis_x + dis_y * dis_y;

				if (block_dis){
					double tex_x = fmod(vy, tile_size);
					tex_x = (right ? tex_x : tile_size - tex_x); // if facing left, flip image

					ray_hit Ray_hit(vx, vy, ray_angle);
					Ray_hit.strip = strip_idx;
					Ray_hit.wall_type = grid[wall_offset];
					Ray_hit.wall_x = wall_x;
					Ray_hit.wall_y = wall_y;
					Ray_hit.right = right;
					Ray_hit.up = up;
					Ray_hit.distance = sqrt(block_dis);
					Ray_hit.sort_distance = Ray_hit.distance;
					
					bool can_add = 1;
					// if a door, render half of rays inside
					if (is_vertical_door(grid[wall_offset])){
						int new_wall_x = int((vx + step_x / 2) / tile_size);
						int new_wall_y = int((vy + step_y / 2) / tile_size);

						if (new_wall_x == wall_x && new_wall_y == wall_y){
							double half_distance_squared = (step_x / 2) * (step_x / 2) + (step_y / 2) * (step_y / 2);
							double half_distance = sqrt(half_distance_squared);
							Ray_hit.distance += half_distance;
							tex_x = fmod(vy + step_y / 2, tile_size);

							// give doors lower drawing priority to prevent the wall above drawing its bottom surface later than the door
							Ray_hit.sort_distance -= 1;
						}
						else{
							can_add = 0;
						}
					}
					Ray_hit.correct_distance = Ray_hit.distance * cos(strip_angle);
					Ray_hit.horizontal = 0;
					Ray_hit.tile_x = tex_x;

					bool gaps = needs_next_wall(grids, tile_size, grid_width, wall_x, wall_y);
					// there is an empty space before this wall
					if (gaps){
						prev_gaps = gaps; // for next wall check
					}
					else{
						verical_wall_hit = Ray_hit;
						vertical_line_distance = block_dis;

						break;
					}

					if (can_add){
						ray_hits.push_back(Ray_hit);
					}
				}
			}

			vx += step_x;
			vy += step_y;
		}

		// horizontal lines check
		double horizontal_line_distance = 0;
		// find y coord of horizontal lines on the left and right
		double hy = int(player_y / tile_size) * tile_size + (up ? -1 : tile_size);
		// calculate x coord of horizontal line
		// line_x = player_x + (player_y - line_y) / tan(alpha)
		double hx = player_x + (player_y - hy) / tan(ray_angle);
		step_x = tile_size / tan(ray_angle) * (up ? 1 : -1);
		step_y = (up ? -tile_size : tile_size);

		prev_gaps = 0;
		while(hx >= 0 && hx < grid_width * tile_size && hy >= 0 && hy < grid_height * tile_size){
			int wall_x = int(hx / tile_size);
			int wall_y = int(hy / tile_size);
			int wall_offset = wall_x + wall_y * grid_width;

			// look for sprites in current cell
			if (sprites_to_look_for){
				vector <sprite*> sprites_found = find_sprites_in_cell(*sprites_to_look_for, wall_x, wall_y, tile_size);

				for (int i = 0; i < (int) sprites_found.size(); ++i){
					sprite *s = sprites_found[i];

					if (!s -> Ray_hit){
						const double dis_x = player_x - s -> x;
						const double dis_y = player_y - s -> y;
						const double block_dis = dis_x * dis_x + dis_y * dis_y;
						s -> distance = sqrt(block_dis);
						sprites_hit.push_back(s);

						ray_hit sprite_ray_hit(vx, vy, ray_angle);
						sprite_ray_hit.strip = strip_idx;
						if (s -> distance){
							sprite_ray_hit.distance = s -> distance;
							sprite_ray_hit.correct_distance = sprite_ray_hit.distance * cos(strip_angle);
						}
						sprite_ray_hit.wall_type = 0;
						sprite_ray_hit.Sprite = s;
						sprite_ray_hit.distance = s -> distance;
						s -> Ray_hit = 1;

						ray_hits.push_back(sprite_ray_hit);
					}
				}
			}

			// check if current cell is a wall
			if (grid[wall_offset] && !is_vertical_door(grid[wall_offset])){
				double dis_x = player_x - hx;
				double dis_y = player_y - hy;
				double block_dis = dis_x * dis_x + dis_y * dis_y;

				// if vertical distance is less than horizontal line distance, stop
				if (vertical_line_distance > 0 && vertical_line_distance < block_dis){
					// unless there was some space below previous wall
					if (!prev_gaps){
						break;
					}
				}

				if (block_dis){
					double tex_x = fmod(hx, tile_size);
					tex_x = (up ? tex_x : tile_size - tex_x); // if facing left, flip image

					ray_hit Ray_hit(hx, hy, ray_angle);
					Ray_hit.strip = strip_idx;
					Ray_hit.wall_type = grid[wall_offset];
					Ray_hit.wall_x = wall_x;
					Ray_hit.wall_y = wall_y;
					Ray_hit.right = right;
					Ray_hit.up = up;
					Ray_hit.distance = sqrt(block_dis);
					Ray_hit.sort_distance = Ray_hit.distance;
					
					bool can_add = 1;
					// if a door, render half of rays inside
					if (is_horizontal_door(grid[wall_offset])){
						int new_wall_x = int((hx + step_x / 2) / tile_size);
						int new_wall_y = int((hy + step_y / 2) / tile_size);

						if (new_wall_x == wall_x && new_wall_y == wall_y){
							double half_distance_squared = (step_x / 2) * (step_x / 2) + (step_y / 2) * (step_y / 2);
							double half_distance = sqrt(half_distance_squared);
							Ray_hit.distance += half_distance;
							tex_x = fmod(hx + step_x / 2, tile_size);

							// give doors lower drawing priority to prevent the wall above drawing its bottom surface later than the door
							Ray_hit.sort_distance -= 1;
						}
						else{
							can_add = 0;
						}
					}
					Ray_hit.correct_distance = Ray_hit.distance * cos(strip_angle);
					Ray_hit.horizontal = 1;
					Ray_hit.tile_x = tex_x;
					horizontal_line_distance = block_dis;

					if (can_add){
						ray_hits.push_back(Ray_hit);
					}

					bool gaps = needs_next_wall(grids, tile_size, grid_width, wall_x, wall_y);
					// there is an empty space before this wall
					if (gaps){
						// add the previous vertical line if exists
						if (vertical_line_distance){
							ray_hits.push_back(verical_wall_hit);

							vertical_line_distance = 0;
						}
						prev_gaps = gaps; // for next wall check
					}
					else{
						break;
					}
				}
			}

			hx += step_x;
			hy += step_y;
		}

		// if no horizontal line was found but a vertical line was
		if (!horizontal_line_distance && vertical_line_distance){
			ray_hits.push_back(verical_wall_hit);
		}
	}
}

void raycaster::find_intersecting_thin_walls(vector <ray_hit> &ray_hits, vector <thin_wall*> &thin_walls,
											double player_x, double player_y, double ray_end_x, double ray_end_y){
	for (int i = 0; i < (int) thin_walls.size(); ++i){
		thin_wall &Thin_wall = *thin_walls[i];

		double ix = 0, iy = 0;

		bool hit_found = shape::lines_intersect(Thin_wall.x1, Thin_wall.y1, Thin_wall.x2, Thin_wall.y2,
												player_x, player_y,
												ray_end_x, ray_end_y,
												&ix, &iy);

		if (hit_found){
			ray_hit Ray_hit;
			double dis_x = player_x - ix;
			double dis_y = player_y - iy;
			double squared_distance = dis_x * dis_x + dis_y * dis_y;
			Ray_hit.squared_distance = squared_distance;
			Ray_hit.distance = sqrt(Ray_hit.squared_distance);
			if (Ray_hit.distance){
				Ray_hit.Thin_wall = &Thin_wall;
				Ray_hit.x = ix;
				Ray_hit.y = iy;

				ray_hits.push_back(Ray_hit);
			}
		}
	}
}

void raycaster::raycast_thin_walls(vector <ray_hit> &ray_hits, vector <thin_wall*> &thin_walls,
								double player_x, double player_y, double player_rot,
								double strip_angle, int strip_idx){
	double ray_angle = strip_angle + player_rot;
	while(ray_angle < 0 || ray_angle >= TWO_PI){
		ray_angle += (ray_angle < 0 ? TWO_PI : (ray_angle >= TWO_PI ? -TWO_PI : 0));
	}

	bool right = (ray_angle < 0.25 * TWO_PI && ray_angle >= 0) || // quad 1
				(ray_angle > 0.75 * TWO_PI); // quad 4

	double vx = (right ? grid_width * tile_size : 0);
	double vy = player_y + (player_x - vx) * tan(ray_angle);

	vector <ray_hit> new_ray_hits;
	vector <ray_hit*> added_ray_hits;

	find_intersecting_thin_walls(new_ray_hits, thin_walls, player_x, player_y, vx, vy);

	for (int i = 0; i < (int) new_ray_hits.size(); ++i){
		ray_hit &Ray_hit = new_ray_hits[i];
		thin_wall *Thin_wall = Ray_hit.Thin_wall;
		thick_wall *Thick_wall = Thin_wall -> Thick_wall;

		Ray_hit.wall_height = Thin_wall -> height;
		Ray_hit.strip = strip_idx;
		double dto = int(Thin_wall -> distance_to_origin(Ray_hit.x, Ray_hit.y) + 0.5);
		Ray_hit.tile_x = (int) dto % this -> tile_size;
		Ray_hit.horizontal = Thin_wall -> horizontal;
		Ray_hit.wall_type = Thin_wall -> wall_type;
		Ray_hit.ray_angle = ray_angle;
	
		if (Ray_hit.distance){
			Ray_hit.correct_distance = Ray_hit.distance * cos(player_rot - ray_angle);

			if (Ray_hit.correct_distance < 1){
				continue;
			}

			added_ray_hits.push_back(&Ray_hit);
		}
	}

	for (int i = 0; i < (int) added_ray_hits.size(); ++i){
		ray_hits.push_back(*added_ray_hits[i]);
	}
}

void raycaster::raycast_sprites(vector <ray_hit> &ray_hits, vector <vector <int>> &grids,
								int grid_width, int grid_height, int tile_size,
								double player_x, double player_y, double player_rot,
								double strip_angle, double strip_idx,
								vector <sprite> *sprites_to_look_for){
	if (!grids.size()){
		return;
	}

	double ray_angle = strip_angle + player_rot;
	while(ray_angle < 0 || ray_angle >= TWO_PI){
		ray_angle += (ray_angle < 0 ? TWO_PI : (ray_angle >= TWO_PI ? -TWO_PI : 0));
	}

	bool right = (ray_angle < 0.25 * TWO_PI && ray_angle >= 0) || // quad 1
				(ray_angle > 0.75 * TWO_PI); // quad 4
	bool up = (ray_angle < TWO_PI * 0.5 && ray_angle >= 0); // quad 1 and 2

	vector <int> &ground_grid = grids[0];

	int current_tile_x = player_x / tile_size;
	int current_tile_y = player_y / tile_size;

	// check player's current tile for sprites
	vector <sprite*> sprites_hit;
	vector <sprite*> sprites_found = find_sprites_in_cell(*sprites_to_look_for, current_tile_x, current_tile_y, tile_size);

	for (int i = 0; i < (int) sprites_found.size(); ++i){
		sprite *s = sprites_found[i];

		if (!s -> Ray_hit){
			const double dis_x = player_x - s -> x;
			const double dis_y = player_y - s -> y;
			const double block_dis = dis_x * dis_x + dis_y * dis_y;

			if (block_dis){
				s -> Ray_hit = 1;
				s -> distance = sqrt(block_dis);

				sprites_hit.push_back(s);
			}
		}
	}

	// vertical lines check
	ray_hit verical_wall_hit;
	// find x coord of vertical lines on the left and right
	double vx = int(player_x / tile_size) * tile_size + (right ? tile_size : -1);
	// calculate y coord of those lines
	// line_y = player_y + (player_x - line_x) * tan(alpha)
	double vy = player_y + (player_x - vx) * tan(ray_angle);

	// calculate stepping vector for each line
	double step_x = (right ? tile_size : -tile_size);
	double step_y = tile_size * tan(ray_angle) * (right ? -1 : 1);

	while(vx >= 0 && vx < grid_width * tile_size && vy >= 0 && vy < grid_height * tile_size){
		int wall_x = int(vx / tile_size);
		int wall_y = int(vy / tile_size);

		vector <sprite*> sprites_found = find_sprites_in_cell(*sprites_to_look_for, wall_x, wall_y, tile_size);

		for (int i = 0; i < (int) sprites_found.size(); ++i){
			sprite *s = sprites_found[i];

			if (!s -> Ray_hit){
				const double dis_x = player_x - s -> x;
				const double dis_y = player_y - s -> y;
				const double block_dis = dis_x * dis_x + dis_y * dis_y;
				s -> distance = sqrt(block_dis);
				sprites_hit.push_back(s);

				ray_hit sprite_ray_hit(vx, vy, ray_angle);
				sprite_ray_hit.strip = strip_idx;
				if (s -> distance){
					sprite_ray_hit.distance = s -> distance;
					sprite_ray_hit.correct_distance = sprite_ray_hit.distance * cos(strip_angle);
				}
				sprite_ray_hit.wall_type = 0;
				sprite_ray_hit.Sprite = s;
				sprite_ray_hit.distance = s -> distance;
				s -> Ray_hit = 1;

				ray_hits.push_back(sprite_ray_hit);
			}
		}

		vx += step_x;
		vy += step_y;
	}

	// horizontal lines check
	double hy = int(player_y / tile_size) * tile_size + (up ? -1 : tile_size);
	double hx = player_x + (player_y - hy) / tan(ray_angle);
	step_x = tile_size / tan(ray_angle) * (up ? 1 : -1);
	step_y = (up ? -tile_size : tile_size);

	while(hx >= 0 && hx < grid_width * tile_size && hy >= 0 && hy < grid_height * tile_size){
		int wall_x = int(hx / tile_size);
		int wall_y = int(hy / tile_size);

		vector <sprite*> sprites_found = find_sprites_in_cell(*sprites_to_look_for, wall_x, wall_y, tile_size);

		for (int i = 0; i < (int) sprites_found.size(); ++i){
			sprite *s = sprites_found[i];

			if (!s -> Ray_hit){
				const double dis_x = player_x - s -> x;
				const double dis_y = player_y - s -> y;
				const double block_dis = dis_x * dis_x + dis_y * dis_y;
				s -> distance = sqrt(block_dis);
				sprites_hit.push_back(s);

				ray_hit sprite_ray_hit(hx, hy, ray_angle);
				sprite_ray_hit.strip = strip_idx;
				if (s -> distance){
					sprite_ray_hit.distance = s -> distance;
					sprite_ray_hit.correct_distance = sprite_ray_hit.distance * cos(strip_angle);
				}
				sprite_ray_hit.wall_type = 0;
				sprite_ray_hit.Sprite = s;
				sprite_ray_hit.distance = s -> distance;
				s -> Ray_hit = 1;

				ray_hits.push_back(sprite_ray_hit);
			}
		}

		hx += step_x;
		hy += step_y;
	}
}