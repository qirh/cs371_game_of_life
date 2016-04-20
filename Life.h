#ifndef Life_h
#define Life_h

#include <vector>
#include <iostream>

#include "gtest/gtest.h"

class Cell;

class AbstractCell {
	friend std::ostream& operator<<(std::ostream& out, const AbstractCell& c);
	friend std::istream& operator>>(std::istream& in, AbstractCell& c);

protected:
	bool alive;
	bool border;
public:
	/*	Order for neighbors in the vector
	  									8 1 5
	  									4 X 2
	  									7 3 6
	*/
	virtual Cell evolve(std::vector<Cell> neighbors) const = 0;
	virtual std::ostream& print(std::ostream& out) const = 0;
	virtual AbstractCell* clone() const = 0;

	virtual ~AbstractCell() {}

	AbstractCell(bool border_ = false) : border(border_) {}

	virtual const bool is_alive() const { return alive; }
	virtual const bool is_border() const { return border; }
};

class ConwayCell : public AbstractCell {
	friend ConwayCell operator+(ConwayCell old_cell, const std::vector<ConwayCell> neighbors);

public:
	ConwayCell(bool border_ = false) : AbstractCell(border_) {}
	ConwayCell(const char& input);
	std::ostream& print(std::ostream& out) const;
	Cell evolve(std::vector<Cell> neighbors) const;

	ConwayCell* clone() const;
};

class FredkinCell : public AbstractCell {
	friend FredkinCell operator+(FredkinCell old_cell, const std::vector<FredkinCell> neighbors);

public:
	FredkinCell(bool border_ = false) : AbstractCell(border_) {}
	FredkinCell(const char& input);
	FredkinCell(int age_, bool alive_);
	std::ostream& print(std::ostream& out) const;
	Cell evolve(std::vector<Cell> neighbors) const;

	FredkinCell* clone() const;

	const int age() const;

protected:
	int age_;
	FRIEND_TEST(FredkinFixture, fredkin_construct1);
	FRIEND_TEST(FredkinFixture, fredkin_construct2);
	FRIEND_TEST(FredkinFixture, fredkin_construct3);
	FRIEND_TEST(FredkinFixture, fredkin_construct4);
	FRIEND_TEST(FredkinFixture, fredkin_construct5);
};

class Cell {
	friend Cell operator+(Cell old_cell, const std::vector<Cell> neigbors);
	friend std::ostream& operator<<(std::ostream& out, const Cell c);
	friend std::istream& operator>>(std::istream& in, Cell& c);

public:
	AbstractCell *acell;

	Cell(AbstractCell* c = nullptr) : acell(c) {}
	Cell(const AbstractCell& c) : acell(c.clone()) {}
	Cell(bool border_) : acell(new ConwayCell(border_)) {}
	Cell(const char& c);
	Cell(const Cell& c);
	~Cell();
	Cell& operator=(const Cell& rhs);

	bool is_alive() const;
};

template <class T>
class Life {
public:
	Life(std::istream& in) {
		bool last_char_was_newline = true; // Used to determine where the input ends and where to add borders

		width = 0;
		height = 0;
		generation = 0;
		population = 0;

		std::vector<T> temp_board; // Using this because we need to add the top border after we're done

		while (true) {
			int input = in.get();

			if (input == '\n' || input == EOF) {
				if (last_char_was_newline)
					break;
				else {
					++height;
					last_char_was_newline = true;

					// Add border on right side
					temp_board.push_back(T(true));

					continue;
				}
			}

			if (height < 1)
				++width;

			if (last_char_was_newline == true) { // Add border on left side
				temp_board.push_back(T(true));
			}

			// Add the actual cell
			T new_cell = T((char) input);

			if (new_cell.is_alive())
				population++;

			temp_board.push_back(new_cell);

			last_char_was_newline = false;
		}

		// Add border for last row
		for (int i = 0; i < width + 2; i++)
			temp_board.push_back(T(true));

		// Add border for first row
		for (int i = 0; i < width + 2; i++)
			board.push_back(T(true));

		// Put the rest of the board after the top border
		board.insert(board.end(), temp_board.begin(), temp_board.end());
	}

	void print(std::ostream& out) {
		out << "Generation = " << generation << ", Population = " << population << "." << std::endl;
		for (int x = 1; x < height + 1; x++) {
			for (int y = 1; y < width + 1; y++) {
				T c = board[x * (width + 2) + y];

				out << c;
			}
			out << std::endl;
		}
		out << std::endl;
	}

	void evolve_all() {
		population = 0;

		std::vector<T> temp_board = board;

		for (int x = 0; x < height; x++) {
			for (int y = 0; y < width; y++) {
				T cell = at(x, y);
				std::vector<T> neighbors;

				neighbors.push_back(temp_board.at((x + 1) * (width + 2) + y));		// up
				neighbors.push_back(temp_board.at((x + 2) * (width + 2) + y + 1));	// right
				neighbors.push_back(temp_board.at((x + 1) * (width + 2) + y + 2));	// down
				neighbors.push_back(temp_board.at((x) * (width + 2) + y + 1));		// left
				neighbors.push_back(temp_board.at((x + 2) * (width + 2) + y));		// top-right
				neighbors.push_back(temp_board.at((x + 2) * (width + 2) + y + 2));	// bottom-right
				neighbors.push_back(temp_board.at((x) * (width + 2) + y + 2));		// bottom-left
				neighbors.push_back(temp_board.at((x) * (width + 2) + y));			// top-left

				T new_cell = cell + neighbors;
				
				if (new_cell.is_alive())
					population++;
				
				at(x, y).~T();		// deallocate old cell to avoid memory leaks
				at(x, y) = new_cell;
			}
		}

		generation++;
	}

	T& at(int x, int y) {
		return board.at((x + 1) * (width + 2) + y + 1);
	}
	const T& at(int x, int y) const {
		return board.at((x + 1) * (width + 2) + y + 1);
	}

	T* begin();
	T* end();
private:
	int height;
	int width;
	std::vector<T> board;

	int generation;
	int population;

	FRIEND_TEST(LifeFixture, life_construct1);
	FRIEND_TEST(LifeFixture, life_construct2);
	FRIEND_TEST(LifeFixture, life_construct3);
};

#endif