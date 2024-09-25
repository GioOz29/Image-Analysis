#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <fstream>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

/*
----------------------------------------------
----------- FEATURE TRANSFORMATION -----------
----------------------------------------------
*/

// GREY IMAGE
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

// RETURN HISTOGRAM GRAY
Mat createHistrogramGray(Mat img){
  unsigned char value = 0; // index value for the histogram (not really needed)
  int histogram[256]; // histogram array - remember to set to zero initially
  int width = img.cols; // say, 320
  int height = img.rows; // say, 240

  int k = 256;
  while (k-- > 0)
    histogram[k] = 0; // reset histogram entry
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      value = img.at<uchar>(i, j);
      histogram[value] += 1;
    }
  }

  Mat histo(200, 256, CV_8UC3, Scalar(0, 0, 0));
  Point p1, p2;
  k = 256;
  while (k-- > 0) {
    p1.x = k;
    p1.y = histo.rows - 1;
    p2.x = k;
    p2.y = histo.rows - 1 - histogram[k] / 1000;
    line(histo, p1, p2, Scalar(255, 255, 255), 2, LINE_4);
  }
  return histo;
}

// RETURN BINARY IMAGE
Mat binary(Mat img, unsigned char threshold) {
    Mat bin = img.clone();
    unsigned char value = 0;
    int width = bin.cols;
    int height = bin.rows;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            value = bin.at<uchar>(i, j);
            if (value > threshold) {
                bin.at<uchar>(i, j) = 255;
            }
            else {
                bin.at<uchar>(i, j) = 0;
            }
        }
    }
    return bin;
}

/*
----------------------------------------------
----------- ELLIPSE EXTRACTION     -----------
----------------------------------------------
*/

// CALCULATE KERNEL
int calculateAdaptiveBlurKernelSize(const Mat& image) {
    double scaleFactor = 0.75;
    Mat gradX, gradY;
    Sobel(image, gradX, CV_32F, 1, 0);
    Sobel(image, gradY, CV_32F, 0, 1);

    Mat gradientMag;
    magnitude(gradX, gradY, gradientMag);

    Scalar mean, stddev;
    meanStdDev(gradientMag, mean, stddev);

    // Adjust the kernel size based on the standard deviation
    int blurKernelSize = static_cast<int>(stddev.val[0] * scaleFactor);

    // Ensure the kernel size is an odd number
    blurKernelSize |= 1;

    return blurKernelSize;
}

// BLUR LOW THRESHOLD (ADAPTIVE)
int calculateAdaptiveLowThreshold(const Mat& image) {
    Mat blurred;
    medianBlur(image, blurred, 7); // Use an appropriate kernel size, for example, 5

    // Calculate the median pixel value of the blurred image
    Scalar medianValue = mean(blurred);

    // Set the low threshold as a fraction (e.g., 0.5) of the median value
    int lowThreshold = static_cast<int>(medianValue[0] * 11);

    return lowThreshold;
}

// BLUR HIGH THRESHOLD (ADAPTIVE)
int calculateAdaptiveHighThreshold(const Mat& image) {
    double highToLowRatio = 1.4;
    // Calculate the high threshold as a multiple of the low threshold
    int lowThreshold = calculateAdaptiveLowThreshold(image);
    int highThreshold = static_cast<int>(lowThreshold * highToLowRatio);

    return highThreshold;
}

// Function to calculate eigenvalues from trace and determinant
vector<double> calculateEigenvalues(double discriminant, double trace) {
    vector<double> eigenvalues;

    // Calculate the eigenvalues
    double lambda1 = (trace + sqrt(discriminant)) / 2.0;
    double lambda2 = (trace - sqrt(discriminant)) / 2.0;

    // Store the eigenvalues in the vector
    eigenvalues.push_back(lambda1);
    eigenvalues.push_back(lambda2);

    return eigenvalues;
}

