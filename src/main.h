#pragma once

void StereoEstimation_DP(
    int height,
    int width,
    cv::Mat& image1, cv::Mat& image2,
    cv::Mat& l_disparity,
    cv::Mat& r_disparity,
    const double& scale, const double& weight
    
);

void StereoEstimation_Naive(
  const int& window_size,
  const int& dmin,
  int height,
  int width,
  cv::Mat& image1, cv::Mat& image2, cv::Mat& naive_disparities, const double& scale);

void Disparity2PointCloud(
  const std::string& output_file,
  int height, int width, cv::Mat& disparities,
  const int& window_size,
  const int& dmin, const double& baseline, const double& focal_length);
