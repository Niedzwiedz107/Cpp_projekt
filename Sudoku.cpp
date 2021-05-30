#include "Sudoku.h"

Sudoku::Sudoku()
{
	for (size_t i = 0; i < 9; i++)
	{
		this->matrix.push_back(vector<int>(9, 0)); // length, initialValue
		this->rows.push_back(std::set<int>{});
		this->cols.push_back(std::set<int>{});
		this->boxes.push_back(std::set<int>{});
	}
	this->emptyCells = vector<cv::Point>();
}

Sudoku::Sudoku(const Sudoku& sudoku)
{
	for (size_t i = 0; i < 9; i++)
	{
		this->matrix.push_back(vector<int>(sudoku.matrix[i]));
		this->rows.push_back(std::set<int>(sudoku.rows[i]));
		this->cols.push_back(std::set<int>(sudoku.cols[i]));
		this->boxes.push_back(std::set<int>(sudoku.boxes[i]));
	}
	this->emptyCells = vector<cv::Point>();
}

void Sudoku::set(size_t i, size_t j, int val)
{
	this->matrix[i][j] = val;
}

vector<int> Sudoku::operator[](size_t i) const
{
	return this->matrix[i];
}

Sudoku Sudoku::operator&(const Sudoku& other) const
{
	Sudoku newSudoku = Sudoku();
	for (size_t i = 0; i < 9; i++)
	{
		for (size_t j = 0; j < 9; j++)
		{
			if (this->matrix[i][j] == other[i][j])
			{
				newSudoku.set(i, j, 0);
			}
			else
			{
				newSudoku.set(i, j, other[i][j]);
			}
		}
	}
	return newSudoku;
}

bool Sudoku::empty() const
{
	for (size_t i = 0; i < 9; i++)
	{
		for (size_t j = 0; j < 9; j++)
		{
			if (this->matrix[i][j] != 0)
			{
				return false;
			}
		}
	}
	return true;
}

void Sudoku::localizeValues()
{
	for (size_t i = 0; i < 9; i++)
	{
		for (size_t j = 0; j < 9; j++)
		{
			int cellValue = this->matrix[i][j];
			if (cellValue == 0)
			{
				this->emptyCells.push_back(cv::Point(i , j));
			}
			else
			{
				this->rows[i].insert(cellValue);
				this->cols[j].insert(cellValue);
				size_t x, y;
				y = i / 3;
				x = j / 3;
				if (y == 0)
				{
					this->boxes[x].insert(cellValue);
				}
				else if (y == 1)
				{
					this->boxes[3 + x].insert(cellValue);
				}
				else
				{
					this->boxes[6 + x].insert(cellValue);
				}
			}
		}
	}
}

void Sudoku::setCellValue(cv::Point pos, int cellValue)
{
	size_t row, col;
	row = pos.x;
	col = pos.y;

	this->matrix[row][col] = cellValue;
	this->rows[row].insert(cellValue);
	this->cols[col].insert(cellValue);
	size_t x, y;
	x = col / 3;
	y = row / 3;
	if (y == 0)
	{
		this->boxes[x].insert(cellValue);
	}
	else if (y == 1)
	{
		this->boxes[3 + x].insert(cellValue);
	}
	else
	{
		this->boxes[6 + x].insert(cellValue);
	}
}

void Sudoku::removeCellValue(cv::Point pos)
{
	size_t row, col;
	row = pos.x;
	col = pos.y;

	int cellValue = this->matrix[row][col];
	this->matrix[row][col] = 0;
	this->rows[row].erase(cellValue);
	this->cols[col].erase(cellValue);

	size_t x, y;
	x = col / 3;
	y = row / 3;
	if (y == 0)
	{
		this->boxes[x].erase(cellValue);
	}
	else if (y == 1)
	{
		this->boxes[3 + x].erase(cellValue);
	}
	else
	{
		this->boxes[6 + x].erase(cellValue);
	}
}

bool Sudoku::isValueValid(cv::Point pos, int cellValue)
{
	size_t row, col;
	row = pos.x;
	col = pos.y;

	if (this->rows[row].find(cellValue) != this->rows[row].end() ||
		this->cols[col].find(cellValue) != this->cols[col].end())
	{
		return false;
	}

	size_t x, y;
	x = col / 3;
	y = row / 3;
	if (y == 0)
	{
		return this->boxes[x].find(cellValue) == this->boxes[x].end();
	}
	else if (y == 1)
	{
		return this->boxes[3 + x].find(cellValue) == this->boxes[3 + x].end();
	}
	else
	{
		return this->boxes[6 + x].find(cellValue) == this->boxes[6 + x].end();
	}
}

bool Sudoku::isSolvable()
{
	if (this->emptyCells.empty())
	{
		return true;
	}

	cv::Point pos;
	pos = this->emptyCells.back();
	this->emptyCells.pop_back();

	for (int val = 1; val < 10; val++)
	{
		if (this->isValueValid(pos, val))
		{
			
			this->setCellValue(pos, val);
			if (this->isSolvable())
			{
				return true;
			}
			this->removeCellValue(pos);
		}
	}
	this->emptyCells.push_back(pos);
	return false;
}

Sudoku Sudoku::solve()
{
	this->localizeValues();
	if (this->isSolvable())
	{
		return *this;
	}
	return Sudoku();
}