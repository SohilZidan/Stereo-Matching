#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> 
#include <fstream>
#include <sstream>
#include "stereomatching.h"
#include "main.h"
#include "utils.h"


int main(int argc, char* argv[]) {

  ////////////////
  // Parameters //
  ////////////////

  // camera setup parameters
  const double focal_length = 3740;
  const double baseline = 160;

  // stereo estimation parameters
  //readMiddleburyDMin(argv[6]);
  const int dmin = (argc > 6)? readMiddleburyDMin(argv[6]): 200;
  const int window_size = (argc > 4)? std::stoi(argv[4]) : 5;
  const double weight =  (argc > 5) ? std::stod(argv[5]) : 1;
  const double scale = 1;

  ///////////////////////////
  // Commandline arguments //
  ///////////////////////////

  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " IMAGE1 IMAGE2 OUTPUT_FILE" << std::endl;
    return 1;
  }

  cv::Mat image1 = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
  cv::Mat image2 = cv::imread(argv[2], cv::IMREAD_GRAYSCALE);

  const std::string output_file = argv[3];

  if (!image1.data) {
    std::cerr << "No image1 data" << std::endl;
    return EXIT_FAILURE;
  }

  if (!image2.data) {
    std::cerr << "No image2 data" << std::endl;
    return EXIT_FAILURE;
  }


  std::cout << "------------------ Parameters -------------------" << std::endl;
  std::cout << "focal_length = " << focal_length << std::endl;
  std::cout << "baseline = " << baseline << std::endl;
  std::cout << "window_size = " << window_size << std::endl;
  std::cout << "occlusion weights = " << weight << std::endl;
  std::cout << "disparity added due to image cropping = " << dmin << std::endl;
  std::cout << "scaling of disparity images to show = " << scale << std::endl;
  std::cout << "output filename = " << argv[3] << std::endl;
  std::cout << "-------------------------------------------------" << std::endl;

  int height = image1.size().height;
  int width = image1.size().width;

  // disparity images
  cv::Mat naive_disparities = cv::Mat::zeros(height, width, CV_16UC1 );
  cv::Mat cv_naive_disparities = cv::Mat::zeros(height, width, CV_16UC1);
  cv::Mat l_disparity = cv::Mat::zeros(height, width, CV_16UC1);
  cv::Mat r_disparity = cv::Mat::zeros(height, width, CV_16UC1);

  // Naive Approach
  double matching_time = (double)cv::getTickCount();
  StereoEstimation_Naive(
    window_size, dmin, height, width,
    image1, image2,
    naive_disparities, scale);

  matching_time = ((double)cv::getTickCount() - matching_time) / cv::getTickFrequency();
  std::cout << "Naive stereo Matching: " << matching_time<< " sec"<< std::endl;
  
  // Dynamic Programming
  matching_time = (double)cv::getTickCount();
  StereoEstimation_DP(
      window_size,
      height,
      width,
      image1, image2,
      l_disparity, r_disparity,
      scale, weight
  );
  matching_time = ((double)cv::getTickCount() - matching_time) / cv::getTickFrequency();
  std::cout << "DP stereo Matching: " << matching_time << " sec" << std::endl;


  ////////////
  // Output //
  ////////////

  std::string params = "Params windsize" + std::to_string(window_size) + "-occweight" + std::to_string(weight);
  ////////////////////
  // Reconstruction //
  ////////////////////
  Disparity2PointCloud(
    output_file + params,
    image1,
    height, width, l_disparity,
    window_size, dmin, baseline, focal_length);

  
  // Convert to range 0 255
  // this help in generalization
  cv::Mat tmp;
  naive_disparities.convertTo(tmp, CV_8UC1);
  tmp.copyTo(naive_disparities);
  l_disparity.convertTo(tmp, CV_8UC1);
  tmp.copyTo(l_disparity);
  r_disparity.convertTo(tmp, CV_8UC1);
  tmp.copyTo(r_disparity);
  
  // save images
  std::stringstream out1;
  
  // cv::imwrite(out1.str(), naive_disparities);

  // out2 << output_file << "_DP_left" << params << ".png";
  // cv::imwrite(out2.str(), l_disparity);

  // out3 << output_file << "_DP_right" << params << ".png";
  // cv::imwrite(out3.str(), r_disparity);
  int winsize = 5;
  float spatial_sigma = 2.828427125;
  float spectral_sigma = 5;

  cv::Mat output = cv::Mat::zeros(height, width, CV_8UC1);;
  std::string suffix =  "_bilateral_wind" + 
                        std::to_string(winsize) +
                        "_spatial" + std::to_string(spatial_sigma) +
                        "_spectral" + std::to_string(spectral_sigma) + 
                        "_";

  BilateralFilter(l_disparity, output, spatial_sigma, spectral_sigma , winsize);
  cv::imshow("Bilateral_left_disp" + suffix, output);
  out1 << output_file << "_l_disparity_" << suffix << ".png";
  // std::cout << out1.str() << std::endl;
  cv::imwrite(out1.str(), output);

  // clearing stringstream
  out1.str(std::string());



  winsize = 9;
  spatial_sigma = 2.828427125;
  spectral_sigma = 5;
  suffix =  "bilateral_wind" + 
            std::to_string(winsize) +
            "spatial_" + std::to_string(spatial_sigma) +
            "spectral_" + std::to_string(spectral_sigma) +
            "_";
  // std::cout << suffix << std::endl;
  BilateralFilter(l_disparity, output, spatial_sigma, spectral_sigma , winsize);
  cv::imshow("Bilateral_left_disp"+suffix, output);
  out1 << output_file << "_l_disparity_" << suffix << ".png";
  // std::cout << out1.str() << std::endl;
  cv::imwrite(out1.str(), output);
  // out1.clear();
  // show images
  // Naive
  /*double minVal;
  double maxVal;
  cv::minMaxLoc(naive_disparities, &minVal, &maxVal, NULL, NULL);
  std::cout << "min: " << minVal << " ,max: " << maxVal << std::endl;*/
  cv::namedWindow("Disparity - Naive", cv::WINDOW_AUTOSIZE);
  cv::imshow("Disparity - Naive", naive_disparities);
  
  // DP
  cv::namedWindow("Disparity - DP - left", cv::WINDOW_AUTOSIZE);
  cv::imshow("Disparity - DP - left", l_disparity);
  
  cv::namedWindow("Disparity - DP - right", cv::WINDOW_AUTOSIZE);
  cv::imshow("Disparity - DP - right", r_disparity);

  // upsampled data
  
  cv::waitKey(0);

  return 0;
}


