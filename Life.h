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
	virtual Cell evolve(const Cell neighbors[8]) const = 0;
	virtual std::ostream& print(std::ostream& out) const = 0;
	virtual AbstractCell* clone() const = 0;

	virtual ~AbstractCell() {}

	AbstractCell(bool border_ = false) : border(border_) {}

	virtual const bool is_alive() const { return alive; }
	virtual const bool is_border() const { return border; }
};

class ConwayCell : public AbstractCell {
	friend ConwayCell operator+(const ConwayCell& old_cell, const ConwayCell neighbors[8]);

public:
	ConwayCell(bool border_ = false) : AbstractCell(border_) {}
	ConwayCell(const char& input);
	std::ostream& print(std::ostream& out) const;
	Cell evolve(const Cell neighbors[8]) const;

	ConwayCell* clone() const;
};

class FredkinCell : public AbstractCell {
	friend FredkinCell operator+(const FredkinCell& old_cell, const FredkinCell neighbors[8]);

public:
	FredkinCell(bool border_ = false) : AbstractCell(border_) {}
	FredkinCell(const char& input);
	FredkinCell(int age_, bool alive_);
	std::ostream& print(std::ostream& out) const;
	Cell evolve(const Cell neighbors[8]) const;

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
	friend Cell operator+(const Cell& old_cell, const Cell neigbors[8]);
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
	bool is_border() const;
};

template <class T>
class Life {
public:
	Life(std::istream& in, int h, int w) {
		width = w;
		height = h;
		generation = 0;
		population = 0;

		int x = 0;
		int y = 0;

		board.resize((width + 2) * (height + 2), T(true)); // Initialize board, fill it with borders

		while (true) {
			int input = in.get();

			//std::cerr << "x " << x << " y " << y << " input " << (char)input << std::endl;

			if (input == EOF || (input == '\n' && y == 0))
				break;

			if (input == '\n') {
				x++;
				assert(y == width);
				y = 0;
				continue;
			}

			// Add the actual cell
			T new_cell = T((char) input);

			if (new_cell.is_alive())
				population++;

			at(x, y).~T();	// deallocate
			at(x, y) = new_cell;

			y++;
		}

		assert(x == height);
	}

	void print(std::ostream& out) {
		out << "Generation = " << generation << ", Population = " << population << "." << std::endl;
		for (int x = 1; x < height + 1; x++) {
			for (int y = 1; y < width + 1; y++) {
				const T cell = board[x * (width + 2) + y];

				out << cell;
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
				const T& cell = at(x, y);

				T neighbors[8] = {
					temp_board[(x + 1) * (width + 2) + y],		// up
					temp_board[(x + 2) * (width + 2) + y + 1],	// right
					temp_board[(x + 1) * (width + 2) + y + 2],	// down
					temp_board[(x) * (width + 2) + y + 1],		// left
					temp_board[(x + 2) * (width + 2) + y],		// top-right
					temp_board[(x + 2) * (width + 2) + y + 2],	// bottom-right
					temp_board[(x) * (width + 2) + y + 2],		// bottom-left
					temp_board[(x) * (width + 2) + y]};			// top-left

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

	template <class T2>
	class iterator {
	public:
		iterator(Life& l, int x_, int y_) : life(l), x(x_), y(y_) {}

		T2& operator*() const {
			return life.at(x, y);
		}
		iterator<T2>& operator++() {
			y++;
			if (y >= life.width) {
				x++;
				y = 0;
			}
			return *this;
		}
		iterator<T2>& operator--() {
			y--;
			if (y < 0) {
				x--;
				y = life.width - 1;
			}
			return *this;
		}
		bool operator==(const iterator<T2>& rhs) const {
			return x == rhs.x && y == rhs.y && &life == &rhs.life;
		}
		bool operator!=(const iterator<T2>& rhs) const {
			return !(*this == rhs);
		}
	private:
		Life<T2>& life;
		int x;
		int y;
	};

	iterator<T> begin() {
		return iterator<T>(*this, 0, 0);
	}
	iterator<T> end() {
		return iterator<T>(*this, height - 1, width);
	}
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