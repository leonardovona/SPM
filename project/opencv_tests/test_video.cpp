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

  // Create a VideoCapture object and open the input file
  // If the input is the web camera, pass 0 instead of the video file name
  VideoCapture vid_capture("test.mp4");

  // Check if camera opened successfully
  if (!vid_capture.isOpened())
  {
    cout << "Error opening video stream or file" << endl;
    return -1;
  }
  else
  {
    // Obtain fps and frame count by get() method and print
    // You can replace 5 with CAP_PROP_FPS as well, they are enumerations
    int fps = vid_capture.get(5);
    cout << "Frames per second: " << fps << endl;

    // Obtain frame_count using opencv built in frame count reading method
    // You can replace 7 with CAP_PROP_FRAME_COUNT as well, they are enumerations
    int frame_count = vid_capture.get(7);
    cout << "Frame count: " << frame_count << endl;
  }
  {
    utimer u("Sequential smooth and greyscale");
    while (1)
    {
      Mat frame;
      // Capture frame-by-frame
      vid_capture >> frame;
      // If the frame is empty, break immediately
      if (frame.empty())
        break;

      Mat greyscaled = greyscale(frame);
      Mat smoothed = smooth(greyscaled);

      if (!smoothed.data)
      {
        printf("No image data \n");
        return -1;
      }

      // imshow("Original", frame);
      // imshow("Greyscaled", greyscaled);
      // imshow("Smoothed", smoothed);

      // Press  ESC on keyboard to exit
      char c = (char)waitKey(25);
      if (c == 27)
        break;
    }

    // cout << frame << endl;
  }

  vid_capture.release();

  // Closes all the frames
  destroyAllWindows();

  return 0;
}