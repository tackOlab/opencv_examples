#include <cmath>
#include <cstdio>
#include <opencv2/opencv.hpp>

float qmatrixL[64] = {
    16, 11, 10, 16, 24,  40,  51,  61,  12, 12, 14, 19, 26,  58,  60,  55,
    14, 13, 16, 24, 40,  57,  69,  56,  14, 17, 22, 29, 51,  87,  80,  62,
    18, 22, 37, 56, 68,  109, 103, 77,  24, 35, 55, 64, 81,  104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99};

float qmatrixC[64] = {17, 18, 24, 47, 99, 99, 99, 99, 18, 21, 26, 66, 99,
                      99, 99, 99, 24, 26, 56, 99, 99, 99, 99, 99, 47, 66,
                      99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
                      99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
                      99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99};

float quantize(float val, double stepsize) {
  float sign = 1.0;
  val /= stepsize;
  if (val < 0.0) {
    sign = -1.0;
  }
  val = fabs(val);
  return sign * floorf(val);
}

float dequantize(float qidx, double stepsize) {
  qidx *= stepsize;
  float sign = 1.0;
  if (qidx < 0.0) {
    sign = -1.0;
  }
  return sign * floorf(fabs(qidx) + 0.5);
}

void block_proc(cv::Mat &in, cv::Mat &out, int y, int x, int bh, int bw,
                float qscale, int c) {
  cv::Mat src = in(cv::Rect(x, y, bw, bh)).clone();
  cv::Mat dst = out(cv::Rect(x, y, bw, bh)).clone();
  cv::Mat fsrc, tmp, fdst;
  src.convertTo(fsrc, CV_32F);
  cv::dct(fsrc, tmp);
  float *sp = (float *)tmp.data;
  float *delta = qmatrixL;
  if (c) {
    delta = qmatrixL;
  }

  for (int i = 0; i < bh; ++i) {
    for (int j = 0; j < bw; ++j) {
      float stepsize = delta[i * bw + j] * qscale;
      float qidx = quantize(sp[i * bw + j], stepsize);
      sp[i * bw + j] = dequantize(qidx, stepsize);
    }
  }
  cv::dct(tmp, fdst, cv::DCT_INVERSE);
  sp = (float *)fdst.data;
  for (int i = 0; i < bh; ++i) {
    for (int j = 0; j < bw; ++j) {
      if (*sp > 255) {
        *sp = 255.0;
      }
      if (*sp < 0) {
        *sp = 0.0;
      }
      out.data[(y + i) * out.cols + x + j] = (unsigned char)(*sp++);
    }
  }
}

double psnr(cv::Mat &img0, cv::Mat &img1) {
  if (img0.cols != img1.cols || img0.rows != img1.rows ||
      img0.channels() != img1.channels()) {
    printf("ERROR in psnr\n");
    exit(EXIT_FAILURE);
  }
  int ncomp = img0.channels();
  int width = img0.cols;
  int height = img0.rows;
  long sum = 0;
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      for (int c = 0; c < ncomp; ++c) {
        int d = img0.data[ncomp * (i * width + j) + c] -
                img1.data[ncomp * (i * width + j) + c];
        d *= d;
        sum += d;
      }
    }
  }
  double mse = sum / (width * height * ncomp);
  return 10 * log10(255 * 255 / mse);
}

int main(int argc, char *argv[]) {
  // read input image
  if (argc < 2) {
    printf("ERROR: an input image is required.\n");
    return EXIT_FAILURE;
  }
  cv::Mat inputImg;
  inputImg = cv::imread(argv[1]);
  if (inputImg.empty()) {
    printf("specified image %s is not found.\n", argv[1]);
    return EXIT_FAILURE;
  }
  int QF = 85;
  // check Qfactor
  if (argc > 2) {
    QF = atoi(argv[2]);
  }
  while (true) {
    printf("QF = %d\n", QF);
    // RGB -> YCbCr
    cv::Mat imgYCbCr;
    cv::cvtColor(inputImg, imgYCbCr, cv::COLOR_BGR2YCrCb);

    float qscale;
    if (QF < 50) {
      qscale = floorf(50.0 / (float)QF);
    } else {
      qscale = 2.0 - 2 * ((float)QF / 100.0);
    }
    if (qscale == 0.0) {
      qscale = 0.5 / 100;
    }

    std::vector<cv::Mat> planes_in, planes_out;
    cv::Mat c1, c2;
    cv::split(imgYCbCr, planes_in);
    planes_out.push_back(planes_in[0]);
    cv::resize(planes_in[1], c1, cv::Size(), 0.5, 0.5);
    cv::resize(planes_in[2], c2, cv::Size(), 0.5, 0.5);
    planes_out.push_back(c1);
    planes_out.push_back(c2);

    planes_in = planes_out;
    cv::Mat fdst;
    int ncomp = planes_in.size();  // 色成分の数（カラー->3, モノクロ->1）
    for (int c = 0; c < ncomp; ++c) {
      int width = planes_out[c].cols;   // 色成分の幅
      int height = planes_out[c].rows;  // 色成分の高さ
      // planes_out[c].convertTo(fdst, CV_32F);
      for (int i = 0; i <= height - 8; i += 8) {
        for (int j = 0; j <= width - 8; j += 8) {
          block_proc(planes_in[c], planes_out[c], i, j, 8, 8, qscale, c);
        }
      }
      // planes_out[c] = fdst;
    }
    cv::Mat tmp, imgRGB;
    // planes_out[0].convertTo(planes_out[0], CV_32F);
    cv::resize(planes_out[1], planes_out[1], cv::Size(), 2.0, 2.0);
    cv::resize(planes_out[2], planes_out[2], cv::Size(), 2.0, 2.0);
    cv::merge(planes_out, imgYCbCr);
    cv::cvtColor(imgYCbCr, imgRGB, cv::COLOR_YCrCb2BGR);
    printf("PSNR = %f [dB]\n", psnr(inputImg, imgRGB));
    cv::imshow("before quantize", inputImg);
    cv::imshow("after reconstruction", imgRGB);
    int keycode = cv::waitKey();
    if (keycode == 'u') {
      QF++;
    }
    if (keycode == 'd') {
      QF--;
    }
    if (QF > 100) {
      QF = 100;
    }
    if (QF < 0) {
      QF = 0;
    }
    if (keycode == 'q') {
      break;
    }
  }
  cv::destroyAllWindows();

  return EXIT_SUCCESS;
}