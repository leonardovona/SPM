#include "opencv2/opencv.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "utimer.cpp"
#include <algorithm>
#include <queue>
#include <vector>
#include <atomic>

#include "motion_detection_utils.cpp"

using namespace std;
using namespace cv;

// // frame is m x n x 3
// Mat greyscale(Mat frame)
// {
//   Mat greyscaled = Mat::zeros(frame.rows, frame.cols, CV_8U);
//   for (int i = 0; i < frame.rows; i++)
//   {
//     for (int j = 0; j < frame.cols; j++)
//     {
//       u_int16_t value = 0;
//       for (int k = 0; k < frame.channels(); k++)
//       {
//         value += frame.at<Vec3b>(i, j)[k];
//       }
//       greyscaled.at<u_int8_t>(i, j) = (value / 3);
//     }
//   }
//   return greyscaled;
// }

// // frame is m x n
// Mat smooth(Mat frame)
// {
//   Mat smoothed = Mat::zeros(frame.rows, frame.cols, CV_8U);
//   int counter;
//   for (int i = 0; i < frame.rows; i++)
//   {
//     for (int j = 0; j < frame.cols; j++)
//     {
//       counter = 0;
//       u_int16_t value = 0;
//       for (int ii = i - 1; ii <= i + 1; ii++)
//       {
//         for (int jj = j - 1; jj <= j + 1; jj++)
//         {
//           if (ii >= 0 && ii < frame.rows && jj >= 0 && jj < frame.cols)
//           {
//             value += frame.at<u_int8_t>(ii, jj);
//             counter++;
//           }
//         }
//       }
//       smoothed.at<u_int8_t>(i, j) = (value / counter);
//     }
//   }

//   return smoothed;
// }

// Mat background_picture;
// int pixels;
// int k;

// int difference(Mat frame)
// {
//   Mat difference = background_picture - frame;

//   int different_pixels = 0;
//   for (int i = 0; i < difference.rows; i++)
//   {
//     for (int j = 0; j < difference.cols; j++)
//     {
//       if (difference.at<u_int8_t>(i, j) != 0)
//       {
//         different_pixels++;
//       }
//     }
//   }
//   // scale to percentage
//   return (different_pixels * 100 / pixels);
// }

// bool motion_detected(Mat frame)
// {
//   // cout << difference(smooth(greyscale(frame))) << endl;
//   return (difference(smooth(greyscale(frame))) >= k);
// }

// queue<Mat> frames_queue;
VideoCapture vid_capture;

int nw;
vector<queue<Mat>> frames_queues;

void read_frames()
{

  // vector<queue<Mat>> queues(nw);
  // frames_queues = queues;
  {
    utimer u("read");
    int index = 0;
    int i = 0;
    // while (true && i < 100)
    while (true)
    {
      // i++;
      //  this_thread::sleep_for(chrono::milliseconds(100));
      Mat frame;
      vid_capture >> frame;
      // If the frame is empty, break immediately

      if (frame.empty())
      {
        for (int i = 0; i < nw; i++)
        {
          // Mat empty_frame;
          frames_queues.at(i).push(frame);
          cout << "EOS " << i << " " << frames_queues.at(index).size() << endl;
          // this_thread::sleep_for(chrono::milliseconds(1000));
        }
        break;
      }
      frames_queues.at(index).push(frame);
      // cout << "Push " << frames_queues.at(index).size() << endl;
      // this_thread::sleep_for(chrono::milliseconds(1000));
      // cout << frames_queues.at(index).size() << endl;
      // cout << index << endl;

      index = (index + 1) % nw;
    }
  }
  vid_capture.release();
  return;
}

atomic<int> number_of_frames_with_motion;

void handle_frames(int worker_number)
{
  // int i = 0;
  // int lost_ticks = 0;
  {
    utimer u("handle");

    // cout << "thread " << worker_number << endl;
    // this_thread::sleep_for(chrono::milliseconds(1000));

    queue<Mat> *frames_queue = &frames_queues.at(worker_number);
    int local_counter = 0;
    Mat frame;
    // this_thread::sleep_for(chrono::seconds(5));
    // cout << frames_queues.at(worker_number).size() << " wn " << worker_number << endl;
    while (true)
    {
      // this_thread::sleep_for(chrono::milliseconds(100));
      // cout << frames_queues.at(0).size() << " local size" << endl;
      //  this_thread::sleep_for(chrono::milliseconds(1000));
      if (!frames_queue->empty())
      {
        // i++;
        // cout << i << endl;
        //  cout << frames_queue.size() << endl;
        //  cout << "in " << worker_number << endl;
        //  cout << frames_queue.back().empty() << endl;
        frame = frames_queue->front();
        frames_queue->pop();

        // imshow("Difference", frame);
        // //  Press  ESC on keyboard to exit
        // char c = (char)waitKey(25);
        // if (c == 27)
        //   break;

        if (frame.empty())
        {
          // cout << "EMPTY! " << worker_number;
          break;
        }
        if (motion_detected(frame))
          local_counter++;
        // cout << number_of_frames_with_motion << endl;
      }
      else
      {
        // this_thread::sleep_for(chrono::nanoseconds(1));
        asm("nop;");
        // lost_ticks++;
        //  this_thread::sleep_for(chrono::seconds(1));
        //  cout << "Thread " << frames_queue.size() << endl;
      }
    }
    // cout << "lost ticks: " << lost_ticks << endl;
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
  /*
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
  */

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

  // vector<queue<Mat>> queues(nw);
  // frames_queues = queues;
  frames_queues.resize(nw);
  // frames_queues.reserve(nw);
  //  init queues

  /*read_frames();

  for(int i = 0; i < nw; i++) {
      handle_frames(i);
  }*/

  // thread read_thread = thread(read_frames);
  {
    utimer u("Total time");
    vector<thread> tids;

    tids.push_back(thread(read_frames));

    for (int i = 0; i < nw; i++)
    {
      tids.push_back(thread(handle_frames, i));
    }

    // thread read_thread = thread(read_frames);

    /*
    // read_thread.join();
    cout << frames_queues.at(0).size() << " main" << endl;*/

    // read_thread.join();

    for (thread &t : tids)
    { // await thread termination
      t.join();
    }
  }

  /*
    {
      utimer u("Sequential motion detection");
      handle_frames();
    }*/

  cout << number_of_frames_with_motion << endl;

  // Closes all the frames
  // destroyAllWindows();

  return 0;
}