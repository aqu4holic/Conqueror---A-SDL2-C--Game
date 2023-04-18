#pragma once

/*
	the long each raycast strip, the less rays with be used
	increase to boost fps, but reduce rendering quality
	takes value from 1 - 4
*/
const int DEFAULT_STRIP_WIDTH = 2;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const int DEFAULT_FOV_DEGRESS = 90;

const int MAP_WIDTH = 16;
const int MAP_HEIGHT = 16;

extern int g_map1[MAP_HEIGHT][MAP_WIDTH];
extern int g_map2[MAP_HEIGHT][MAP_WIDTH];
extern int g_map3[MAP_HEIGHT][MAP_WIDTH];

extern int g_floormap1[MAP_HEIGHT][MAP_WIDTH];
extern int g_floormap2[MAP_HEIGHT][MAP_WIDTH];
extern int g_floormap3[MAP_HEIGHT][MAP_WIDTH];

extern int g_ceilingmap1[MAP_HEIGHT][MAP_WIDTH];
extern int g_ceilingmap2[MAP_HEIGHT][MAP_WIDTH];
extern int g_ceilingmap3[MAP_HEIGHT][MAP_WIDTH];

extern int g_spritemap1[MAP_HEIGHT][MAP_WIDTH];
extern int g_spritemap2[MAP_HEIGHT][MAP_WIDTH];
extern int g_spritemap3[MAP_HEIGHT][MAP_WIDTH];