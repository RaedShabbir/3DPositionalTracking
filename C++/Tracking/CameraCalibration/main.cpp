#include "stdafx.h"
#include "opencv2\opencv.hpp"
#include "opencv2\core.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\aruco.hpp" //contains marker functionality 
#include "opencv2\calib3d.hpp"
#include <sstream>
#include <iostream>
#include <fstream>


using namespace cv;
using namespace std;

//enter key values in meters
const float chessSquareDimension = 0.0245f;
const float markerDimension = 0.05f;
const float sensorWidth = 0.002f;
const float sensorHeight = 0.003f;
const Size chessboardSize = Size(6, 9);

//Camera calibration is completed using a chessboard image in this app
//please refer to opencv documentation for different calibration strategies 

void createMarkers()
{	//here the unique aruco markers are iteratively chosen from a dicitonary and saved to the working directory 
	Mat outputMarker;
	//setup the predefined aruco marker dictionary (50 markers inside)
	Ptr <aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

	//iterate through dictionary and save up to the number of markers you want 
	for (int i = 0; i < 8; i++)
	{
		aruco::drawMarker(markerDictionary, i, 500, outputMarker, 1); //saves the i't marker from the markerDictionary with pixel size 500x500, border width of 1 bit
		ostringstream convert; 
		string img = "4x4Marker_";
		convert << img << i << ".jpg"; //append i to the marker name for identification 
		imwrite(convert.str(), outputMarker);
	}
}

void createBoardPositions(Size boardSize, float squareLen, vector<Point3f>& corners)
{	//here we used a double nested loop to iterativley populate the corners vector with the locations of the squares on the board 
	//in world space coords (i.e. meters)  
	for (int i = 0; i < boardSize.height; i++) //loop over the board height 
	{
		for (int j = 0; j < boardSize.width; j++) //loop over the board width 
		{
			corners.push_back(Point3f(j * squareLen, i*squareLen, 0.0f)); //note z is zero since we have a flat plane 
		}
	}


}

void getBoardCorners(vector<Mat> images, vector<vector<Point2f>>&allFoundCorners, bool showResults = false)
{	//so after receiving a vector full of chessboard images, we want to iterate through each one and extract the corners 
	for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++) {
		
		vector<Point2f> pointList; //vector to contain found corners 
		bool found = findChessboardCorners(*iter, Size(9, 6), pointList, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE); //finds corners
		
		if (found)
		{
			allFoundCorners.push_back(pointList); //push corners into vector of vectors 
		}
		
		if (showResults)
		{
			drawChessboardCorners(*iter, Size(9, 6), pointList, found); //draw corners onto image 
			imshow("Corner Finder", *iter); //show image 
			waitKey(0);
		}
	}
}

void cameraCalibration(vector<Mat> calibImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distortionCoeffs)
{	//calibrates the camera using getBoardCorners and createBoardPositions 
	vector<vector<Point2f>> boardImgSpacePoints; 
	getBoardCorners(calibImages, boardImgSpacePoints, false); //get board corners which will populate boardImgSpacePoints 

	vector<vector<Point3f>> boardWorldSpacePoints(1); //known board positions for each image (in real 3d space), initially of size 1
	createBoardPositions(boardSize, squareEdgeLength, boardWorldSpacePoints[0]);
	boardWorldSpacePoints.resize(boardImgSpacePoints.size(), boardWorldSpacePoints[0]);

	vector<Mat> rVectors, tVectors; 
	distortionCoeffs = Mat::zeros(8, 1, CV_64F);

	calibrateCamera(boardWorldSpacePoints, boardImgSpacePoints, boardSize, cameraMatrix, distortionCoeffs, rVectors, tVectors);

}

bool saveCalibration(string name, Mat cameraMatrix, Mat distortionCoefficients)
{	//saves calibration parameters to txt file, order is important for loading
	ofstream outStream(name);
	if (outStream)
	{
		uint16_t rows = cameraMatrix.rows;
		uint16_t columns = cameraMatrix.cols; 

		outStream << rows << endl;
		outStream << columns << endl;

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++) {
				double value = cameraMatrix.at<double>(r, c);
				outStream << value << endl;
			}
		}

		rows = distortionCoefficients.rows;
		columns = distortionCoefficients.cols;

		outStream << rows << endl;
		outStream << columns << endl;
		
		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++) {
				double value = distortionCoefficients.at<double>(r, c);
				outStream << value << endl;
			}
		}

		outStream.close();
		return true;

	}
	return false;
}