RotatedRect fitEllipse2(const vector<Point>& points) {
    //number of points to fit
    int numPoints = points.size();

    // Ensure we have enough points to fit an ellipse
    if (numPoints < 5) {
        throw invalid_argument("Insufficient points for ellipse fitting");
    }

    // Formulate the least squares problem Ax = b for the ellipse parameters
    Mat A(numPoints, 5, CV_64F);
    Mat bTerm(numPoints, 1, CV_64F);

    for (int i = 0; i < numPoints; ++i) {
        double x = points[i].x;
        double y = points[i].y;

        A.at<double>(i, 0) = x * x;
        A.at<double>(i, 1) = x * y;
        A.at<double>(i, 2) = y * y;
        A.at<double>(i, 3) = x;
        A.at<double>(i, 4) = y;
        A.at<double>(i, 5) = 1;

        bTerm.at<double>(i) = -1;  // Assuming the equation of the ellipse is Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0
    }

    // Solve the least squares problem to obtain ellipse parameters -> used SVD decomposition 
    Mat x;
    solve(A, bTerm, x, DECOMP_SVD);

    // Extract ellipse parameters
    double A_val = x.at<double>(0);
    double B_val = x.at<double>(1);
    double C_val = x.at<double>(2);
    double D_val = x.at<double>(3);
    double E_val = x.at<double>(4);
    double F_val = x.at<double>(5);

    // Calculate trace and determinant of A_val
    double trace_A = A_val + C_val;    //+F???   Tr(matrix)=A+C+F
    // double det_A = A_val * C_val - B_val * B_val; TEST 1
   // Det(matrix)= ACF + 2BDE - AE^2 - BD^2 - CF^2
    double det_A = A_val * C_val * F_val + 2 * B_val * D_val * E_val - A_val * pow(E_val, 2) - B_val * pow(D_val, 2) - C_val * pow(F_val, 2);
    
    // Calculate the discriminant
    double discriminant = trace_A * trace_A - 4 * det_A;

    RotatedRect ellipse(Point2f(0, 0), Size2f(0, 0), 0);
;
    if(discriminant >= 0){
        // Calculate eigenvalues
        vector<double> lambdaval = calculateEigenvalues(discriminant, trace_A);

        // Calculate semimajor axis and semiminor axis from eigenvalues
        double a = sqrt(1.0 / lambdaval[0]);
        double b = sqrt(1.0 / lambdaval[1]);

        // Calculate other ellipse parameters
        double cx = (2 * C_val * D_val - B_val * E_val) / (B_val * B_val - 4 * A_val * C_val);
        double cy = (2 * A_val * E_val - B_val * D_val) / (B_val * B_val - 4 * A_val * C_val);
        
        double theta = 0.5 * atan2(B_val, A_val - C_val);
        
        //Find semi major axis of the ellipse
        double major_axis = max(a, b);
        
        // Convert ellipse parameters to OpenCV's RotatedRect format
        
        ellipse.center = Point2f(static_cast<float>(cx), static_cast<float>(cy));
        ellipse.size = Size2f(static_cast<float>(a * 2), static_cast<float>(b * 2)); // Use 'ellipse_b'
        ellipse.angle = static_cast<float>(theta * 180.0 / CV_PI);
        
        // Save the major axis of the ellipse
        cout << "Major axis (2*a) = " << ellipse.size.width << "\n" << endl;
        cout << "Angle of rotation = " << ellipse.angle << "\n" << endl;

        // Draw the ellipses on top of the previous image
        
        return ellipse;
    } else return ellipse;

    
}

// RECOGNISE ELLIPSES
vector<RotatedRect> filterEllipses(const vector<vector<Point>>& contours){
    vector<RotatedRect> ellipses;

    for (const auto& contour : contours) {
        // Keep track of the maximum aspect ratio
        double maxAspectRatio = 0.0;
        // Check if the contour has enough points to fit an ellipse
        if (contour.size() >= 100) {
            // Fit an ellipse to the contour
            RotatedRect ellipse = fitEllipse(contour);
            
            // if(ellipse.center == Point2f(0, 0) && ellipse.size == Size2f(0, 0) && ellipse.angle == 0){
            //     break;
            // }

            // Filter based on certain criteria
            // For example, you can filter by aspect ratio, size, or any other relevant property
            double aspectRatio = ellipse.size.width / ellipse.size.height;

            if (aspectRatio > 0.65 && aspectRatio < 0.9) {
                // Add the ellipse to the list if it meets the criteria
                ellipses.push_back(ellipse);
                // If the current aspect ratio is greater than the maximum found so far
                if (aspectRatio > maxAspectRatio) {
                    // Update the maximum aspect ratio
                    maxAspectRatio = aspectRatio;

                    // Clear the existing ellipses and add the current one
                    ellipses.clear();
                    ellipses.push_back(ellipse);
                }
            }
        }
    }

    return ellipses;
}

// DROWING ELLIPSE
void drawEllipses(Mat& image, const vector<RotatedRect>& ellipses){
    for(const auto& ellipse : ellipses){
        // Draw the ellipse on the image in blue
        cv::ellipse(image, ellipse, Scalar(255, 0, 0), 15);
    }
}

/*
----------------------------------------------
----------- SQUARE EXTRACTION      -----------
----------------------------------------------
*/

