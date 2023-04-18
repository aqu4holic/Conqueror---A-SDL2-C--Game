#pragma once

struct point{
	double x, y;

	point(float _x = 0, float _y = 0){
		x = _x;
		y = _y;
	}
};

struct shape{
	static bool lines_intersect(double x1, double y1, double x2, double y2,
								double x3, double y3, double x4, double y4,
								double *ix, double *iy);
	
	static bool point_in_rect(double pt_x, double pt_y, double x, double y, double w, double h);
};