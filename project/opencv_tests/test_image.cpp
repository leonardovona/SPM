#include "opencv2/opencv.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "utimer.cpp"
#include <algorithm>

using namespace std;
using namespace cv;

// frame is m x n x 3
Mat greyscale(Mat frame)
{
  Mat greyscaled = Mat::zeros(frame.rows, frame.cols, CV_8U);
  for (int i = 0; i < frame.rows; i++)
  {
    for (int j = 0; j < frame.cols; j++)
    {
      u_int16_t value = 0;
      for (int k = 0; k < frame.channels(); k++)
      {
        value += frame.at<Vec3b>(i, j)[k];
      }
      greyscaled.at<u_int8_t>(i, j) = (value / 3);
    }
  }
  return greyscaled;
}

// frame is m x n
Mat smooth(Mat frame)
{
  Mat smoothed = Mat::zeros(frame.rows, frame.cols, CV_8U);
  int counter;
  for (int i = 0; i < frame.rows; i++)
  {
    for (int j = 0; j < frame.cols; j++)
    {
      counter = 0;
      u_int16_t value = 0;
      for (int ii = i - 1; ii <= i + 1; ii++)
      {
        for (int jj = j - 1; jj <= j + 1; jj++)
        {
          if(ii >= 0 && ii < frame.rows && jj >= 0 && jj < frame.cols) {
            value += frame.at<u_int8_t>(ii, jj);
            counter++;
          }
        }
      }
      smoothed.at<u_int8_t>(i, j) = (value / counter);
    }
  }

  return smoothed;
}

int main()
{
  Mat image;
  image = imread("test.jpg", 1);
  if (!image.data)
  {
    printf("No image data \n");
    return -1;
  }
  namedWindow("Display Image", WINDOW_AUTOSIZE);
  imshow("Display Image", smooth(greyscale(image)));
  waitKey(0);

  return 0;
}