#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include "Sudoku.h"

using namespace cv;
using namespace std;

class SudokuFinder
{
private:
	Mat frame;
	vector<Point> sudokuContour;
	Mat M;
	Mat warpedSudoku;
public:
	vector<Point> findSudokuContour(Mat frame);
	void makeWarpedSudoku();
	Sudoku findSudokuDigits();
	void prepareDigitImg(Mat& digitImg) const;
	int recognizeDigit(Mat digitImg) const;
	Mat drawResult(const Sudoku& foundSudoku,const Sudoku& resSudoku);

};

bool compareContourAreasDecreasing(vector<Point> contour1, vector<Point> contour2);
bool compareSum(Point p1, Point p2);
bool compareDiff(Point p1, Point p2);