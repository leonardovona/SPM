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

string filename;

void test_greyscale()
{
  VideoCapture vid_capture(filename);

  // Check if camera opened successfully
  if (!vid_capture.isOpened())
  {
    cout << "Error opening video stream or file" << endl;
    return;
  }

  Mat frame;
  vid_capture >> frame;

  if (frame.empty())
  {
    return;
  }

  {
    utimer u("[OUT] Greyscaling time");
    greyscale(frame);
  }

  vid_capture.release();

  return;
}

void test_smooth()
{
  VideoCapture vid_capture(filename);

  // Check if camera opened successfully
  if (!vid_capture.isOpened())
  {
    cout << "Error opening video stream or file" << endl;
    return;
  }

  Mat frame;
  vid_capture >> frame;

  if (frame.empty())
  {
    return;
  }

  Mat greyscaled = greyscale(frame);

  {
    utimer u("[OUT] Smooth time");
    smooth(greyscaled);
  }

  vid_capture.release();

  return;
}

void test_difference()
{
  VideoCapture vid_capture(filename);

  // Check if camera opened successfully
  if (!vid_capture.isOpened())
  {
    cout << "Error opening video stream or file" << endl;
    return;
  }

  Mat frame;
  vid_capture >> background_picture;

  if (background_picture.empty())
  {
    return;
  }

  vid_capture >> frame;

  if (frame.empty())
  {
    return;
  }

  background_picture = smooth(greyscale(background_picture));
  Mat smoothed = smooth(greyscale(frame));
  k = 10;
  pixels = background_picture.rows * background_picture.cols;

  {
    utimer u("[OUT] Detect motion");
    difference(smoothed);
  }

  vid_capture.release();

  return;
}

void test_frame_capture_single()
{
  VideoCapture vid_capture(filename);

  // Check if camera opened successfully
  if (!vid_capture.isOpened())
  {
    cout << "Error opening video stream or file" << endl;
    return;
  }

  int i = 0;
  long us;
  long total;
  Mat frame;

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
  cout << "[OUT] Average frame capture single time: " << (total / i) << endl;

  return;
}

void test_frame_capture_complete()
{
  VideoCapture vid_capture(filename);

  // Check if camera opened successfully
  if (!vid_capture.isOpened())
  {
    cout << "Error opening video stream or file" << endl;
    return;
  }

  int frame_count = vid_capture.get(7);

  int i = 0;
  long us;
  long total;
  Mat frame;
  {
    utimer u("Frame capture complete", &us);
    while (1)
    {
      vid_capture >> frame;
      if (frame.empty())
      {
        break;
      }
    }
  }
  cout << "[OUT] Average frame capture complete time: " << (us / frame_count) << endl;

  return;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    cout << "Usage: test_timings video" << endl;
    return -1;
  }

  filename = argv[1];

  test_greyscale();

  test_smooth();

  test_difference();

  test_frame_capture_single();

  test_frame_capture_complete();

  return 0;
}