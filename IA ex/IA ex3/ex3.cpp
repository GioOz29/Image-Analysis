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

string path = "/Users/giovanniorzalesi/Desktop/IA ex3/PEN.png";
void imageinfo(Mat img);
Mat copy(Mat img);
Mat lowpass_filter(Mat img);

int main(){
    Mat img, img_low;

    img = imread(path, IMREAD_GRAYSCALE);   // Load as gray
    imageinfo(img);
    img_low = lowpass_filter(img);
    Mat img_high = img-img_low;

    // Display all
    while(1){
        imshow("Image", img);
        imshow("Low", img_low);
        imshow("High", img_high);
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

Mat lowpass_filter(Mat img){
    // Variables of the filter
    int n = 5;
    int kernel_size = 2 * n + 1;
    double weight = 1 / pow(kernel_size,2);
    
    Mat filter = copy(img);

    for(int i = n; i < img.rows-n; i++){
        for(int j = n; j < img.cols-n; j++){
            double sum = 0.0;
            for(int k = -n; k < n; k++){
                for(int l = -n; l < n; l++){
                    sum += img.at<uchar>(k + i, l + j) * weight;
                }
            }
            filter.at<uchar>(i,j) = static_cast<uchar>(sum);
        }
    }
    return filter;
}

// cd build
// cmake ..
// make
// ./p3