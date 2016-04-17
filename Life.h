#ifndef Life_h
#define Life_h

#include <vector>
#include <iostream>

#include "gtest/gtest.h"

class Cell;

class AbstractCell {
public:
	/*	Order for neighbors in the vector
	  									8 1 5
	  									4 X 2
	  									7 3 6
	*/
	virtual Cell evolve(std::vector<Cell> neighbors) = 0;
	virtual void print(std::ostream& out) = 0;
	virtual AbstractCell* clone() const = 0;

	AbstractCell(bool border = false) : is_border(border) {}

protected:
	bool alive;
	bool is_border;
};

class ConwayCell : public AbstractCell {
public:
	ConwayCell(const char& input);
	void print(std::ostream& out);
	Cell evolve(std::vector<Cell> neighbors);

	ConwayCell* clone() const;

private:
	FRIEND_TEST(ConwayFixture, conway_construct1);
	FRIEND_TEST(ConwayFixture, conway_construct2);
};

class FredkinCell : public AbstractCell {
public:
	FredkinCell(const char& input);
	FredkinCell(int age_, bool alive_);
	void print(std::ostream& out);
	Cell evolve(std::vector<Cell> neighbors);

	FredkinCell* clone() const;

private:
	int age;
	FRIEND_TEST(FredkinFixture, fredkin_construct1);
	FRIEND_TEST(FredkinFixture, fredkin_construct2);
	FRIEND_TEST(FredkinFixture, fredkin_construct3);
	FRIEND_TEST(FredkinFixture, fredkin_construct4);
	FRIEND_TEST(FredkinFixture, fredkin_construct5);
};

class Cell {
public:
	AbstractCell *acell;

	Cell(AbstractCell* c) : acell(c) {}
	Cell(const Cell& c);
	~Cell();
	Cell& operator=(const Cell& rhs);
};

template <class T>
class Life {
public:
	Life(std::istream& in) {
		bool last_char_was_newline = true; // Used to determine where the input ends and where to add borders

		width = 0;
		height = 0;

		std::vector<Cell> temp_board; // Using this because we need to add the top border after we're done

		while (true) {
			int input = in.get();

			std::cerr << "input '" << (char)input << "' width " << width << " height " << height << " last_char_was_newline " << last_char_was_newline << std::endl;

			if (input == '\n' || input == EOF) {
				if (last_char_was_newline)
					break;
				else {
					++height;
					last_char_was_newline = true;

					// Add border on right side
					temp_board.push_back(Cell(new T(true)));

					continue;
				}
			}

			if (height < 1)
				++width;

			if (last_char_was_newline == true) { // Add border on left side
				temp_board.push_back(Cell(new T(true)));
			}

			T* new_cell = new T((char) input);
			temp_board.push_back(Cell(new_cell));

			last_char_was_newline = false;
		}

		// Add border for last row
		for (int i = 0; i < width + 2; i++)
			temp_board.push_back(Cell(new T(true)));

		// Add border for first row
		for (int i = 0; i < width + 2; i++)
			board.push_back(Cell(new T(true)));

		// Put the rest of the board after the top border
		board.insert(board.end(), temp_board.begin(), temp_board.end());
	}


	void print(std::ostream& out);

	void evolve_all();
	Cell& at(int x, int y);

	Cell* begin();
	Cell* end();
private:
	int height;
	int width;
	std::vector<Cell> board;

	int generation;
	int population;

	FRIEND_TEST(LifeFixture, life_construct1);
};

#endif