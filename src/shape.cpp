#include "shape.h"

using namespace std;

// http://paulbourke.net/geometry/pointlineplane/javascript.txt
bool shape::lines_intersect(double x1, double y1, double x2, double y2,
								double x3, double y3, double x4, double y4,
								double *ix, double *iy){
	// check if lines are length 0
	if ((x1 == x2 && y1 == y2) || (x3 == x4 && y3 == y4)){
		return 0;
	}

	double denominator = ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));

	// lines parallel
	if (denominator == 0.0){
		return 0;
	}

	double ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denominator;
  	double ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denominator;

	// is the intersection along the segments
	if (ua < 0 || ua > 1 || ub < 0 || ub > 1){
		return 0;
	}

	float x = x1 + ua * (x2 - x1);
	float y = y1 + ua * (y2 - y1);

	if (ix) {
		*ix = x;
	}
	if (iy) {
		*iy = y;
	}
	return 1;
}

bool shape::point_in_rect(double pt_x, double pt_y, double x, double y, double w, double h){
	return (x <= pt_x && pt_x <= (x + w) && y <= pt_y && pt_y <= (y + h));
}