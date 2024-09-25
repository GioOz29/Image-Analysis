#include "ex2.h"


Mat loadPenImage() {
  return imread("test.jpg");
}

Mat createBinaryImage(Mat &img, unsigned char threshold) {

  unsigned char val;
  for (int i = 0; i < img.rows; i++) {
	for (int j = 0; j < img.cols; j++) {
	  val = img.at<uchar>(i, j);
	  if (val > threshold) {
		img.at<uchar>(i, j) = 0;
	  }
	  else {
		img.at<uchar>(i, j) = 255;
	  }
	}
  }
  return img;
}


void centerOfMass(Mat img, double com[2]) {
  double m[2][2];
  long double sum;
  for (int k = 0; k < 2; k++) {
	for (int p = 0; p < 2; p++) {
	  sum = 0;
	  for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
		  sum += pow(j, k) * pow(i, p) *img.at<uchar>(i, j);
		 
		}
	  }
	  m[k][p] = sum;
	}
  }
  com[0] = m[1][0] / m[0][0];
  com[1] = m[0][1] / m[0][0];
}

void findCentralMoments(Mat img, double com[2], double m[][3]) {
  long double sum;
  for (int k = 0; k < 3; k++) {
	for (int p = 0; p < 3; p++) {
	  sum = 0;	  
	  for (int j = 0; j < img.cols; j++) {
		for (int i = 0; i < img.rows; i++) {
		  sum += pow(j- com[0], k) * pow(i - com[1], p) * img.at<uchar>(i, j);
		}
	  }
	  if (k > 0 || p > 0) {
		m[k][p] = sum/m[0][0];
	  }
	  else {
		m[k][p] = sum;
	  }
	}
  }
}

void drawCross(Mat &img, Point p) {
  Point start, end;
  if (p.x - 5 > 0 && p.y - 5 > 0 && p.x + 5 < img.cols && p.y + 5 < img.rows) {
	start.x = p.x - 5;
	start.y = p.y - 5;
	end.x = p.x + 5;
	end.y = p.y + 5;
	line(img, start, end, Scalar(0, 0, 255),2);
	start.x = p.x - 5;
	start.y = p.y + 5;
	end.x = p.x + 5;
	end.y = p.y - 5;
	line(img, start, end, Scalar(0, 0, 255),2);

  }
  else {
	cout << "cross is too close to edge of image";
  }
}


void ex2Part1() {
  Mat img = loadPenImage();
  Mat grey = createGreyScaleImage(img);
  Mat histo = createHistrogramGray(grey);

  Mat grey_ori = grey;

  unsigned char threshold = 120;
  createBinaryImage(grey, threshold);


  namedWindow("ShowImage");
  namedWindow("ShowImage_ori");
  namedWindow("Histogram");

  imshow("ShowImage_ori", grey_ori);
  imshow("ShowImage", grey);
  imshow("Histogram", histo);
  waitKey(0);

}

void ex2Part2() {
  Mat img = loadPenImage();
  Mat grey = createGreyScaleImage(img);

  unsigned char threshold = 70;
  createBinaryImage(grey, threshold);
  imshow("Greyscale", grey);
  waitKey(0);

  double com[2];
  centerOfMass(grey, com);
 
  Point p;
  p.x = com[0];
  p.y = com[1];
  drawCross(img, p);

  imshow("CenterOfMass", img);
  waitKey(0);
}


void ex2Part3() {
  Mat img = loadPenImage();
  Mat grey = createGreyScaleImage(img);

  unsigned char threshold = 70;
  createBinaryImage(grey, threshold);
  imshow("Greyscale", grey);
  waitKey(0);

  double m[3][3];
  double com[2];
  centerOfMass(grey, com);
  findCentralMoments(grey, com, m);

  double angle = 2 * m[1][1] / (m[2][0] - m[0][2]);

  Point p;
  p.x = com[0];
  p.y = com[1];

  drawCross(img, p);

  cout << "\n" << angle;
  angle = atan(angle) / 2;
  cout << "\n" << angle;

  Point start;
  Point end;
  int length_of_line = 500;
  start.x = p.x + length_of_line/2 * sin(angle);
  start.y = p.y - length_of_line / 2 * cos(angle);
  end.x = p.x - length_of_line / 2 * sin(angle);
  end.y = p.y + length_of_line / 2 * cos(angle);
  line(img, start, end, Scalar(255, 0, 0), 2);

  imshow("CenterOfMass", img);
  waitKey(0);
}