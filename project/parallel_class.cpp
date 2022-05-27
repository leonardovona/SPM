#include "opencv2/opencv.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "utimer.cpp"
#include <algorithm>
#include <queue>
#include <vector>
#include <atomic>

#include "motion_detection_utils_class.cpp"

using namespace std;
using namespace cv;

vector<queue<Mat>> frames_queues;

void read_frames(MotionDetector motion_detector, int nw)
{
  int index = 0;
  int i = 0;

  while (true)
  {
    Mat frame = motion_detector.get_frame();

    // If the frame is empty, break immediately
    if (frame.empty())
    {
      // send EOSes
      for (int i = 0; i < nw; i++)
      {
        frames_queues.at(i).push(frame);
        cout << "EOS " << i << " " << frames_queues.at(index).size() << endl;
      }
      break;
    }
    frames_queues.at(index).push(frame);

    index = (index + 1) % nw;
  }
  return;
}

atomic<int> number_of_frames_with_motion;

void handle_frames(MotionDetector motion_detector, queue<Mat> *frames_queue)
{
  {
    int local_counter = 0;
    Mat frame;

    while (true)
    {
      if (!frames_queue->empty())
      {
        frame = frames_queue->front();
        frames_queue->pop();

        if (frame.empty())
        {
          break;
        }
        if (motion_detector.motion_detected(frame))
          local_counter++;
      }
      else
      {
        asm("nop;");
      }
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
  int k = atoi(argv[2]);
  int nw = atoi(argv[3]);

  MotionDetector motion_detector(filename, k);

  number_of_frames_with_motion = 0;

  frames_queues.resize(nw);

  {
    utimer u("Parallel");
    vector<thread> tids;

    tids.push_back(thread(read_frames, motion_detector, nw));

    for (int i = 0; i < nw; i++)
    {
      tids.push_back(thread(handle_frames, motion_detector, &frames_queues.at(i)));
    }

    for (thread &t : tids)
    { // await thread termination
      t.join();
    }
  }

  cout << number_of_frames_with_motion << endl;

  return 0;
}