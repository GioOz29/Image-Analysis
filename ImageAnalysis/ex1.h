#pragma once
#include <iostream>
#include <stdio.h>
#include<opencv2/opencv.hpp>


using namespace cv;
using namespace std;

Mat createHistrogramGray(Mat img);
Mat createHistrogramColor(Mat img, unsigned char channel);
Mat createGreyScaleImage(Mat img);

void helloWorld();
void part1(string file);
void part2(string file);
void part3_4_5();



































