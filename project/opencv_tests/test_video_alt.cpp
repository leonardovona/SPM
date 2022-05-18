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