#include "opencv2/opencv.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "utimer.cpp"
#include <algorithm>
#include <queue>
#include <vector>
#include <atomic>
#include "sharedQueue.cpp"

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
  return (different_pixels * 100 / pixels);
}

bool motion_detected(Mat frame)
{
  return (difference(smooth(greyscale(frame))) >= k);
}

VideoCapture vid_capture;

int nw;
//vector<sharedQueue<Mat>> frames_queues;
vector<sharedQueue<Mat>> frames_queues(4);
void read_frames()
{
  {
    utimer u("read");
    int index = 0;
    int i = 0;
    while (true)
    {
      Mat frame;
      vid_capture >> frame;

      if (frame.empty())
      {
        for (int i = 0; i < nw; i++)
        {
          frames_queues.at(i).push(frame);
        }
        break;
      }
      frames_queues.at(index).push(frame);

      index = (index + 1) % nw;
    }
  }
  return;
}

atomic<int> number_of_frames_with_motion;

void handle_frames(int worker_number)
{
  {
    utimer u("handle");

    int local_counter = 0;
    Mat frame;

    while (true)
    {
      frame = frames_queues.at(worker_number).pop();
      //cout << frames_queue.size() << endl;
      if (frame.empty())
        break;

      if (motion_detected(frame))
        local_counter++;
    }
    number_of_frames_with_motion += local_counter;
  }
  return;
}

int main(int argc, char **argv)
{
  if (argc != 4)
  {
    cout << "Usage: test_video video k nw" << endl;
    return -1;
  }

  string filename = argv[1];
  k = atoi(argv[2]);
  nw = atoi(argv[3]);

  // Create a VideoCapture object and open the input file
  // If the input is the web camera, pass 0 instead of the video file name
  vid_capture = VideoCapture(filename);

  // Check if camera opened successfully
  if (!vid_capture.isOpened())
  {
    cout << "Error opening video stream or file" << endl;
    return -1;
  }

  // init background picture
  vid_capture >> background_picture;
  if (background_picture.empty())
  {
    cout << "ERROR";
    return -1;
  }
  background_picture = smooth(greyscale(background_picture));

  // init global info
  pixels = background_picture.rows * background_picture.cols;

  number_of_frames_with_motion = 0;

  //vector<sharedQueue<Mat>> queues(nw);
  //frames_queues = queues;

  vector<thread> tids;
  for (int i = 0; i < nw; i++)
  {
    tids.push_back(thread(handle_frames, i));
  }

  thread read_thread = thread(read_frames);

  read_thread.join();
  cout << number_of_frames_with_motion << endl;

  for (thread &t : tids)
  { // await thread termination
    t.join();
  }

  cout << number_of_frames_with_motion << endl;
  vid_capture.release();

  return 0;
}