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

string path = "/Users/giovanniorzalesi/Desktop/IA ex1/ariane5_1b.jpg";
void imageinfo(Mat img);
Mat grey_image(Mat img);
Mat histogram(Mat img);
Mat histogram_gray(Mat img);

int main(){
    Mat img, gray, img_hist, gray_hist;

    img = imread(path);
    imageinfo(img);
    gray = grey_image(img);
    img_hist = histogram(img);
    gray_hist = histogram_gray(gray);

    while (1) { // Display all the colour values histogram
        imshow("Image", img);
        imshow("Grey", gray);
        imshow("Histogram", img_hist);
        imshow("Histogram Grey", gray_hist);
        char c = (char)waitKey(10);
        if (c == 13) break; //Press escape to stop program
    }
    imwrite("/Users/giovanniorzalesi/Desktop/IA ex1/gray.jpg", gray);
    return 0;
}

void imageinfo(Mat img){
    int width = img.rows;
    int height = img.cols;
    int dim = img.dims;
    int depth = img.depth();
    int type = img.type();
}

Mat grey_image(Mat img){
    int width = img.rows;
    int height = img.cols;
    Mat grey(width, height, CV_8UC1);   // Creating a new image with one channel

    int x, y, z;

    for(int i = 0; i<width;i++){
        for(int j = 0;j<height;j++){
            x = img.at<Vec3b>(i, j)[0];
            y = img.at<Vec3b>(i, j)[1];
            z = img.at<Vec3b>(i, j)[2];

            grey.at<uchar>(i,j) = (x + y + z) / 3;  
        }
    }
    return grey;
}

Mat histogram(Mat img){
    unsigned char value = 0; // index value for the histogram
    int histogram[256]; // histogram array - remember to set to zero initially
    int width = img.rows; 
    int height = img.cols;
    int k=256;
    int channel = img.dims;
    Mat image_hist(1024, 1024, CV_8UC3);    // I create the image to store the histogram
    
    for(int c = 0; c <= channel; c++){   // Loop over the channels (colours)

        while (k-- > 0)
        histogram[k] = 0; // reset histogram entry

        for(int i=0; i<height; i++){    // Create the Histogram values
            for(int j=0; j<width; j++){
                value = img.at<Vec3b>(i,j)[c];
                histogram[value] += 1;
            }
        }
        int maxval = 0;
        for(int i = 0; i < 256; i++){   // Scale the values
            if(maxval < histogram[i]) maxval = histogram[i];
        }

        for(int i=0; i < 256; i++){ // Create the lines for the histogram
            histogram[i] = (histogram[i] * 1024) / maxval;
            if(c == 0){
                rectangle(image_hist, Point(i*4, 1024), Point(i*4, 1024-histogram[i]), Scalar(255,0,0), 2); // img, start point, end point, color, thickness
            } else if(c==1){
                rectangle(image_hist, Point(i*4, 1024), Point(i*4, 1024-histogram[i]), Scalar(0,255,0), 2);
            } else if(c==2){
                rectangle(image_hist, Point(i*4, 1024), Point(i*4, 1024-histogram[i]), Scalar(0,0,255), 2);
            }
        }
    }
    return image_hist;
}

Mat histogram_gray(Mat img){
    unsigned char value = 0; // index value for the histogram
    int histogram[256]; // histogram array - remember to set to zero initially
    int width = img.rows; 
    int height = img.cols;
    int k=256;

    while (k-- > 0)
    histogram[k] = 0; // reset histogram entry

    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            value = img.at<uchar>(i,j);
            histogram[value] += 1;
        }
    }
    int maxval = 0;
    for(int i = 0; i < 256; i++){
        if(maxval < histogram[i]) maxval = histogram[i];
    }

    Mat image_hist(1024, 1024, CV_8UC1,Scalar(0));

    for(int i=0; i < 256; i++){
        histogram[i] = (histogram[i] * 1024) / maxval;
        rectangle(image_hist, Point(i*4, 1024), Point(i*4, 1024-histogram[i]), Scalar(255), 2); // img, start point, end point, color, thickness
    }
    return image_hist;
}

// cd build
// cmake ..
// make
// ./p1