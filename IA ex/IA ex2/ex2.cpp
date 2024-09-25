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

string path = "/Users/giovanniorzalesi/Desktop/IA ex2/PEN.png";
void imageinfo(Mat img);
Mat grey_image(Mat img);
Mat histogram(Mat img);
Mat histogram_gray(Mat img);
Mat copy(Mat img);
Mat binary(Mat bin, unsigned char threshold);
vector<double> c_mass(Mat img);
vector<double> moments(Mat image, vector<double> v);
double theta(vector<double> mu);

int main(){
    Mat img, img_bin;
    unsigned char threshold = 50;

    img = imread(path, IMREAD_GRAYSCALE);   // Load as gray
    imageinfo(img);
    img_bin = copy(img);
    // Binary image
    img_bin = binary(img_bin, threshold);
    // Centre of mass
    vector<double> v = c_mass(img_bin);
    cout << "X position: " << v[0] << "; Y position: "  << v[1] << "; Total: " << v[2] << endl;
    // Modify the images
    line(img, Point(v[0]-5, v[1]-5), Point(v[0]+5, v[1]+5), Scalar(255,255,255), 2);
    line(img, Point(v[0]-5, v[1]+5), Point(v[0]+5, v[1]-5), Scalar(255,255,255), 2);
    // Moments
    vector<double> mu = moments(img_bin, v);
    cout << "Moments: mu20 " << mu[0] << ", mu02 " << mu[1] << ", mu11 " << mu[2] << endl;
    // Theta
    double angle = theta(mu);
    cout << "Theta: "<< angle << endl;
    double x = v[0] + 200 * cos(angle); double y = v[1] - 200 * sin(angle);
    line(img, Point(v[0], v[1]), Point(static_cast<int>(x), static_cast<int>(y)), Scalar(255, 255, 255), 2);
    // Hu set
    double I1 = (mu[0]+mu[1])/v[3];

    // Display all
    while(1){
        imshow("Image", img);
        imshow("Copy", img_bin);
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

vector<double> c_mass(Mat image){
    vector<double> v;
    int width = image.cols;
    int height = image.rows;
    unsigned char value = 0;
    int sumX = 0; int sumY = 0; int whitecount = 0;

    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            value = image.at<uchar>(i,j);
            if(value != 255){
                sumX += j;
                sumY += i;
                whitecount++;
            }
        } 
    }
    // Calculate the centre of mass
    double centreX = static_cast<double> (sumX)/whitecount;
    double centreY = static_cast<double> (sumY)/whitecount;
    v.push_back(centreX); v.push_back(centreY); v.push_back(whitecount);
    return v;
}

vector<double> moments(Mat image, vector<double> v){
    vector<double> mu;
    double mu20 = 0.0; double mu02 = 0.0; double mu11 = 0.0;
    double diff_x, diff_y;
    int width = image.cols;
    int height = image.rows;
    unsigned int value = 0;

    for(int i=0; i<height;i++){
        for(int j=0; j<width; j++){
            diff_x = j-v[0];
            diff_y = i-v[1];

            mu20 += pow(diff_x,2);
            mu02 += pow(diff_y, 2);
            mu11 += diff_x * diff_y;
        }
    }
    mu.push_back(mu20); mu.push_back(mu02); mu.push_back(mu11);
    return mu;
}

double theta(vector<double> mu){
    double t = 0.5 * atan2(2 * mu[2], mu[0] - mu[1]);
    double deg = t * 180.0 / CV_PI;
    cout << "Deg: " << deg << endl;
    return t;
}

// cd build
// cmake ..
// make
// ./p2