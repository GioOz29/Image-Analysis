#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>

// Gaussian Kernel matrix
Mat generateGaussianKernel() {
    int size = 5;
    double sigma = 0.3;
    int n = (size-1)/2;
    // Create a kernel matrix of the specified size and type
    Mat kernel(size, size, CV_64F);
    double normal = 1.0 / (2.0 * M_PI * pow(sigma,2));
    
    for (int x = -n; x <= n; ++x) {
        for (int y = -n; y <= n; ++y) {
            double exponent = -(pow(x,2) + pow(y,2)) / (2.0 * pow(sigma,2));
            kernel.at<double>(x + n, y + n) = normal * exp(exponent);
        }
    }

    // Normalize the kernel to ensure the sum is 1.0
    kernel /= sum(kernel)[0];

    return kernel;
}

// filter2D(image, output, kernel)
Mat filter_2D(Mat img, Mat kernel){
    // Generating the output blurred
    Mat blur = img.clone();
    int kernelx = kernel.cols/2;
    int kernely = kernel.rows/2;

    for (int i = kernely; i < img.rows - kernely; ++i) {
        for (int j = kernelx; j < img.cols - kernelx; ++j) {
            double sum = 0.0;
            for (int k = -kernely; k <= kernely; ++k) {
                for (int l = -kernelx; l <= kernelx; ++l) {
                    sum += kernel.at<double>(k + kernely, l + kernelx) * img.at<uchar>(i + k, j + l);
                }
            }
            blur.at<uchar>(i, j) = sum;
        }
    }
    return blur;
}

// Gradient Calculation
Mat gradx(Mat img){
    // Sobel kernel
    Mat kernelx = (Mat_<double>(3,3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);
    // Convolution with the image
    Mat gradx = filter_2D(img, kernelx);
    return gradx;
}

Mat grady(Mat img){
    // Sobel kernel
    Mat kernely = (Mat_<double>(3,3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);
    // Convolution with the image
    Mat grady = filter_2D(img, kernely);
    return grady;
}

// Magnitude
Mat magnitude(Mat gradx, Mat grady){
    Mat mag(gradx.size(), CV_8U);
    int width = gradx.cols;
    int height = gradx.rows;

    for(int i = 0; i<width;i++){
        for(int j = 0;j<height;j++){ 
            mag.at<double>(i,j) = sqrt(pow(gradx.at<double>(i,j),2) + pow(grady.at<double>(i,j),2));
        }
    }
    return mag;
}

// Direction
Mat direction(Mat gradx, Mat grady){
    Mat dir(gradx.size(), CV_8U);
    int width = gradx.cols;
    int height = gradx.rows;

    for(int i = 0; i<width;i++){
        for(int j = 0;j<height;j++){ 
            dir.at<double>(i,j) = atan2(grady.at<double>(i,j),gradx.at<double>(i,j));
        }
    }
    return dir;
}


// RIDGE DETECTION
Mat customGaborKernel(int size){
    // Parameter for the Gabor filter 
    double sigma = 1.0;         // Standard deviation of the gaussian envelope
    double theta = CV_PI / 4.0; // Orientation of the filter (45 degrees in this case)
    double lambda = 10.0;       // Wavelength of the sinusoidal factor
    double gamma = 0.5;         // Spatial aspect ratio

    Mat kernel(size, size, CV_64F);
    int n = (size-1)/2;

    double sigmaX = sigma;
    double sigmaY = sigma / gamma;

    for (int x = -n; x <= n; ++x) {
        for (int y = -n; y <= n; ++y) {
            // Rotation
            double xr = x * cos(theta) + y * sin(theta);
            double yr = -x * sin(theta) + y * cos(theta);

            // Gabor function formula
            double value = exp(-(pow(xr,2) + pow(gamma,2) * pow(yr,2)) / (2 * sigmaX * sigmaY)) * cos(2 * CV_PI * xr / lambda);
            kernel.at<double>(x + n, y + n) = value;
        }
    }

    return kernel;

}

// CIRCLE AND ELLIPSE DETECTION
Mat highlightArcs(Mat bin) {
    // Generating an image where I can highlight the circles
    Mat arc;
    cvtColor(bin, arc, COLOR_GRAY2BGR);

    // Find contours
    vector<vector<Point>> contours;
    findContours(bin, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Fit ellipses to each contour
    for (const auto& contour : contours) {
        // Fit ellipse only if the contour has enough points
        if (contour.size() >= 5) {
            RotatedRect ellipse = fitEllipse(contour);
            // Highlight the detected ellipse
            cv::ellipse(arc, ellipse, Scalar(255), 2);
        }
    }

    return arc;
}