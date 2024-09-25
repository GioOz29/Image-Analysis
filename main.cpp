#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "functions.hpp"

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

/*
-------------------------------------------------------------------
----------- STEP 1: READ ALL THE IMAGES FROM THE FOLDER -----------
----------- STEP 2: READ THE IMAGE WITH THE GRAY        -----------
----------- STEP 3: FILTER TO OBTAIN THE THE ELLIPSES   -----------
----------- STEP 4: DRAW THE ELLIPSES ON THE IMAGE      -----------
----------- STEP 5: DETECT AND DRAW THE COLOSE SQUARE   -----------
----------- STEP 6: CALCULATE THE DISTANCE              -----------
----------- STEP 7: CALCULATE THE X,Y POSITION          -----------
----------- STEP 8: SAVE THE VALUES IN A CSV FILE       -----------
----------- STEP 9: ON MATLAB DISPLAY THE POSITIONS     -----------
-------------------------------------------------------------------
*/

// IMAGES PATH
string img40 = "/Users/giovanniorzalesi/Desktop/IA proj/set/Set1_00040.jpg";
string folder = "/Users/giovanniorzalesi/Desktop/IA proj/set";
string pos = "/Users/giovanniorzalesi/Desktop/IA proj/pos.csv";

int main() {
    // Save the parameters into a file .csv
    ofstream outputFile(pos);
    // IMAGE FOLDER
    vector<String> filenames;
    glob(folder, filenames, false);
    ©
    // LOOP THROUGHT THE IMAGES
    for(auto const &f : filenames){
        cout << string(f) << endl;
        // Load the images

        Mat img = imread(f);
        Mat img_grey = imread(f, IMREAD_GRAYSCALE);
        
        // Adaptive Gaussian Blur
        int kernel = calculateAdaptiveBlurKernelSize(img);
        Mat blur;
        GaussianBlur(img_grey, blur, Size(kernel, kernel), 0, 0);
            
        // Adaptive Canny Edge detection
        int lowThreshold = calculateAdaptiveLowThreshold(blur);
        int highThreshold = calculateAdaptiveHighThreshold(blur);
        Mat canny;
        Canny(blur, canny, lowThreshold, highThreshold);

        // Find contours
        vector<vector<Point>> contours;
        findContours(canny, contours, RETR_TREE, CHAIN_APPROX_TC89_KCOS);
        
        // Ellipse fitting
        vector<RotatedRect> ellipses = filterEllipses(contours);
        drawEllipses(img, ellipses);

        // Calculate the diameter of the ellipse to estimate the distance
        // Extract major axes
        float majorAxis = max(ellipses[0].size.width, ellipses[0].size.height);

        // Distance extraction
        double distance = calculateDistance(majorAxis);
        
        // Translation extraction
        Point2f center = ellipses[0].center;
        vector<double> translation = translationCamera(center.x, center.y);

        // DECTECT THE CENTRE OF THE RECTANGLE
        Mat squares;
        threshold(img_grey, squares, 200, 255, THRESH_BINARY);
        vector<vector<Point>> lines;
        findContours(squares, lines, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        Point center_rectangle = detectWhiteSquares(lines, img);
        circle(img, center_rectangle, 5, Scalar(0, 0, 255), 20);

        // DISPLAY
        imshow("img", img);
        waitKey(0);

        if(outputFile.is_open()){
            cout << "input" << endl;
            // Write x,y,z,rotation in case

            outputFile << translation[0] << "," << -translation[1] << "," << distance << "," << 6.9 << endl;

            
        } else cerr << "couldn't";
    }
    // Close the file
    outputFile.close();
    return 0;
}

// touch CMakeLists.txt
// cmake_minimum_required(VERSION 3.10)
// project(p)
// find_package(OpenCV REQUIRED)
// add_executable(p main.cpp)
// target_link_libraries(p ${OpenCV_LIBS})

// mkdir build
// cd build
// cmake ..
// make
// ./p
