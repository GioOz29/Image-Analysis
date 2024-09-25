#include "functions.cpp"
#include "Kernel&Filters.cpp"

// FEATURE TRANSFORMATION
Mat grey_image(Mat img);

Mat createHistrogramGray(Mat img);

Mat binary(Mat bin, unsigned char thershold);

vector<RotatedRect> filterEllipses(const vector<vector<Point>>& contours);

void drawEllipses(Mat& image, const vector<RotatedRect>& ellipses);

// CAMERA FUNCTIONS