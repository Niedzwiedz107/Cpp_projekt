#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgproc.hpp"
#include <opencv2/videoio.hpp>

#include <iostream>

#include "SudokuFinder.h"
#include "Sudoku.h"

using namespace cv;
using namespace std;


int main()
{
	VideoCapture capture;
	Mat frame;
	Mat resImg;
	vector<Point> sudokuContour;
	SudokuFinder mySudokuFinder = SudokuFinder();
	Sudoku foundSudoku;
	Sudoku solvedSudoku;
	capture.open(0);
	if (capture.isOpened())
	{
		while (1)
		{
			capture >> frame;
			if (frame.empty()) break;
			sudokuContour = mySudokuFinder.findSudokuContour(frame);
			if (!sudokuContour.empty()) drawContours(frame, vector<vector<Point>>(1, sudokuContour), 0, Scalar(0, 255, 0), 2);

			imshow("CameraView", frame);

			char key = waitKey(1); // press q to quit CameraView
			if (key == 's')
			{
				cout << "Pressed s" << endl;
				foundSudoku = mySudokuFinder.findSudokuDigits();
				solvedSudoku = Sudoku(foundSudoku);
				solvedSudoku = solvedSudoku.solve();		

				if (!solvedSudoku.empty())
				{
					cout << "Solved" << endl;
					resImg = mySudokuFinder.drawResult(foundSudoku, solvedSudoku);
					imshow("Solved Sudoku", resImg);
					waitKey(0);
					destroyAllWindows();
				}
				else
				{
					cout << "Could not solve sudoku!" << endl;
				}
			}
			else if (key == 'q')
			{
				cout << "Pressed q" << endl;
				break;
			}
		}
	}

	return 0;
}