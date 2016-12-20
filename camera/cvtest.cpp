#include <cv.h>
#include <highgui.h>
#include <ctype.h>

int main(int argc, char **argv){
  cv::Mat frame;
  int c;

  cv::VideoCapture capture(0);
  cv::namedWindow("window", CV_WINDOW_AUTOSIZE);

  while (capture.isOpened()) {
    capture.read(frame);
    cv::imshow ("window", frame);
    c = cv::waitKey(2);
    if (c == '\x1b') break;
  }
  cv::destroyWindow ("window");
  return 0;

}
