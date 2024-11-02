#include <iostream>
#include <opencv2/opencv.hpp>
#include <tuple>
#include <utility>

using coordinate = cv::Point;

// find the pupil in the frame
coordinate findPupil(cv::Mat &frame, coordinate lastLocation, int x1, int y1,
                     int x2, int y2);

coordinate spiral(cv::Mat &frame, coordinate lastLocation, int stepsize,
                  cv::Rect bound, int threshhold);

cv::Rect findPupilBox(cv::Mat &frame, coordinate pupilLocation, int threshhold);

static int checks = 0;
static long long checksum = 0;
static int frames = 0;
static cv::Mat displayframe;


int main() {

  // read the first frame of eye2.mp4
  cv::VideoCapture cap("eye3.mp4");

  cv::Mat frame;
  cap >> frame;

  displayframe = frame.clone();

  int width = frame.cols;
  int height = frame.rows;

  // create ouptut.mp4 with the same size as the input video
  cv::VideoWriter outputVideo;
  int ex = static_cast<int>(cv::VideoWriter::fourcc('X', '2', '6', '4'));
  outputVideo.open("output.mp4", ex, 30, cv::Size(width, height));

  int centerX = width / 2;
  int centerY = height / 2;

  const int boundingx1 = (width * 25) / 100;
  const int boundingx2 = (width * 75) / 100;
  const int boundingy1 = (height * 40) / 100;
  const int boundingy2 = (height * 90) / 100;

  coordinate lastPupilLocation = coordinate(centerX, centerY);

  // find the pupil
  coordinate pupil = findPupil(frame, lastPupilLocation, boundingx1, boundingy1,
                               boundingx2, boundingy2);

  // draw a circle at the pupil
  cv::circle(frame, pupil, 10, cv::Scalar(0, 0, 255), 2);
  // show the frame
  // cv::imshow("frame", frame);
  // cv::waitKey(0);

  // loop through all frames and find the pupil based on the last location
  while (true) {
    checks = 0;
    // get the next frame
    cap >> frame;
    frames++;

    displayframe = frame.clone();

    // if the frame is empty, break
    if (frame.empty()) {
      break;
    }

    cv::rectangle(displayframe,
                  cv::Rect(boundingx1, boundingy1, boundingx2 - boundingx1,
                           boundingy2 - boundingy1),
                  cv::Scalar(100, 0, 0));
    // find the pupil
    pupil = findPupil(frame, lastPupilLocation, boundingx1, boundingy1,
                      boundingx2, boundingy2);
    spiral(frame, lastPupilLocation, 10,
           cv::Rect(boundingx1, boundingy1, boundingx2 - boundingx1,
                    boundingy2 - boundingy1),
           35);
    // draw a circle at the pupil
    // cv::circle(frame, pupil, 10, cv::Scalar(0, 0, 255), 2);

    // draw box around pupil
    auto pupilBox = findPupilBox(frame, pupil, 50);
    // print the pupil box
    // std::cout << "Pupil box: " << pupilBox << std::endl;
    // draw the pupil box
    cv::rectangle(displayframe, pupilBox, cv::Scalar(0, 255, 0), 1);

    // set lastpupillocation to the center of the pupil box
    lastPupilLocation = cv::Point(pupilBox.x + pupilBox.width / 2,
                                  pupilBox.y + pupilBox.height / 2);
    // draw circle at center
    cv::circle(displayframe, lastPupilLocation, 10, cv::Scalar(0, 0, 255), 2);

    std::cout << "Checks: " << checks << std::endl;
    checksum += checks;
    checks = 0;

    // show the frame
    cv::imshow("frame", displayframe);
    cv::waitKey(0);

    // Write the frame to output file
    outputVideo.write(frame);
  }

  // release the video capture
  cap.release();

  // release the output video
  outputVideo.release();

  // destroy all windows
  cv::destroyAllWindows();

  std::cout << "Average checks per frame: " << checksum / frames << std::endl;
  return 0;
}

coordinate findPupil(cv::Mat &frame, coordinate lastLocation, int x1, int y1,
                     int x2, int y2) {
  // Get the size of the frame
  int width = frame.cols;
  int height = frame.rows;

  // Get the center
  int centerX = width / 2;
  int centerY = height / 2;

  return spiral(frame, lastLocation, 10, cv::Rect(x1, y1, x2 - x1, y2 - y1),
                35);
}

bool isPupil(cv::Mat &frame, coordinate where, int threshhold) {
  cv::Vec3b pixel = frame.at<cv::Vec3b>(where);
  checks++;
  cv::circle(displayframe, where, 1, cv::Scalar(250, 50, 0), 2);

  if (pixel[0] < threshhold && pixel[1] < threshhold && pixel[2] < threshhold) {
    return true;
  } else {
    return false;
  }
}

