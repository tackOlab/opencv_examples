#include <cstdio>
#include <opencv2/opencv.hpp>

int quantize(int val, double stepsize) { return val; }

int dequantize(int qidx, double stepsize) { return qidx; }

using namespace cv;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("ERROR: an input image is required.\n");
    return EXIT_FAILURE;
  }
  cv::Mat image0;
  image0 = cv::imread(argv[1]);
  if (image0.empty()) {
    printf("specified image %s is not found.\n", argv[1]);
    return EXIT_FAILURE;
  }
  double stepsize = 1.0;
  cv::Mat image1 = image0;
  int width = image0.cols;   // 画像の幅
  int height = image0.rows;  // 画像の高さ
  int ncomp = image0.channels();  // 色成分の数（カラー->3, モノクロ->1）
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      for (int c = 0; c < ncomp; ++c) {
        int val = image0.data[ncomp * (i * width + j) + c];
        int qidx = quantize(val, stepsize);
        int rec = dequantize(qidx, stepsize);
        image1.data[ncomp * (i * width + j) + c] = rec;
      }
    }
  }

  cv::imshow("before quantize", image0);
  cv::imshow("after reconstruction", image1);
  cv::waitKey();
  cv::destroyAllWindows();

  return EXIT_SUCCESS;
}