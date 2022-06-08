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
    utimer u("Sequential motion detection");

    MotionDetector motion_detector(filename, k);

    while (1)
    {
      frame = motion_detector.get_frame();
      // If the frame is empty, break immediately
      if (frame.empty())
        break;

      if (motion_detector.motion_detected(frame))
        number_of_frames_with_motion++;
    }
  }
  catch (Exception e)
  {
    cerr << e.what() << endl;
    return -1;
  }

  cout << number_of_frames_with_motion << endl;

  return 0;
}