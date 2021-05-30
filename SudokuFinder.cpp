#include "SudokuFinder.h"

bool compareContourAreasDecreasing(vector<Point> contour1, vector<Point> contour2)
{
	return fabs(contourArea(contour1)) > fabs(contourArea(contour2));
}

bool compareSum(Point p1, Point p2)
{
	int s1 = p1.x + p1.y;
	int s2 = p2.x + p2.y;
	return s1 < s2;
}

bool compareDiff(Point p1, Point p2)
{
	int d1 = p1.y - p1.x;
	int d2 = p2.y - p2.x;
	return d1 < d2;
}


vector<Point> SudokuFinder::findSudokuContour(Mat frame)
{
	// Clone frame for further processing
	this->frame = frame.clone();

	// Image processing
	Mat gray, blur, threshold;
	cvtColor(frame, gray, COLOR_BGR2GRAY);
	GaussianBlur(gray, blur, Size(3, 3), 0);
	adaptiveThreshold(blur, threshold, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 11, 2);

	// Finding contours
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(threshold, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	sort(contours.begin(), contours.end(), compareContourAreasDecreasing);

	// Choose the biggest one
	int maxArea = frame.rows * frame.cols;

	double approxLength;
	vector<Point> approxPoly;
	for (auto const& contour : contours)
	{
		if (fabs(contourArea(contour)) < 0.9 * maxArea)
		{
			double approxLength = 0.1 * arcLength(Mat(contour), true);
			approxPolyDP(contour, approxPoly, approxLength, true);
			if (approxPoly.size() == 4)
			{
				this->sudokuContour.assign(approxPoly.begin(), approxPoly.end());
				break;
			}
		}
	}
	return this->sudokuContour;
}

void SudokuFinder::makeWarpedSudoku()
{
	if (this->sudokuContour.empty()) return;

	// Order points to maintain 4 points perspective transform
	Point2f tl, tr, br, bl;
	sort(this->sudokuContour.begin(), this->sudokuContour.end(), compareSum);
	tl = this->sudokuContour[0];
	br = this->sudokuContour[3];

	sort(this->sudokuContour.begin(), this->sudokuContour.end(), compareDiff);
	tr = this->sudokuContour[0];
	bl = this->sudokuContour[3];

	Point2f src[4];
	src[0] = tl;
	src[1] = tr;
	src[2] = bl;
	src[3] = br;

	// Size of the transformed image
	int width, height, size;
	double w1 = sqrt(pow(bl.x - br.x, 2) + pow(bl.y - br.y, 2));
	double w2 = sqrt(pow(tl.x - tr.x, 2) + pow(tr.y - tl.y, 2));
	width = max((int)w1, (int)w2);

	double h1 = sqrt(pow(tr.x - br.x, 2) + pow(tr.y - br.y, 2));
	double h2 = sqrt(pow(tl.x - bl.x, 2) + pow(tl.y - bl.y, 2));
	height = max((int)h1, (int)h2);

	size = max(width, height);

	if (size < 10) return;

	// Points clockwise order
	Point2f dest[4];
	dest[0] = Point2f(0, 0);
	dest[1] = Point2f(size, 0);
	dest[2] = Point2f(0, size);
	dest[3] = Point2f(size, size);

	this->M = getPerspectiveTransform(src, dest);
	warpPerspective(this->frame, this->warpedSudoku, this->M, Size(size, size));
}

Sudoku SudokuFinder::findSudokuDigits()
{
	Sudoku retSudoku = Sudoku();
	if (this->sudokuContour.empty()) return retSudoku;

	this->makeWarpedSudoku();
	if (this->warpedSudoku.empty()) return retSudoku;

	Mat sudokuImg;
	resize(this->warpedSudoku, sudokuImg, Size(600, 600), 0, 0, INTER_LINEAR);
	cvtColor(sudokuImg, sudokuImg, COLOR_BGR2GRAY);
	GaussianBlur(sudokuImg, sudokuImg, Size(11, 11), 0);
	adaptiveThreshold(sudokuImg, sudokuImg, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 11, 2);

	int fieldSize = 600 / 9;
	
	Mat removeLinesImg, horizontalLinesImg, verticalLinesImg;
	// Connect lines
	removeLinesImg = sudokuImg.clone();
	Mat kernelRepair = getStructuringElement(MORPH_RECT, Size(5, 5));
	morphologyEx(removeLinesImg, removeLinesImg, MORPH_CLOSE, kernelRepair, Point(-1,-1), 2);
	dilate(removeLinesImg, removeLinesImg, kernelRepair, Point(-1, -1), 2);

	// Remove horizontal lines
	Mat kernelHorizontal = getStructuringElement(MORPH_RECT, Size(fieldSize / 2, 1));
	morphologyEx(removeLinesImg, horizontalLinesImg, MORPH_OPEN, kernelHorizontal, Point(-1, -1), 2);
	bitwise_not(horizontalLinesImg, horizontalLinesImg);
	bitwise_and(sudokuImg, horizontalLinesImg, sudokuImg);

	// Remove vertical lines
	Mat kernelVertical = getStructuringElement(MORPH_RECT, Size(1, fieldSize / 2));
	morphologyEx(removeLinesImg, verticalLinesImg, MORPH_OPEN, kernelVertical, Point(-1, -1), 2);
	bitwise_not(verticalLinesImg, verticalLinesImg);
	bitwise_and(sudokuImg, verticalLinesImg, sudokuImg);

	// Open and close morph operations
	Mat kernelDilate = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(sudokuImg, sudokuImg, MORPH_CLOSE, kernelDilate, Point(-1, -1));
	Mat kernelErode = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(sudokuImg, sudokuImg, MORPH_OPEN, kernelErode, Point(-1, -1));

	bitwise_not(sudokuImg, sudokuImg);
	GaussianBlur(sudokuImg, sudokuImg, Size(5, 5), 0);
	

	//imshow("SudokuPreProcessed", sudokuImg);
	//waitKey(5000);
	//destroyAllWindows();

	Mat digitImg;
	int recognizedDigit;
	for (size_t i = 0; i < 9; i++)
	{
		for (size_t j = 0; j < 9; j++)
		{
			sudokuImg(Rect(j * fieldSize, i * fieldSize, fieldSize, fieldSize)).copyTo(digitImg);
			this->prepareDigitImg(digitImg);
			recognizedDigit = this->recognizeDigit(digitImg);
			retSudoku.set(i, j, recognizedDigit);
		}
	}
	cout << retSudoku;
	return retSudoku;
}

Mat SudokuFinder::drawResult(const Sudoku& foundSudoku, const Sudoku& resSudoku)
{
	int width, height;
	width = this->warpedSudoku.cols;
	height = this->warpedSudoku.rows;

	int xDt, yDt;
	xDt = width / 9;
	yDt = width / 9;

	int startX, startY;
	startX = int(0.3 * xDt);
	startY = int(0.8 * yDt);

	for (size_t i = 0; i < 9; i++)
	{
		for (size_t j = 0; j < 9 ; j++)
		{
			if (foundSudoku[i][j] == 0)
			{
				putText(this->warpedSudoku, to_string(resSudoku[i][j]), Point(startX + j * xDt, startY + i * yDt), 
					FONT_HERSHEY_COMPLEX, 0.6, Scalar(0, 0, 255), 2, false);
			}
		}
	}
	
	// Get back from warp perspective
	Mat retImg, ROI, mask, maskInv;
	warpPerspective(this->warpedSudoku, retImg, this->M, Size(this->frame.cols, this->frame.rows), WARP_INVERSE_MAP);
	
	// Mask and add result to frame
	cvtColor(retImg, mask, COLOR_BGR2GRAY);
	threshold(mask, mask, 10, 255, THRESH_BINARY);
	bitwise_not(mask, maskInv);
	bitwise_and(retImg, retImg, retImg, mask);
	frame.copyTo(ROI, maskInv);

	add(ROI, retImg, retImg);
	return retImg;
}


void SudokuFinder::prepareDigitImg(Mat& digitImg) const
{
	resize(digitImg, digitImg, Size(150, 150), 0, 0, INTER_AREA);
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(digitImg, digitImg, MORPH_OPEN, kernel);
	morphologyEx(digitImg, digitImg, MORPH_CLOSE, kernel);
	GaussianBlur(digitImg, digitImg, Size(3, 3), 0);
}

int SudokuFinder::recognizeDigit(Mat digitImg) const
{
	string digitImgFileName = string("digitImgTmp.png");
	vector<int> compression_params;
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);
	imwrite(digitImgFileName, digitImg, compression_params);
	string cmd = string("C:/Users/Norbert/AppData/Local/Programs/Tesseract-OCR/tesseract.exe digitImgTmp.png stdout -l eng --psm 10 --oem 3 --dpi 300 > recognizedTmp.txt");
	system(cmd.c_str());
	remove(digitImgFileName.c_str());

	ifstream ifs("recognizedTmp.txt");
	string ret{istreambuf_iterator<char>(ifs), istreambuf_iterator<char>() };
	ifs.close();
	remove("recognizedTmp.txt");

	for (auto c : ret)
	{
		if (c > 0 && isdigit(c))
		{
			char tmp = char(c);
			return atoi(&tmp);
		}
	}
	return 0;
}