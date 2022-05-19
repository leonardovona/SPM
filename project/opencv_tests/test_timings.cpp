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
  return (different_pixels * 100 / pixels) >= k;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    cout << "Usage: test_timings video" << endl;
    return -1;
  }

  string filename = argv[1];

  // Create a VideoCapture object and open the input file
  // If the input is the web camera, pass 0 instead of the video file name
  VideoCapture vid_capture(filename);

  // Check if camera opened successfully
  if (!vid_capture.isOpened())
  {
    cout << "Error opening video stream or file" << endl;
    return -1;
  }

  // init background picture
  vid_capture >> background_picture;

  Mat frame;

  int i = 0;
  long us;
  long total;
  while (1)
  {
    {
      utimer u("Frame capture", &us);
      vid_capture >> frame;
    }
    if (frame.empty())
    {
      break;
    }
    total += us;
    i++;
  }
  cout << "Average frame capture time: " << (total / i) << endl;

  /*
  if (background_picture.empty())
  {
    cout << "ERROR";
    return -1;
  }
  */
  /*
    {
       utimer u("Greyscaling time");
       greyscale(background_picture);
     }
  */

  // Mat greyscaled = greyscale(background_picture);
  /*
  {
    utimer u("Smooth time");
    smooth(greyscaled);
  }
  */

  /*
  background_picture = smooth(greyscaled);
  Mat smoothed = smooth(greyscale(frame));
  k = 10;
  pixels = background_picture.rows * background_picture.cols;
  {
    utimer u("Detect motion");
    difference(smoothed);
  }
  */
  vid_capture.release();

  return 0;
}