bool loadCalibration(string name, Mat& cameraMatrix, Mat& distortionCoefficients)
{
	ifstream inStream(name);
	if (inStream)
	{
		uint16_t rows;
		uint16_t columns; 

		inStream >> rows;
		inStream >> columns;

		//load camera matrix 
		cameraMatrix = Mat(Size(columns, rows), CV_64F); 

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				cameraMatrix.at<double>(r, c) = read;
			}
		}

		//Load Distance Coefficients 
		inStream >> rows;
		inStream >> columns; 

		distortionCoefficients = Mat::zeros(rows, columns, CV_64F);
		
		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				distortionCoefficients.at<double>(r, c) = read;
			}
		}
		inStream.close();
		return true;
	}

	return false;

}

int trackPosition(const Mat& cameraMatrix, const Mat& distortionCoefficients, float markerDimensions)
{	//main function that mointors and detects markers 
	Mat frame; 
	vector<int> markerIds; 
	vector < vector<Point2f>> markerCorners, rejectedCandidates;
	aruco::DetectorParameters parameters; 

	Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50); //load correct aruco dict 
	
	VideoCapture vid(0); //first webcam

	if (!vid.isOpened())
	{
		return -1;
	}

	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);

	vector<Vec3d> rotationVectors, translationVectors; 

	while (true)
	{
		if (!vid.read(frame))
			break;
		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds); //marker corners and ids are filled up 
		aruco::estimatePoseSingleMarkers(markerCorners, markerDimension, cameraMatrix, distortionCoefficients, rotationVectors, translationVectors);

		for (int i = 0; i < markerIds.size(); i++)
		{
			aruco::drawAxis(frame, cameraMatrix, distortionCoefficients, rotationVectors[i], translationVectors[i], 0.05f);

			string xPos = to_string(translationVectors[i][0]);
			string yPos = to_string(translationVectors[i][1]);
			string zPos = to_string(translationVectors[i][2]);
			string outputCoordinates = "X: " + xPos + "\n" + "  Y: " + yPos + "\n" + "  Z: " + zPos;
			putText(frame, outputCoordinates, Point2f(30, 30), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255, 255));

		}
		imshow("Webcam", frame);
		if (waitKey(30) >= 0) break; 
	}
	return 1;

}

void calibrationHelper(Mat& cameraMatrix, Mat& distortionCoefficients)
{	//here we setup camera calibration and use previous functions to help 
	//the function looks at an image frame and draws on the corners if found, when found press space bar to save
	//eventually with enough saved frames we can press enter to begin calibration using the above cameraCalibration function 
	int framesPerSecond = 30;
	Mat frame;
	Mat drawToFrame;
	vector<Mat> savedImages; 
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	string textOutput = "saved"; 
	VideoCapture vid(0);

	if (!vid.isOpened())
	{
		return; //breaks if no video stream
	}

	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);

	while (true)
	{
		if (!vid.read(frame)) //break if next frame not read
			break;

		vector<Vec2f> foundPoints;
		bool found = false;
		found = findChessboardCorners(frame, chessboardSize, foundPoints, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		frame.copyTo(drawToFrame);
		drawChessboardCorners(drawToFrame, chessboardSize, foundPoints, found);
		if (found)
			imshow("Webcam", drawToFrame);
		else
			imshow("Webcam", frame);
		char character = waitKey(1000 / framesPerSecond);

		switch (character)
		{
		case ' ':
			//save image for calibration by pressing space bar when corners found 
			if (found) {
				Mat temp;
				frame.copyTo(temp);
				savedImages.push_back(temp);
				cout << textOutput << "\n";
			}
			break;
		case 13:
			//start calibrating by pressing enter 
			if (savedImages.size() > 10) //only calibrates if more than 10 images are saved (aim for >30) 
			{
				cameraCalibration(savedImages, chessboardSize, chessSquareDimension, cameraMatrix, distortionCoefficients);
				saveCalibration("CalibrationParams", cameraMatrix, distortionCoefficients);
				return;
			}
			break;
		case 27:
			//exit calibration by hitting escape 
			return;
			break;
		}
	}
}

int main(int argv, char** args)
{	//the program first calibrates the camera, loads calibration, and begins positional tracking 
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distortionCoefficients;
	string ans; 
	cout << "Would you like to perform calibration? (Y/N)";
	cin >> ans;

	if (ans == "Y")
	{
		calibrationHelper(cameraMatrix, distortionCoefficients); //calibrates camera
	}

	loadCalibration("CalibrationParams", cameraMatrix, distortionCoefficients);
	trackPosition(cameraMatrix, distortionCoefficients, markerDimension);
	
}
