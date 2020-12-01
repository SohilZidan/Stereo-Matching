#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include "stereomatching.h"


void StereoEstimation_DP(
    const int& window_size,
    int height,
    int width,
    cv::Mat& image1, cv::Mat& image2,
    cv::Mat& l_disparity,
    cv::Mat& r_disparity,
    const double& scale, 
    const double& weight
) 
{
    int half_window_size = window_size / 2;

    cv::Mat C;// = cv::Mat::zeros(width, width, CV_8UC1);
    cv::Mat M;// = cv::Mat::ones(cv::Size(width, width), CV_8UC1);

    cv::Mat disleft = cv::Mat::zeros(height, width, CV_16UC1);
    cv::Mat disright = cv::Mat::zeros(height, width, CV_16UC1);
    //float lambda = 100; // occlusion cost

#pragma omp parallel for
    for (int row = 0 + half_window_size; row < height - half_window_size; ++row) {
        C = cv::Mat::zeros(width, width, CV_32FC1);
        M = cv::Mat::ones(width, width, CV_8UC1);
        /*std::cout
            << "Calculating disparities for the DP approach... "
            << std::ceil((row / static_cast<double>(height - 1)) * 100) << "%\r"
            << std::flush;
            */
           

        // initialization
        for (int k = 0 + half_window_size; k < width - half_window_size/* - half_window_size*/; k++)
        {
            C.at<float>(k, 0) = k * weight;
        }
        for (int k = 0; k < width/* - half_window_size*/; k++)
        {
            C.at<float>(0, k) = k * weight;
        }

        for (int k = 0 + half_window_size; k < width - half_window_size; k++)
        {
            M.at<uchar>(0, k) = 3; // occluded from right
            M.at<uchar>(k, 0) = 2; // occluded from left
        }


        for (int i = 1 + half_window_size; i < width - half_window_size ; i++)
        {
            for (int j = 1 + half_window_size; j < width - half_window_size; j++)
            {
                int dissimilarity = 0;
                for (int u = -half_window_size; u <= half_window_size; u++)
                {
                    for (int v = -half_window_size; v <= half_window_size; v++)
                    {
                        int val_left = image1.at<uchar>(row + u, i + v);
                        int val_right = image2.at<uchar>(row + u, j + v);
                        dissimilarity += (val_left - val_right) * (val_left - val_right);
                    }

                }
                //int val_left = image1.at<uchar>(row, i);
                //int val_right = image2.at<uchar>(row, j);
                //dissimilarity = (val_left - val_right) * (val_left - val_right);

                float min1 = C.at<float>(i - 1, j - 1) + dissimilarity; // match
                float min2 = C.at<float>(i - 1, j) + weight; // left occlusion
                float min3 = C.at<float>(i, j - 1) + weight; // right occlusion
                
                // finding optimal cost 
                float min_c = std::min({ min1,min2, min3 });
                C.at<float>(i, j) = min_c;

                float eps = 1.0e-05;
                if ( fabs(min_c - min1) <= eps)
                    M.at<uchar>(i, j) = 1; // match
                else if (fabs(min_c - min2) <= eps)
                    M.at<uchar>(i, j) = 2; // occluded from left
                else if (fabs(min_c - min3) <= eps)
                    M.at<uchar>(i, j) = 3; // occluded from right
                
            }

        }
        //std::cin.get();
        /*cv::namedWindow("Left Disparity", cv::WINDOW_AUTOSIZE);
        cv::imshow("Left Disparity", C);
        break;*/
        //std::cout << "Computing disparity" << std::endl;
        int i = width - half_window_size - 1;
        int j = i;
        while (i > 0 + half_window_size && j > 0 + half_window_size)
        {
            switch (M.at<uchar>(i, j))
            {
            case 1:
                disleft.at<uint16_t>(row, i) = abs(i-j) * scale;
                /*if (abs(i - j) * scale > 255) {
                    std::cout << abs(i - j) * scale << std::endl;
                    std::cout << disleft.at<uint16_t>(row, i) << std::endl;
                }*/
                disright.at<uint16_t>(row, j) = abs(j-i) * scale;
                i--;
                j--;
                break;
            case 2:
                disleft.at<uint16_t>(row, i) = 0;
                i--;
                break;
            case 3:
                disright.at<uint16_t>(row, j) = 0;
                j--;
                break;
            }
            
        }
    }

    /*cv::namedWindow("Left Disparity", cv::WINDOW_AUTOSIZE);
    cv::imshow("Left Disparity", disleft);

    cv::namedWindow("Right Disparity", cv::WINDOW_AUTOSIZE);
    cv::imshow("Right Disparity", disright);*/
    //disleft.convertTo(l_disparity,CV_8UC1, 1.0/255, 0.0);
    //cv::normalize(disleft, l_disparity, 0, 255, cv::NORM_MINMAX);
    //cv::normalize(disright, r_disparity, 0, 255, cv::NORM_MINMAX);
    l_disparity = disleft;
    r_disparity = disright;
    //std::cout << disparities;
}