coordinate spiral(cv::Mat &frame, coordinate lastLocation, int stepsize,
                  cv::Rect bound, int threshhold) {
  // draw a square spiral with a circle every stepsize pixels until a bound is
  // hit
  int x = lastLocation.x;
  int y = lastLocation.y;
  int radius = 5;
  int direction = 0;

  // Print arguments:
  std::cout << "spiral: " << x << " " << y << " " << stepsize << " " << bound.x
            << " " << bound.y << " " << bound.width << " " << bound.height
            << std::endl;

  int range = 0;

  while (x < bound.width + bound.x && y < bound.height + bound.y &&
         x > bound.x && y > bound.y) {
    // draw a circle at the current location
    // cv::circle(frame, cv::Point(x, y), radius, cv::Scalar(255, 0, 255), 2);

    switch (direction) {
    case 0: {
      range++;
      direction++;
      for (int i = 0; i < range; i++) {
        x += i * stepsize;
        // cv::circle(frame, cv::Point(x, y), radius, cv::Scalar(255, 0, 255),
        // 2);
        if (isPupil(frame, {x, y}, threshhold)) {
          return coordinate(x, y);
        }
      }
    } break;
    case 1: {
      direction++;
      for (int i = 0; i < range; i++) {
        y += i * stepsize;
        // cv::circle(frame, cv::Point(x, y), radius, cv::Scalar(255, 0, 255),
        // 2);
        if (isPupil(frame, {x, y}, threshhold)) {
          return coordinate(x, y);
        }
      }
    } break;
    case 2: {
      range++;
      direction++;
      for (int i = 0; i < range; i++) {
        x -= i * stepsize;
        // cv::circle(frame, cv::Point(x, y), radius, cv::Scalar(255, 0, 255),
        // 2);
        if (isPupil(frame, {x, y}, threshhold)) {
          return coordinate(x, y);
        }
      }
    } break;
    case 3: {
      direction = 0;
      for (int i = 0; i < range; i++) {
        y -= i * stepsize;
        // cv::circle(frame, cv::Point(x, y), radius, cv::Scalar(255, 0, 255),
        // 2);
        if (isPupil(frame, {x, y}, threshhold)) {
          return coordinate(x, y);
        }
      }
    } break;
    }
  }
  return coordinate(lastLocation.x, lastLocation.y);
}

cv::Rect findPupilBox(cv::Mat &frame, coordinate pupilLocation,
                      int threshhold) {
  // March left, right, up, down and calculate the bounding area of the pupil
  // using the isPupil function

  auto constexpr steps = 25;
  auto constexpr distance = 20;

  // march left
  auto x = pupilLocation.x;
  auto y = pupilLocation.y;
  while (true) {
    int i = 0;
    if (!isPupil(frame, {x, y}, threshhold)) {
      if (!isPupil(frame, {x, y + distance}, threshhold) &&
          !isPupil(frame, {x, y - distance}, threshhold)) {
        for (; i < steps; i++) {
          x++;
          if (isPupil(frame, {x, y}, threshhold)) {
            break;
          }
        }
        break;
      }
    }
    x -= steps;
  }
  int left = x;
  x = pupilLocation.x;
  // march right
  while (true) {
    int i = 0;
    if (!isPupil(frame, {x, y}, threshhold)) {
      if (!isPupil(frame, {x, y + distance}, threshhold) &&
          !isPupil(frame, {x, y - distance}, threshhold)) {
        for (; i < steps; i++) {
          x--;
          if (isPupil(frame, {x, y}, threshhold)) {
            break;
          }
        }
        break;
      }
    }
    x += steps;
  }
  int right = x;
  x = pupilLocation.x;
  // march up
  while (true) {
    int i = 0;
    if (!isPupil(frame, {x, y}, threshhold)) {
      if (!isPupil(frame, {x + distance, y}, threshhold) &&
          !isPupil(frame, {x - distance, y}, threshhold)) {
        for (; i < steps; i++) {
          y++;
          if (isPupil(frame, {x, y}, threshhold)) {
            break;
          }
        }
        break;
      }
    }
    y -= steps;
  }
  int top = y;
  y = pupilLocation.y;
  // march down
  while (true) {
    int i = 0;
    if (!isPupil(frame, {x, y}, threshhold)) {
      if (!isPupil(frame, {x + distance, y}, threshhold) &&
          !isPupil(frame, {x - distance, y}, threshhold)) {
        for (; i < steps; i++) {
          y--;
          if (isPupil(frame, {x, y}, threshhold)) {
            break;
          }
        }
        break;
      }
    }
    y += steps;
  }
  int bottom = y;

  return {left, top, right - left, bottom - top};
}