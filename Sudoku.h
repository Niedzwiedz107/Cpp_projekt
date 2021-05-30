#pragma once
#include <opencv2/core.hpp>
#include <set>
#include <iostream>
using namespace std;

class Sudoku
{
private:
	vector<vector<int>> matrix;
	vector<set<int>> rows;
	vector<set<int>> cols;
	vector<set<int>> boxes;
	vector<cv::Point> emptyCells;

public:
	Sudoku();
	Sudoku(const Sudoku& sudoku);
	void set(size_t i, size_t j, int val);
	vector<int> operator[](size_t i) const;
	Sudoku operator&(const Sudoku& other) const;
	bool empty() const;

	void localizeValues();
	void setCellValue(cv::Point pos, int cellValue);
	void removeCellValue(cv::Point pos);
	bool isValueValid(cv::Point pos, int cellValue);
	bool isSolvable();
	Sudoku solve();
};

inline ostream& operator<<(ostream& os, const Sudoku& sudoku)
{
	for (size_t i = 0; i < 9; i++)
	{
		for (size_t j = 0; j < 9; j++)
		{
			os << sudoku[i][j] << " ";
		}
		os << endl;
	}
	return os;
}