#include "opencv2/opencv.hpp"
#include <iostream>
#include "utimer.cpp"

#include "MotionDetector.cpp"

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    cout << "Usage: sequential_av video k" << endl;
    return -1;
  }

  string filename = argv[1];
  int k = atoi(argv[2]);

  int number_of_frames_with_motion = 0;
  Mat frame;

  try
  {
    MotionDetector motion_detector(filename, k);

    while (1)
    {
      frame = motion_detector.get_frame();
      // If the frame is empty, break immediately
      if (frame.empty())
        break;
      auto start = std::chrono::high_resolution_clock::now();
      if (motion_detector.motion_detected(frame))
        number_of_frames_with_motion++;
      auto elapsed = std::chrono::high_resolution_clock::now() - start;
      cout << chrono::duration_cast<chrono::microseconds>(elapsed).count() << endl;
    }
  }
  catch (Exception e)
  {
    cerr << e.what() << endl;
    return -1;
  }
  return 0;
}