// RECOGNISE SQUARES
Point detectWhiteSquares(const vector<vector<Point>>& contours, Mat image) {
    // Define a minimum area threshold for contours to filter out noise
    // Calculate the median contour area
    vector<double> contourAreas;
    for (const auto& contour : contours) {
        double area = contourArea(contour);
        contourAreas.push_back(area);
    }

    // Calculate the median contour area
    double medianContourArea = 0.0;
    if (!contourAreas.empty()) {
        size_t size = contourAreas.size();
        nth_element(contourAreas.begin(), contourAreas.begin() + size / 2, contourAreas.end());
        medianContourArea = contourAreas[size / 2];
    }

    // Use the median contour area to set adaptive thresholds
    const double maxContourArea = medianContourArea * 350;
    const double minContourArea = medianContourArea * 250;
    Point center;
    center = Point(-1, -1);

    for (const auto& contour : contours) {
        // Calculate the area of the contour
        double area = contourArea(contour);

        // Check if the contour is larger than the threshold
        if (area > minContourArea && area < maxContourArea) {
            // Approximate the contour to a polygon
            vector<Point> approx;
            approxPolyDP(contour, approx, arcLength(contour, true) * 0.03, true);

            // Check if the polygon has 4 corners (a square)
            if (approx.size() == 4) {
                // Calculate the center of the square
                Moments mu = moments(contour, false);
                center = Point(mu.m10 / mu.m00, mu.m01 / mu.m00);
            }
        }
    }
    return center;
}

/*
----------------------------------------------
----------- X,Y,Z CALCULATION      -----------
----------------------------------------------
*/

// RETURN THE PARAMETER DISTANCE OF THE CAMERA
double calculateDistance(double imgDiameter) {
    // Define the parameters
    double realDiameter = 6.9;  // cm
    double focal_length = 2.5 * 1.6; // cm  *APS parameter
    float pix_val = 3.71 * pow(10,-4);  // cm/pixel

    // Distance calculation
    double distance = focal_length * realDiameter / (imgDiameter * pix_val);
    
    return distance;
}

// RETURN THE TRANSLATION OF THE CAMERA
vector<double> translationCamera(double cxDock, double cyDock){
    // Define the parameters
    double Hfov = 1.78;
    double Vfov = 1.19;
    double xpixels = 6000;
    double ypixels = 4000;
    double focal_length = 2.5 * 1.6; // cm *APS parameter

    // Define the final vector
    vector<double> translation;

    // Translation from the centre image in pixels
    double cx = cxDock - xpixels/2;
    double cy = cyDock - ypixels/2;

    cx = cx * Hfov / (2*focal_length);
    cy = cy * Vfov / (2*focal_length);

    translation.push_back(cx);
    translation.push_back(cy);

    return translation;
}

// FEATURE MATCHING (ALREADY TESTED, IT WORKS, BUT I AM NOT SURE WE WILL NEED SOMETHING LIKE THIS)
void matching(string path1, string path2) {
   // Load the image
   Mat img1 = imread(path1, IMREAD_GRAYSCALE);   // already loaded a gray

   //detect the keypoints from image
   auto detector = SiftFeatureDetector::create();
   vector<KeyPoint> keypoints;
   detector->detect(img1, keypoints);

   //present the keypoints on image
   Mat image_with_keypoints;
   drawKeypoints(img1, keypoints, image_with_keypoints);

   //extract the sift descriptors from image
   auto extractor = SiftDescriptorExtractor::create();
   Mat descriptors;
   extractor->compute(img1, keypoints, descriptors); // san input image eixa valei to "image" anti gia "grayscaleImage". De vlepo diafora sto output

   //for visualization
   namedWindow("image with keypoints", WINDOW_AUTOSIZE);
   imshow("image with keypoints", image_with_keypoints);

   // Load the image
   Mat img2 = imread(path2, IMREAD_GRAYSCALE);

   //detect the keypoints from image
   //auto detector = SiftFeatureDetector::create();
   vector<KeyPoint> keypointsII;
   detector->detect(img2, keypointsII);

   //present the keypoints on image
   Mat image_with_keypointsII;
   drawKeypoints(img2, keypointsII, image_with_keypointsII);

   //extract the sift descriptors from image
   //auto extractor = SiftDescriptorExtractor::create();
   Mat descriptorsII;
   extractor->compute(img2, keypointsII, descriptorsII); // san input image eixa valei to "image" anti gia "grayscaleImage". De vlepo diafora sto output

   //for visualization
   namedWindow("image with keypointsII", WINDOW_AUTOSIZE);
   imshow("image with keypointsII", image_with_keypointsII);

   //Matching descriptor vectors with a FLANN based matcher
   Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
   vector< vector<DMatch> > knn_matches;
   matcher->knnMatch(descriptors, descriptorsII, knn_matches, 2);

   //Filter matches using the Lowe's ratio test
   const float ratio_thresh = 0.7f;
   std::vector<DMatch> good_matches;
   for (size_t i = 0; i < knn_matches.size(); i++)
   {
       if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
       {
           good_matches.push_back(knn_matches[i][0]);
       }
   }

   //Draw matches
   Mat img_matches;
   drawMatches(img1, keypoints, img2, keypointsII, good_matches, img_matches, Scalar::all(-1),
       Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

   //Show detected matches
   imshow("Good Matches", img_matches);

   waitKey(0);
}