void StereoEstimation_Naive(
  const int& window_size,
  const int& dmin,
  int height,
  int width,
  cv::Mat& image1, cv::Mat& image2, cv::Mat& naive_disparities, const double& scale)
{
  int half_window_size = window_size / 2;
  int dims[3] = { height, width, width };
  // store the patch window result of each coordinate with each disparity value to use is with box filtering
  cv::Mat box_filter = cv::Mat::zeros(3, dims, CV_32F);
  // false at the first row scan
  bool first_row_skipped = false;

  for (int i = half_window_size; i < height - half_window_size; ++i) {

    std::cout
      << "Calculating disparities for the naive approach... "
      << std::ceil(((i - half_window_size + 1) / static_cast<double>(height - window_size + 1)) * 100) << "%\r"
      << std::flush;

#pragma omp parallel for
    for (int j = half_window_size; j < width - half_window_size; ++j) {
      int min_ssd = INT_MAX;
      int disparity = 0;

      for (int d = -j + half_window_size; d < width - j - half_window_size; ++d) {
          int ssd = 0;

          // TODO: sum up matching cost (ssd) in a window
          if (!first_row_skipped || true)
          {
              for (int u = -half_window_size; u <= half_window_size; u++)
              {
                  for (int v = -half_window_size; v <= half_window_size; v++)
                  {
                      int val_left = image1.at<uchar>(i + u, j + v);
                      int val_right = image2.at<uchar>(i + u, j + v + d);
                      ssd += (val_left - val_right) * (val_left - val_right);
                  }

              }
              
          }
          else
          {
              // ssd of x,y-1
              /*ssd = box_filter.at<uchar>(i- 1, j, d);
              std::cout << "ssd = " << ssd << std::endl;
              std::cout << i - half_window_size - 1 << ", " << j - half_window_size << ", " << d << std::endl;
              std::cout << "i = " << i << ", j = " << j <<  std::endl;*/
              int ss;
              

              // green part
              int included_row = 0;
              //std::cout << "Including indices:" << std::endl;
              for (int k = -half_window_size; k <= half_window_size; k++)
              {
                  //std::cout << "(" << i + half_window_size << ", " << j + k << ")" << ", ";
                  int val_left = image1.at<uchar>(i + half_window_size, j + k);
                  int val_right = image2.at<uchar>(i + half_window_size, j + k + d);
                  included_row += (val_left - val_right) * (val_left - val_right);
              }
              //std::cout << std::endl;

              // red part
              int excluded_row = 0;
              //std::cout << "Excluding indices:" << std::endl;
              for (int k = -half_window_size; k <= half_window_size; k++)
              {
                  //std::cout << "(" << i - half_window_size - 1 << ", " << j + k << ")" << ", ";
                  int val_left = image1.at<uchar>(i - half_window_size - 1, j + k);
                  int val_right = image2.at<uchar>(i - half_window_size - 1, j + k + d);
                  excluded_row += (val_left - val_right) * (val_left - val_right);
              }

              //std::cin.get();
              //ssd = prev_ssd;
              /*std::cout << "included = " << included_row << std::endl;
              std::cout << "excluded = " << excluded_row << std::endl;
              ssd += included_row - excluded_row;
              std::cout << "new ssd = " << ssd << std::endl;
              std::cout << "--------------------------------" <<std::endl;*/
              // red: i - half_window_size - 1, x= j + (-half_window_size --> half_window_size)
              // green: i + n, x= j + (-half_window_size --> half_window_size)
          }

          // box_filter.at<uchar>(i, j, d) = ssd;
          /*if (d == 14) {
              std::cout << std::endl;
              std::cout << ssd << std::endl;
              std::cout << i << std::endl;
              std::cout << j << std::endl;
              std::cout << d << std::endl;
              std::cout << i - half_window_size << std::endl;
              std::cout << j - half_window_size << std::endl;
              std::cout << box_filter.at<uchar>(i, j, d) << std::endl;
              std::cin.get();
          }*/

          if (ssd < min_ssd) {
              min_ssd = ssd;
              disparity = d;
          }
      }

      naive_disparities.at<uint16_t>(i - half_window_size, j - half_window_size) = std::abs(disparity) * scale;
    }
    first_row_skipped = true;
  }

  std::cout << "Calculating disparities for the naive approach... Done.\r" << std::flush;
  std::cout << std::endl;
}



void Disparity2PointCloud(
  const std::string& output_file,
  cv::Mat& image1,
  int height, int width, cv::Mat& disparities,
  const int& window_size,
  const int& dmin, const double& baseline, const double& focal_length)
{
  std::stringstream out3d;
  out3d << output_file << ".txt";
  std::ofstream outfile(out3d.str());

  // RGB grey image
  cv::Mat bgrimg;
  cv::cvtColor(image1, bgrimg, cv::COLOR_GRAY2BGR);

  for (int i = 0; i < height - window_size; ++i) {
    std::cout << "Reconstructing 3D point cloud from disparities... " << std::ceil(((i) / static_cast<double>(height - window_size + 1)) * 100) << "%\r" << std::flush;
    for (int j = 0; j < width - window_size; ++j) {
      if (disparities.at<uchar>(i, j) == 0) continue;
      // Triangulation
      const double Z = focal_length * baseline / (disparities.at<uchar>(i,j) + dmin);
      const double X = j * Z / focal_length;
      const double Y = i * Z / focal_length;
      const double R = bgrimg.at<cv::Vec3b>(i, j)[2];
      const double G = bgrimg.at<cv::Vec3b>(i, j)[1];
      const double B = bgrimg.at<cv::Vec3b>(i, j)[0];

      outfile << X << " " << Y << " " << Z << " " << R << " " << G << " " << B << std::endl;
    }
  }

  std::cout << "Reconstructing 3D point cloud from disparities... Done.\r" << std::flush;
  std::cout << std::endl;
}

