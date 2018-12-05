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

cv::Mat getIDctImage(cv::Mat dctImage);

cv::Mat getJSTEGImage(cv::Mat dctImage, std::vector<bool>& data, bool everyCoeff = false);

cv::Mat getF3Image(cv::Mat dctImage, std::vector<bool>& data, bool everyCoeff = false);

std::map<int32_t, uint32_t> getCoeffMap(const cv::Mat& dctImage, bool everyCoeff = false);

std::vector<bool> getDataFromTamperedImage(cv::Mat tImage);

void writeToOutputFile(std::fstream& file, int32_t data);

void writeHistogramFile(std::fstream& file, std::map< int32_t, uint32_t> map);

void printData(std::vector<bool> data);