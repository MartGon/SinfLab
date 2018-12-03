// OpenCV libs
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// C++ standard libs
#include <iostream>
#include <string>
#include <fstream>
#include <random>

cv::Mat readMatrixFromFile(std::string filename);

cv::Mat getDctBlock(cv::Mat block);

cv::Mat getiDctBlock(cv::Mat dctblock);

cv::Mat getDctImage(cv::Mat image);

cv::Mat getJSTEGImage(cv::Mat dctImage, std::vector<bool>& data, std::map<int32_t, uint32_t>& coeff_count);

cv::Mat getIDctImage(cv::Mat dctImage);

std::vector<bool> getDataFromTamperedImage(cv::Mat tImage, std::map<int32_t, uint32_t>& coeff_count);

void writeToOutputFile(std::fstream& file, int32_t data);

void writeHistogramFile(std::fstream& file, std::map< int32_t, uint32_t> map);

void printData(std::vector<bool> data);