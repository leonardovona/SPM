#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

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
          if (ii >= 0 && ii < frame.rows && jj >= 0 && jj < frame.cols)
          {
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

Mat background_picture;
int pixels;
int k;

int difference(Mat frame)
{
  Mat difference = background_picture - frame;
  int different_pixels = 0;
  for (int i = 0; i < difference.rows; i++)
  {
    for (int j = 0; j < difference.cols; j++)
    {
      if (difference.at<u_int8_t>(i, j) != 0)
      {
        different_pixels++;
      }
    }
  }
  // scale to percentage
  //cout << (different_pixels * 100 / pixels) << endl;
  return (different_pixels * 100 / pixels);
}

bool motion_detected(Mat frame)
{
  return (difference(smooth(greyscale(frame))) >= k);
}