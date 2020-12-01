#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> 
#include <fstream>
#include <sstream>
#include "stereomatching.h"
#include "filters.h"
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