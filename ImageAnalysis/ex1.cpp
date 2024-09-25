#include "ex1.h"

void helloWorld() {
  cout << "Hello World \n";
}


Mat createGreyScaleImage(Mat img) {
  Mat grey;
  cvtColor(img, grey, COLOR_BGR2GRAY);
  return grey;
}


Mat createHistrogramColor(Mat img, unsigned char channel) {
  unsigned char value = 0; // index value for the histogram (not really needed)
  int histogram[256]; // histogram array - remember to set to zero initially
  int width = img.cols; // say, 320
  int height = img.rows; // say, 240

  int k = 256;
  while (k-- > 0)
    histogram[k] = 0; // reset histogram entry

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      value = img.at<Vec3b>(i, j)[channel];
      histogram[value] += 1;
    }
  }


  Mat histo(200, 256, CV_8UC3, Scalar(0, 0, 0));
  Point p1, p2;
  k = 256;
  while (k-- > 0) {
    p1.x = k;
    p1.y = histo.rows-1;
    p2.x = k;
    p2.y = histo.rows - 1-histogram[k]/1000;
    line(histo, p1, p2,Scalar(0,255,0),2,LINE_4);
  }
  return histo;
}

Mat createHistrogramGray(Mat img) {
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
    line(histo, p1, p2, Scalar(0, 255, 0), 2, LINE_4);
  }
  return histo;
}

#define MAX_RAND 999
void contourSearch(Mat pic, Point pos, int rimx[], int rimy[], int local_tresh) {
  
  int count = 0;
  Point newpos;
  int randx[MAX_RAND], randy[MAX_RAND];
  int draw_type = 0;
  newpos = pos;
  while (newpos.x >= 0 && newpos.y>=0 && newpos.x < pic.cols && newpos.y< pic.rows)
  {
    // save current position in list
    rimx[count] = newpos.x; 
    rimy[count] = newpos.y;
    count++;
    // Select next search direction
    draw_type = (draw_type + 6) % 8; 
    switch (draw_type)
    {
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
        break; }
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


void part1(string file) {
  Mat img = imread(file);
  cout << "filename:" << file;
  cout << "\nrows and columns: " << img.rows << ", " << img.cols;
  cout << "\nChannels: " << img.channels() << "\n";
  cout << "Type: " << img.channels() << "\n";
  int B = img.at<Vec3b>(57, 0)[0];
  int G = img.at<Vec3b>(57, 0)[1];
  int R = img.at<Vec3b>(57, 0)[2];
  cout << "BGR: Pixel(57,0): " << B << ", " << G << ", " << R << "\n";
}

void part2(string file) {
  Mat img = imread(file);
  namedWindow("output");
  resizeWindow("output", 100, 100);
  waitKey(0);
  moveWindow("output", 100, 100);
  waitKey(0);
  imshow("output", img);
  waitKey(0);
  
  destroyAllWindows();

  Mat histo;
  histo = createHistrogramColor(img, 0);
  namedWindow("ShowHistogram");
  imshow("ShowHistogram", histo);
  waitKey(0);

}



void part3_4_5() {

  Mat img, grey;
  VideoCapture cap(0);	//Try changing to 1 if not working 
  //Check if camera is available
  if (!cap.isOpened())
  {
    cout << "Could not initialize capturing...\n";
  }

  namedWindow("ShowImage");

  char c;
  int window_x[4] = {0, 100, 0, 100}, window_y[4] = { 0, 0, 100, 100 }, img_res_x[4] = { 100, 200, 400, 800 }, img_res_y[4] = { 100, 200, 400, 800 }, brightnes[4] = {0, 25, 50, 100};
  unsigned char count_move = 0, count_reso = 0, count_brig = 0;
  signed char count_shut = -13;

  cap >> img;
  imshow("ShowImage", img);

  //Show Camera output
  while (1) {
    if (count_move == 4) {
      count_move = 0;
    }
    if (count_brig == 4) {
      count_brig = 0;
    }
    if (count_reso == 4) {
      count_reso = 0;
    }
    if (count_shut == 0) {
      count_shut = -13;
    }
       
    c = waitKey(0);

    if (c == 99) {
      cap >> img;
      imshow("ShowImage", img);
    }
    //Use m to move window around according to variables window_x and window_y
    if (c == 109) {
      moveWindow("ShowImage", window_x[count_move], window_y[count_move]);
      count_move++;
    }
    //Use g to capture grey image and create histogram
    else if (c == 103) {
      grey = createGreyScaleImage(img);
      imshow("ShowImage", grey);
      Mat histo = createHistrogramGray(grey);
      namedWindow("ShowHistogram");
      imshow("ShowHistogram", histo);
    }
    //use s to change shutter time
    else if (c == 115) {
      cap.set(CAP_PROP_EXPOSURE, count_shut);
      cap >> img;
      imshow("ShowImage", img);
      count_shut++;
    }
    //use b to change shutter time
    else if (c == 98) {
      cap.set(CAP_PROP_BRIGHTNESS, brightnes[count_brig]);
      cout << brightnes[count_brig] << "\n";
      cap >> img;
      imshow("ShowImage", img);
      count_brig++;
    }
    //use w to save image
    else if (c == 119) {
      imwrite("test.png", img);
    }
    //use r to change resolution according to img_res_x img_res_y
    else if (c == 114) {
      cap.set(CAP_PROP_FRAME_WIDTH, img_res_x[count_reso]);
      cap.set(CAP_PROP_FRAME_HEIGHT, img_res_y[count_reso]);
      cap >> img;
      count_reso++;
    }
    else if (c == 27) {
      break; 	//Press escape to stop program - 27 means escape in ascii
    }
    else {
      imshow("ShowImage", img);
    }
  }
}