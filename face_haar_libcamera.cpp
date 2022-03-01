#include <cstdio>
#include <opencv2/opencv.hpp>

int main() {
  cv::Mat frame;
  cv::Mat output;
  cv::CascadeClassifier detector_face, detector_eye;
  // path to XML files might be different. Check your install of OpenCV.
  detector_face.load(
      "/usr/share/opencv4/haarcascades/"
      "haarcascade_frontalface_alt.xml");
  detector_eye.load(
      "/usr/share/opencv4/haarcascades/"
      "haarcascade_eye.xml");
  const int cap_width = 640;
  const int cap_height = 480;
  cv::VideoCapture camera(0);
  if (camera.isOpened() == false) {
    printf("ERROR: cannot open the camera.\n");
    return EXIT_FAILURE;
  }
  camera.set(cv::CAP_PROP_FRAME_WIDTH, cap_width);
  camera.set(cv::CAP_PROP_FRAME_HEIGHT, cap_height);
  while (true) {
    if (camera.read(frame) == false) {
      printf("ERROR: cannot grab a frame\n");
      break;
    }
    std::vector<cv::Rect> faces, eyes;
    detector_face.detectMultiScale(frame, faces, 1.1, 3, 0, cv::Size(20, 20));
    detector_eye.detectMultiScale(frame, eyes, 1.1, 3, 0, cv::Size(20, 20));
    for (int i = 0; i < faces.size(); ++i) {
      rectangle(frame, faces[i], cv::Scalar(0, 0, 255), 3);
    }
    for (int i = 0; i < eyes.size(); ++i) {
      rectangle(
          frame, cv::Point(eyes[i].x, eyes[i].y),
          cv::Point(eyes[i].x + eyes[i].width, eyes[i].y + eyes[i].height),
          cv::Scalar(0, 255, 0), 3);
    }
    cv::flip(frame, output, 1);
    cv::imshow("face detection", output);
    int keycode = cv::waitKey(1);
    if (keycode == 'q') {
      break;
    }
  }
  cv::destroyAllWindows();

  return EXIT_SUCCESS;
}