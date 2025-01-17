#include <iostream>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "utils.hpp"

void loadCalibrationData(cv::Mat &P_rect_00, cv::Mat &RT)
{
	RT.at<double>(0, 0) = 7.533745e-03;
	RT.at<double>(0, 1) = -9.999714e-01;
	RT.at<double>(0, 2) = -6.166020e-04;
	RT.at<double>(0, 3) = -4.069766e-03;
	RT.at<double>(1, 0) = 1.480249e-02;
	RT.at<double>(1, 1) = 7.280733e-04;
	RT.at<double>(1, 2) = -9.998902e-01;
	RT.at<double>(1, 3) = -7.631618e-02;
	RT.at<double>(2, 0) = 9.998621e-01;
	RT.at<double>(2, 1) = 7.523790e-03;
	RT.at<double>(2, 2) = 1.480755e-02;
	RT.at<double>(2, 3) = -2.717806e-01;
	RT.at<double>(3, 0) = 0.0;
	RT.at<double>(3, 1) = 0.0;
	RT.at<double>(3, 2) = 0.0;
	RT.at<double>(3, 3) = 1.0;

	P_rect_00.at<double>(0, 0) = 7.215377e+02;
	P_rect_00.at<double>(0, 1) = 0.000000e+00;
	P_rect_00.at<double>(0, 2) = 6.095593e+02;
	P_rect_00.at<double>(0, 3) = 0.000000e+00;
	P_rect_00.at<double>(1, 0) = 0.000000e+00;
	P_rect_00.at<double>(1, 1) = 7.215377e+02;
	P_rect_00.at<double>(1, 2) = 1.728540e+02;
	P_rect_00.at<double>(1, 3) = 0.000000e+00;
	P_rect_00.at<double>(2, 0) = 0.000000e+00;
	P_rect_00.at<double>(2, 1) = 0.000000e+00;
	P_rect_00.at<double>(2, 2) = 1.000000e+00;
	P_rect_00.at<double>(2, 3) = 0.000000e+00;
}
void lidarOnImage(const cv::Mat &img, const std::vector<LidarPoint> &lidarPoints)
{
	// store calibration data in OpenCV matrices
	cv::Mat P_rect_00(3, 4, cv::DataType<double>::type);
	cv::Mat RT(4, 4, cv::DataType<double>::type);

	loadCalibrationData(P_rect_00, RT);

	// project lidar points
	cv::Mat visImg = img.clone();
	cv::Mat overlay = visImg.clone();

	cv::Mat X(4, 1, cv::DataType<double>::type);
	cv::Mat Y(3, 1, cv::DataType<double>::type);

	for (auto it = lidarPoints.begin(); it != lidarPoints.end(); ++it)
	{
		float maxX = 25.0, maxY = 6.0, minZ = -1.4;

		if (it->x > maxX || it->x < 0.0 || abs(it->y) > maxY || it->z < minZ || it->r < 0.01)
		{
			continue;
		}

		X.at<double>(0, 0) = it->x;
		X.at<double>(1, 0) = it->y;
		X.at<double>(2, 0) = it->z;
		X.at<double>(3, 0) = 1;

		Y = P_rect_00 * RT * X;
		cv::Point pt;
		pt.x = Y.at<double>(0, 0) / Y.at<double>(0, 2);
		pt.y = Y.at<double>(1, 0) / Y.at<double>(0, 2);

		float val = it->x;
		float maxVal = 20.0;
		int red = std::min(255, (int)(255 * abs((val - maxVal) / maxVal)));
		int green = std::min(255, (int)(255 * (1 - abs((val - maxVal) / maxVal))));
		cv::circle(overlay, pt, 5, cv::Scalar(0, green, red), -1);
	}

	float opacity = 0.6;
	cv::addWeighted(overlay, opacity, visImg, 1 - opacity, 0, visImg);

	std::string windowName = "LiDAR data on image overlay";

	cv::namedWindow(windowName, 3);
	cv::imshow(windowName, visImg);
	cv::waitKey(0);
}

int main(int argc, char const *argv[])
{
	cv::Mat img;
	std::vector<LidarPoint> lidar_pts;

	// load image in file
	img = cv::imread(argv[1]);

	// load Lidar points from file
	readLidarPts(argv[2], lidar_pts);
	lidarOnImage(img, lidar_pts);
}