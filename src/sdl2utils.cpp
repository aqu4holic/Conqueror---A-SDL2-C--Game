#include "sdl2utils.h"

SDL_Color get_rgba_pixel_color(Uint8 *pixels, int x, int y, int w){
	SDL_Color color;

	color.b = pixels[4 * (x + y * w) + 0];
	color.g = pixels[4 * (x + y * w) + 1];
	color.r = pixels[4 * (x + y * w) + 2];
	color.a = pixels[4 * (x + y * w) + 3];

	return color;
}

Uint8 *copy_surface_from_pixels(SDL_Surface *surface, Uint32 pixel_format, SDL_Renderer *renderer, int *width, int *height, int *pitch){
	Uint8 *pixels = nullptr;
	SDL_Surface *tmp_surface = nullptr;
	SDL_Texture *texture = nullptr;
	int size_in_bytes = 0;

	tmp_surface = SDL_ConvertSurfaceFormat(surface, pixel_format, 0);
	if (tmp_surface){
		texture = SDL_CreateTexture(renderer, pixel_format, SDL_TEXTUREACCESS_STATIC, tmp_surface -> w, tmp_surface -> h);
	}

	if (texture){
		if (width){
			*width = tmp_surface -> w;
		}
		if (height){
			*height = tmp_surface -> h;
		}
		if (pitch){
			*pitch = tmp_surface -> pitch;
		}

		size_in_bytes = tmp_surface -> pitch * tmp_surface -> h;

		pixels = (Uint8*) malloc(size_in_bytes);

		memcpy(pixels, tmp_surface -> pixels, size_in_bytes);
	}

	if (texture){
		SDL_DestroyTexture(texture);
	}
	if (tmp_surface){
		SDL_FreeSurface(tmp_surface);
	}

	return pixels;
}

bool bitmap::load(const char *filename, SDL_Renderer *renderer, Uint32 pixel_format){
	destroy();

	surface_texture Surface_texture;

	if (Surface_texture.load_bitmap(filename)){
		void *tmp_pixels = copy_surface_from_pixels(Surface_texture.get_surface(), pixel_format, renderer, &width, &height, &pitch);

		if (tmp_pixels){
			int size_in_bytes = pitch * width;
			this -> pixels = malloc(size_in_bytes);
			memcpy(this -> pixels, tmp_pixels, size_in_bytes);

			return 1;
		}
	}

	return 0;
}