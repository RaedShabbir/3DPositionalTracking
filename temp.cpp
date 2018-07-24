#include "stdafx.h"
#include "opencv2\opencv.hpp"
#include "opencv2\core.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\aruco.hpp"
#include "opencv2\calib3d.hpp"

#include <sstream>
#include <iostream>
#include <fstream>


using namespace cv;
using namespace std;

//enter key values in meters
const float calibrationSquareDimension = 0.0245f;
const float arucoSquareDimension = 0.5f;
const Size chessboardDimensions = Size(6, 9);

void createArucoMarkers()
{
	Mat outputMarker;
	//setup the predefined aruco marker dictionary (50 markers inside)
	Ptr <aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

	//iterate and write markers to project directory
	for (int i = 0; i < 8; i++)
	{
		aruco::drawMarker(markerDictionary, i, 500, outputMarker, 1);
		ostringstream convert; //must append i to marker name
		string imageName = "4x4Marker_";
		convert << imageName << i << ".jpg";
		imwrite(convert.str(), outputMarker);

	}
}

void createKnownBoardPositions(Size, boardSize, float squareEdgeLength, vector<Point3f>& corners)
{
	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			corners.push_back(Point3f(j * squareEdgeLength, i*squareEdgeLength, 0.0f)); //note z is zero since we have a flat plane
		}
	}


}

void getChessboardCorners(vector<Mat> images, vector<vector<Point3f>>&allFoundCorners, bool showResults = false)
{	//function for calibrating based on images only
	//so after receiving a vector full of images, we want to extract the corners
	for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++) {
		vector<Point2f> pointBuf;
		bool found = findChessboardCorners(*iter, Size(9,6), pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		if (found)
		{
			allFoundCorners.push_back(pointBuf);
		}
		if (showResults)
		{
			drawChessboardCorners(*iter, Size(9, 6), pointBuf, found);
			imshow("Corner Finder", *iter);
			waitKey(0);
		}
	}
}

int main(int argv, char** args)
{
	createArucoMarkers();
	//only run this once to generate the markers
	int framesPerSecond = 20;
	Mat frame;
	Mat drawToFrame;
	Mat cmeraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distanceCoefficients;
	vector<Mat> savedImages; //can save good calibrations
	vector<vector<Point2f>> markerCorners, rejectedCandidates;

	VideoCapture vid(0);

	if (!vid.isOpened())
	{
		return;
	}

	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);

	while (true)
	{
		if (!vid.read(frame)) //break if next frame not read
			break;

		vector<Vec2f> foundPoints;
		bool found = false;
		found = findChessboardCorners(frame, chessboardDimensions, )



	}

}
