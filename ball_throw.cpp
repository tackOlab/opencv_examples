#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <opencv2/opencv.hpp>

int main() {
  constexpr double PI = 3.1415926;
  constexpr double g = 9.8;
  double h0 = 1.0;
  double v0 = 10.0;
  double max_d = 0.0;
  double max_h = 0.0;
  for (int theta = 0; theta < 90; theta += 5) {
    double rad = PI * (double)theta / 180;
    double h = (v0 * v0 * sin(rad) * sin(rad)) / (2 * g) + h0;
    double d =
        v0 * cos(rad) *
        (v0 * sin(rad) + sqrt(v0 * v0 * sin(rad) * sin(rad) + 2 * g * h0)) / g;
    max_d = (max_d < d) ? d : max_d;
    max_h = (max_h < h) ? d : max_h;
    printf("%d %f %f\n", theta, h, d);
  }

  double x, y;
  int ix, iy;
  cv::Mat field = cv::Mat::zeros(512, 512, CV_8UC3);
  for (int theta = 0; theta < 90; theta += 5) {
    printf("theta = %d\n", theta);
    printf("%f\n", sqrt(4 - (1.998 + 0.002) * (1.998 + 0.002)));

    cv::Scalar color = cv::Scalar(rand() % 256, rand() % 256, rand() % 256);
    for (double t = 0.0; t < 5.0; t += 0.01) {
      double rad = PI * (double)theta / 180;
      x = v0 * cos(rad) * t;
      y = h0 + v0 * sin(rad) * t - 0.5 * g * t * t;
      // printf("%f %f %f\n", t, x, y);
      ix = (int)((x / max_d) * 512 + 0.5);
      iy = 512 - (int)(y / max_h * 512 + 0.5);
      if (iy > 512) {
        break;
      }
      // printf("%f %f %d %d\n", x, y, ix, iy);
      cv::Point2i p = cv::Point2i(ix, iy);
      cv::circle(field, p, 4, color, -1);
      cv::imshow("result", field);
      // cv::circle(field, p, 4, cv::Scalar(0, 0, 0), -1);
      int keycode = cv::waitKey(1);
      if (keycode == 'q') {
        return EXIT_FAILURE;
      }
    }
  }
  cv::waitKey();
}