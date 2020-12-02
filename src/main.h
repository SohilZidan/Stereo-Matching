#pragma once

cv::Mat getNoise(cv::Mat& im, uchar mean = 0, uchar stddev = 25);

void creatExperiments(
  cv::Mat im, std::string output_file,
  int height,
  int width,
  float spatial_sigmas[], 
  int size1, 
  float spectral_sigmas[], 
  int size2, 
  const int winsize = 5
  );