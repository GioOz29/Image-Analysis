#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include<opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

using namespace cv;
using namespace std;

#define CV_8U   0
#define CV_8S   1
#define CV_16U  2
#define CV_16S  3
#define CV_32S  4
#define CV_32F  5
#define CV_64F  6
#define MAX_RAND 100

string path = "/Users/giovanniorzalesi/Desktop/IA ex4/PEN.png";
void imageinfo(Mat img);
Mat copy(Mat img);
Mat binary(Mat bin, unsigned char threshold);
void contourSearch(Mat pic, Point pos, int rimx[], int rimy[], int local_tresh);

int main(){
    Mat img, img_bin;
    int t = 100;
    Point spoint(300,300);
    int rimx[MAX_RAND];
    int rimy[MAX_RAND];
    int localThreshold = 128;

    img = imread(path, IMREAD_GRAYSCALE);   // Load as gray
    imageinfo(img);
    img_bin = copy(img);
    img_bin = binary(img_bin, t);

    contourSearch(img_bin, spoint, rimx, rimy, localThreshold);

    for (int i = 0; i < MAX_RAND; i++) {
        Point pt(rimx[i], rimy[i]);
        circle(img_bin, pt, 1, cv::Scalar(100), -1); // Draw a white point
    }

    // Display all
    while(1){
        imshow("Image", img);
        imshow("Bin", img_bin);
        char c = (char)waitKey(10);
        if (c == 27) break; //Press escape to stop program
    }
    return 0;
}

void imageinfo(Mat img){
    int width = img.rows;
    int height = img.cols;
    int dim = img.dims;
    int depth = img.depth();
    int type = img.type();
}

Mat copy(Mat img){
    Mat c = img.clone();
    return c;
}

Mat binary(Mat bin, unsigned char threshold){
    unsigned char value = 0;
    int width = bin.cols;
    int height = bin.rows;

    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            value = bin.at<uchar>(i,j);
            if(value > threshold){
                bin.at<uchar>(i,j) = 255;
            } else{
                bin.at<uchar>(i,j) = 0;
            }
        }
    }
    return bin;
}

void contourSearch(Mat pic, Point pos, int rimx[], int rimy[], int local_tresh) {
    int count = 0;
    Point newpos;
    int randx[MAX_RAND], randy[MAX_RAND];
    int draw_type = 0;
    newpos = pos;
    while (newpos.x >= 0 && newpos.y>=0 && newpos.x < pic.cols && newpos.y < pic.rows){
        // save current position in list
        rimx[count] = newpos.x;
        rimy[count] = newpos.y;
        count++;
        // Select next search direction
        draw_type = (draw_type + 6) % 8;
        switch (draw_type){
            case 0:
            if (pic.at<uchar>(newpos.y,newpos.x + 1)> local_tresh){
                newpos.x += 1;
                draw_type = 0;
                break;
            }
            case 1:
            if (pic.at<uchar>(newpos.y + 1, newpos.x + 1) > local_tresh){
                newpos.x += 1;
                newpos.y += 1;
                draw_type = 1;
                break;
            }
            case 2:
            if (pic.at<uchar>(newpos.y + 1, newpos.x) > local_tresh){
                newpos.y += 1;
                draw_type = 2;
                break;
            }
            case 3: if (pic.at<uchar>(newpos.y +1, newpos.x - 1) > local_tresh){
                newpos.x -= 1;
                newpos.y += 1;
                draw_type = 3;
                break;
            }
            case 4:
            if (pic.at<uchar>(newpos.y, newpos.x - 1) > local_tresh){
                newpos.x -= 1;
                draw_type = 4;
                break;
            }
            case 5:
            if (pic.at<uchar>(newpos.y - 1, newpos.x - 1) > local_tresh){
                newpos.x -= 1;
                newpos.y -= 1;
                draw_type = 5;
                break;
            }
            case 6:
            if (pic.at<uchar>(newpos.y - 1, newpos.x) > local_tresh){
                newpos.y -= 1;
                draw_type = 6;
                break;
            }
            case 7:
            if (pic.at<uchar>(newpos.y - 1, newpos.x + 1) > local_tresh){
                newpos.x += 1;
                newpos.y -= 1;
                draw_type = 7;
                break;
            }
            case 8:
            if (pic.at<uchar>(newpos.y, newpos.x + 1) > local_tresh){
                newpos.x += 1;
                draw_type = 0;
                break;
            }
            case 9:
            if (pic.at<uchar>(newpos.y + 1, newpos.x + 1) > local_tresh){
                newpos.x += 1;
                newpos.y += 1;
                draw_type = 1;
                break;
            }
            case 10:
            if (pic.at<uchar>(newpos.y + 1, newpos.x) > local_tresh){
                newpos.y += 1;
                draw_type = 2;
                break; 
            }
            case 11:
            if (pic.at<uchar>(newpos.y + 1, newpos.x - 1) > local_tresh){
                newpos.x -= 1;
                newpos.y += 1;
                draw_type = 3;
            break;
            }
            case 12:
            if (pic.at<uchar>(newpos.y, newpos.x - 1) > local_tresh){
                newpos.x -= 1;
                draw_type = 4;
            break;
            }
            case 13:
            if (pic.at<uchar>(newpos.y - 1, newpos.x - 1) > local_tresh){
                newpos.x -= 1;
                newpos.y -= 1;
                draw_type = 5;
                break;
            }
            case 14:
            if (pic.at<uchar>(newpos.y - 1, newpos.x) > local_tresh){
                newpos.y -= 1;
                draw_type = 6;
                break;
            }
        }
        // If we are back at the beginning, we declare success
        if (newpos.x == pos.x && newpos.y == pos.y)
        break;
        // Abort if the contour is too complex.
        if (count >= MAX_RAND)
        break;
    }
}

// cd build
// cmake ..
// make
// ./p4