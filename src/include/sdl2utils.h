#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

using namespace std;

Uint8 *copy_surface_from_pixels(SDL_Surface *surface, Uint32 pixel_format, SDL_Renderer *renderer, int *width, int *height, int *pitch);

SDL_Color get_rgba_pixel_color(Uint8 *pixels, int x, int y, int w);

class surface_texture{
public:
	surface_texture(){
		surface = nullptr;
		texture = nullptr;
	}
	~surface_texture(){
		destroy();
	}

	SDL_Surface *get_surface(){
		return surface;
	}
	SDL_Texture *get_texture(){
		return texture;
	}

	void destroy(){
		if (surface){
			SDL_FreeSurface(surface);
			surface = nullptr;
		}

		if (texture){
			SDL_DestroyTexture(texture);
			texture = nullptr;
		}
	}

	SDL_Surface *load_bitmap(const char *filename){
		destroy();

		surface = SDL_LoadBMP(filename);

		return surface;
	}

	SDL_Surface *load_image(const char *filename){
		destroy();

		surface = IMG_Load(filename);

		return surface;
	}

	SDL_Texture *create_texture(SDL_Renderer *renderer){
		if (texture){
			SDL_DestroyTexture(texture);
			texture = nullptr;
		}
		if (surface){
			texture = SDL_CreateTextureFromSurface(renderer, surface);
		}

		return texture;
	}
private:
	SDL_Surface *surface;
	SDL_Texture *texture;
};

class bitmap{
public:
	bitmap(){
		width = height = pitch = 0;

		pixels = nullptr;
	}
	~bitmap(){
		destroy();
	}

	void destroy(){
		if (pixels){
			free(pixels);

			pixels = nullptr;
		}
	}

	void *get_pixels(){
		return pixels;
	}
	int get_width() const{
		return width;
	}
	int get_height() const{
		return height;
	}
	int get_pitch() const{
		return pitch;
	}

	bool load(const char *filename, SDL_Renderer *renderer, Uint32 pixel_format);
private:
	int width, height, pitch;
	void *pixels;
};