void BilateralFilter(
  const cv::Mat& input, 
  cv::Mat& output,
  float spatial_sigma, 
  float spectral_sigma,
  const int window_size) {

  const auto width = input.cols;
  const auto height = input.rows;

  cv::Mat gaussianKernel = CreateGaussianKernel(window_size, spatial_sigma, false);

  // TEMPORARY CODE
  for (int r = 0; r < height; ++r) {
    for (int c = 0; c < width; ++c) {
      output.at<uchar>(r, c) = 0;
    }
  }

  auto d = [](float a, float b) {
    return std::abs(a - b);
  };
  // static float sigma;
  // sigma = spectral_sigma;
  auto p = [](float val) { 
    const float sigma = 5;
    const float sigmaSq = sigma * sigma;
    const float normalization = std::sqrt(2*M_PI) * sigma;
    return (1 / normalization) * std::exp(-val / (2 * sigmaSq));
  };

  for (int r = window_size / 2; r < height - window_size / 2; ++r) {
    for (int c = window_size / 2; c < width - window_size / 2; ++c) {

      float sum_w = 0;
      float sum = 0;

      for (int i = -window_size / 2; i <= window_size / 2; ++i) {
        for (int j = -window_size / 2; j <= window_size / 2; ++j) {

          float range_difference
            = d(input.at<uchar>(r, c), input.at<uchar>(r + i, c + j));

          float w 
            = p(range_difference)
            * gaussianKernel.at<float>(i + window_size / 2, j + window_size / 2);

          sum
            += input.at<uchar>(r + i, c + j) * w;
          sum_w
            += w;
        }
      }

      output.at<uchar>(r, c) = sum / sum_w;

    }
  }
}


cv::Mat CreateGaussianKernel(
  int window_size,
  double sigmaAll,
  bool auto_sigma
  ) 
{
  cv::Mat kernel(cv::Size(window_size, window_size), CV_32FC1);

  int half_window_size = window_size / 2;

  // see: lecture_03_slides.pdf, Slide 13
  const double k = 2.5;
  const double r_max = std::sqrt(2.0 * half_window_size * half_window_size);
  const double sigma = sigmaAll;//r_max / k;
  if(auto_sigma)
  {
    double sigma = r_max / k;
  }

  // sum is for normalization 
  float sum = 0.0;

  for (int x = -window_size / 2; x <= window_size / 2; x++) {
    for (int y = -window_size / 2; y <= window_size / 2; y++) {
      float val = exp(-(x * x + y * y) / (2 * sigma * sigma));
      kernel.at<float>(x + window_size / 2, y + window_size / 2) = val;
      sum += val;
    }
  }

  // normalising the Kernel 
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j)
      kernel.at<float>(i, j) /= sum;

  // note that this is a naive implementation
  // there are alternative (better) ways
  // e.g. 
  // - perform analytic normalisation (what's the integral of the gaussian? :))
  // - you could store and compute values as uchar directly in stead of float
  // - computing it as a separable kernel [ exp(x + y) = exp(x) * exp(y) ] ...
  // - ...

  return kernel;
}