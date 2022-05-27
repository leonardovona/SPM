#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

class MotionDetector
{
private:
  Mat background_picture;
  int pixels;
  int k;
  VideoCapture vid_capture;

  void init_background_picture()
  {
    vid_capture >> background_picture;

    if (background_picture.empty())
    {
      // THROW EXCEPTION
    }

    background_picture = smooth(greyscale(background_picture));
  }

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
    return (different_pixels * 100 / pixels);
  }

public:
  MotionDetector(string filename, int k)
  {
    vid_capture = VideoCapture(filename);
    if (!vid_capture.isOpened())
    {
      // THROW EXCEPTION
    }
    k = k; // CHECK

    init_background_picture();

    pixels = background_picture.rows * background_picture.cols;
  }

  ~MotionDetector()
  {
    vid_capture.release();
  }

  Mat get_frame()
  {
    Mat frame;
    vid_capture >> frame;
    return frame;
  }

  bool motion_detected(Mat frame)
  {
    return (difference(smooth(greyscale(frame))) >= k);
  }
};