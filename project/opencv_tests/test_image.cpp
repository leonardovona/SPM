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
  for (int i = 1; i < frame.rows - 1; i++)
  {
    for (int j = 1; j < frame.cols - 1; j++)
    {
      u_int16_t value = 0;
      // for (int ii = max(0, i - 1); ii <= min(i + 1, frame.rows - 1); ii++)
      for (int ii = i - 1; ii <= i + 1; ii++)
      {
        // for (int jj = max(0, j - 1); jj <= min(j + 1, frame.cols - 1); jj++)
        for (int jj = j - 1; jj <= j + 1; jj++)
        {
          value += frame.at<u_int8_t>(ii, jj);
        }
      }
      smoothed.at<u_int8_t>(i, j) = (value / 9);
    }
  }

  // cout << "First column" << endl;
  u_int16_t value;
  for (int i = 1; i < frame.rows - 1; i++)
  {
    value = 0;
    for (int ii = i - 1; ii <= i + 1; ii++)
    {
      for (int jj = 0; jj <= 1; jj++)
      {
        value += frame.at<u_int8_t>(ii, jj);
      }
    }
    smoothed.at<u_int8_t>(i, 0) = (value / 6);
  }
  
    // cout << "Last column" << endl;
    for (int i = 1; i < frame.rows - 1; i++)
    {
      value = 0;
      for (int ii = i - 1; ii <= i + 1; ii++)
      {
        for (int jj = frame.cols - 2; jj <= frame.cols - 1; jj++)
        {
          value += frame.at<u_int8_t>(ii, jj);
        }
      }
      smoothed.at<u_int8_t>(i, frame.cols - 1) = (value / 6);
    }

    // cout << "First row" << endl;
    for (int j = 1; j < frame.cols - 1; j++)
    {
      value = 0;
      for (int jj = j - 1; jj <= j + 1; jj++)
      {
        for (int ii = 0; ii <= 1; ii++)
        {
          value += frame.at<u_int8_t>(ii, jj);
        }
      }
      smoothed.at<u_int8_t>(0, j) = (value / 6);
    }

    // cout << "Last row" << endl;
    // last row
    
    for (int j = 1; j < frame.cols - 1; j++)
    {
      value = 0;
      for (int jj = j - 1; jj <= j + 1; jj++)
      {
        for (int ii = frame.rows - 2; ii <= frame.rows - 1; ii++)
        {
          value += frame.at<u_int8_t>(ii, jj);
        }
      }
      smoothed.at<u_int8_t>(frame.rows - 1, j) = (value / 6);
    }

    // cout << "Top left corner" << endl;
    // top left corner
    value = 0;
    for (int i = 0; i <= 1; i++)
    {
      for (int j = 0; j <= 1; j++)
      {
        value += frame.at<u_int8_t>(i, j);
      }
    }
    smoothed.at<u_int8_t>(0, 0) = (value / 4);

    // cout << "Top right corner" << endl;
    // top right corner
    value = 0;
    for (int i = 0; i <= 1; i++)
    {
      for (int j = frame.cols - 2; j <= frame.cols - 1; j++)
      {
        value += frame.at<u_int8_t>(i, j);
      }
    }
    smoothed.at<u_int8_t>(0, frame.cols - 1) = (value / 4);

    // cout << "Bottom left corner" << endl;
    // bottom left corner
    value = 0;
    for (int i = frame.rows - 2; i <= frame.rows - 1; i++)
    {
      for (int j = 0; j <= 1; j++)
      {
        value += frame.at<u_int8_t>(i, j);
      }
    }
    smoothed.at<u_int8_t>(frame.rows - 1, 0) = (value / 4);

    // cout << "Bottom right corner" << endl;
    // bottom right corner
    value = 0;
    for (int i = frame.rows - 2; i <= frame.rows - 1; i++)
    {
      for (int j = frame.cols - 2; j <= frame.cols - 1; j++)
      {
        value += frame.at<u_int8_t>(i, j);
      }
    }
    smoothed.at<u_int8_t>(frame.rows - 1, frame.rows - 1) = (value / 4);
  
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