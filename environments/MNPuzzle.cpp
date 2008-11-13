/*
 *  MNPuzzle.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 5/9/07.
 *  Copyright 2007 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 */

#include "MNPuzzle.h"

MNPuzzle::MNPuzzle(unsigned int _width, unsigned int _height)
		: width(_width), height(_height)
{

	// stores applicable operators at each of the width*height positions
	// The order of operators is Right, Left, Down, Up
	std::vector<slideDir> ops(4);
	for (unsigned int blank = 0; blank < width*height; blank++)
	{
		ops.resize(0);

		if (blank % width < width - 1)
		{
			ops.push_back(kRight);
		}
		if (blank % width > 0)
		{
			ops.push_back(kLeft);
		}
		if (blank < width*height - width)
		{
			ops.push_back(kDown);
		}
		if (blank > width - 1)
		{
			ops.push_back(kUp);
		}

		operators.push_back(ops);
	}
}

MNPuzzle::MNPuzzle(unsigned int _width, unsigned int _height,
                   std::vector<slideDir> &op_order) :
		width(_width), height(_height)
{
	bool up_act = false;
	bool down_act = false;
	bool left_act = false;
	bool right_act = false;

	assert(op_order.size() == 4);

	for (unsigned int op_num = 0; op_num < 4; op_num++)
	{
		if (op_order[op_num] == kUp)
		{
			up_act = true;
		}
		else if (op_order[op_num] == kDown)
		{
			down_act = true;
		}
		else if (op_order[op_num] == kLeft)
		{
			left_act = true;
		}
		else if (op_order[op_num] == kRight)
		{
			right_act = true;
		}
	}

	assert(up_act && down_act && left_act && right_act);

	// stores applicable operators at each of the width*height positions
	std::vector<slideDir> ops(4);
	for (unsigned int blank = 0; blank < width*height; blank++)
	{
		ops.resize(0);
		for (unsigned int op_num = 0; op_num < 4; op_num++)
		{
			if (op_order[op_num] == kUp && blank > width - 1)
			{
				ops.push_back(kUp);
			}
			if (op_order[op_num] == kLeft && blank % width > 0)
			{
				ops.push_back(kLeft);
			}
			if (op_order[op_num] == kRight && blank % width < width - 1)
			{
				ops.push_back(kRight);
			}
			if (op_order[op_num] == kDown && blank < width*height - width)
			{
				ops.push_back(kDown);
			}
		}

		operators.push_back(ops);
	}
}

MNPuzzle::~MNPuzzle()
{
}

void MNPuzzle::StoreGoal(MNPuzzleState &s)
{
	assert(s.height == height);
	assert(s.width == width);
	goal_xloc.resize(width*height);
	goal_yloc.resize(width*height);
	for (unsigned int x = 0; x < width; x++)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			goal_xloc[s.puzzle[x + y*width]] = x;
			goal_yloc[s.puzzle[x + y*width]] = y;
		}
	}
}

void MNPuzzle::ClearGoal()
{
	goal_xloc.clear();
	goal_yloc.clear();
}
void MNPuzzle::GetSuccessors(MNPuzzleState &stateID,
                             std::vector<MNPuzzleState> &neighbors)
{
	neighbors.resize(0);

	for (unsigned int i = 0; i < operators[stateID.blank].size(); i++)
	{
		neighbors.push_back(stateID);
		ApplyAction(neighbors.back(), operators[stateID.blank][i]);
	}
}

void MNPuzzle::GetActions(MNPuzzleState &stateID, std::vector<slideDir> &actions)
{
	actions.resize(0);
	for (unsigned int i = 0; i < operators[stateID.blank].size(); i++)
	{
		actions.push_back(operators[stateID.blank][i]);
	}
}

slideDir MNPuzzle::GetAction(MNPuzzleState &s1, MNPuzzleState &s2)
{
	return kUp;
}

void MNPuzzle::ApplyAction(MNPuzzleState &s, slideDir a)
{
	switch (a)
	{
		case kUp:
			if (s.blank >= s.width)
			{
				s.puzzle[s.blank] = s.puzzle[s.blank-s.width];
				s.blank -= s.width;
				s.puzzle[s.blank] = 0;
			}
			break;
		case kDown:
			if (s.blank < s.puzzle.size() - s.width)
			{
				s.puzzle[s.blank] = s.puzzle[s.blank+s.width];
				s.blank += s.width;
				s.puzzle[s.blank] = 0;
			}
			break;
		case kRight:
			if ((s.blank%s.width) < s.width-1)
			{
				s.puzzle[s.blank] = s.puzzle[s.blank+1];
				s.blank += 1;
				s.puzzle[s.blank] = 0;
			}
			break;
		case kLeft:
			if ((s.blank%s.width) > 0)
			{
				s.puzzle[s.blank] = s.puzzle[s.blank-1];
				s.blank -= 1;
				s.puzzle[s.blank] = 0;
			}
			break;
	}
}

bool MNPuzzle::InvertAction(slideDir &a)
{
	switch (a)
	{
		case kLeft: a = kRight; break;
		case kUp: a = kDown; break;
		case kDown: a = kUp; break;
		case kRight: a = kLeft; break;
	}
	return true;
}

double MNPuzzle::HCost(MNPuzzleState &state1, MNPuzzleState &state2)
{
	assert(state1.height==state2.height);
	assert(state1.width==state2.width);
	double hval = 0;

	if (goal_xloc.size() != 0)
	{
		assert(state1.height == height);
		assert(state1.width == width);

		for (unsigned int x = 0; x < width; x++)
		{
			for (unsigned int y = 0; y < height; y++)
			{
				if (state1.puzzle[x + y*width] != 0)
				{
					hval += (abs(goal_xloc[state1.puzzle[x + y*width]] - x) +
					         abs(goal_yloc[state1.puzzle[x + y*width]] - y));
				}
			}
		}
	}
	else
	{
		std::vector<int> xloc(state2.width*state2.height);
		std::vector<int> yloc(state2.width*state2.height);

		for (unsigned int x = 0; x < state2.width; x++)
		{
			for (unsigned int y = 0; y < state2.height; y++)
			{
				xloc[state2.puzzle[x + y*state2.width]] = x;
				yloc[state2.puzzle[x + y*state2.width]] = y;
			}
		}
		for (unsigned int x = 0; x < state1.width; x++)
		{
			for (unsigned int y = 0; y < state1.height; y++)
			{
				if (state1.puzzle[x + y*state1.width] != 0)
				{
					hval += (abs(xloc[state1.puzzle[x + y*state1.width]] - x) +
					         abs(yloc[state1.puzzle[x + y*state1.width]] - y));
				}
			}
		}
	}
	if (PDB.size() != 0)
		return std::max(hval, DoPDBLookup(state1));
	return hval;
}

double MNPuzzle::GCost(MNPuzzleState &, MNPuzzleState &)
{
	return 1;
}

bool MNPuzzle::GoalTest(MNPuzzleState &state, MNPuzzleState &goal)
{
	return (state == goal);
}

uint64_t MNPuzzle::GetStateHash(MNPuzzleState &state)
{
	std::vector<int> puzzle = state.puzzle;
	uint64_t hashVal = 0;
	int numEntriesLeft = state.puzzle.size();
	for (unsigned int x = 0; x < state.puzzle.size(); x++)
	{
		hashVal += puzzle[x]*Factorial(numEntriesLeft-1);
		numEntriesLeft--;
		for (unsigned y = x; y < puzzle.size(); y++)
		{
			if (puzzle[y] > puzzle[x])
				puzzle[y]--;
		}
	}
	return hashVal;
}

/**
 * Tiles are the tiles we care about
 */
uint64_t MNPuzzle::GetPDBHash(MNPuzzleState &state, const std::vector<int> &tiles)
{
	std::vector<int> locs;
	locs.resize(tiles.size());
	for (unsigned int x = 0; x < state.puzzle.size(); x++)
	{
		for (unsigned int y = 0; y < tiles.size(); y++)
		{
			if (state.puzzle[x] == tiles[y])
				locs[y] = x;
		}
	}

	uint64_t hashVal = 0;
	int numEntriesLeft = state.puzzle.size();
	for (unsigned int x = 0; x < locs.size(); x++)
	{
		hashVal += locs[x]*Factorial(numEntriesLeft-1)/Factorial(state.puzzle.size()-tiles.size());
		numEntriesLeft--;
		for (unsigned y = x; y < locs.size(); y++)
		{
			if (locs[y] > locs[x])
				locs[y]--;
		}
	}
	assert(hashVal*Factorial(state.puzzle.size()-tiles.size()) < Factorial(state.puzzle.size()));
	return hashVal;
}


uint64_t MNPuzzle::GetActionHash(slideDir act)
{
	switch (act)
	{
		case kUp: return 0;
		case kDown: return 1;
		case kRight: return 2;
		case kLeft: return 3;
	}
	return 4;
}

void MNPuzzle::OpenGLDraw(int window)
{
}


void MNPuzzle::OpenGLDraw(int window, MNPuzzleState &s)
{
	glLineWidth(1.0);
	glEnable(GL_LINE_SMOOTH);

	float w = width;
	float h = height;

	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			glPushMatrix();
			glColor3f(0.0, 1.0, 0.0);
			glTranslatef(x*2.0/w-1.0, (1+y)*2.0/h-1.0, -0.001);
			glScalef(1.0/(w*120.0), 1.0/(h*120.0), 1);
			glRotatef(180, 0.0, 0.0, 1.0);
			glRotatef(180, 0.0, 1.0, 0.0);
			//glTranslatef((float)x/width-0.5, (float)y/height-0.5, 0);
			if (s.puzzle[x+y*width] > 9)
				glutStrokeCharacter(GLUT_STROKE_ROMAN, '0'+(((s.puzzle[x+y*width])/10)%10));
			if (s.puzzle[x+y*width] > 0)
				glutStrokeCharacter(GLUT_STROKE_ROMAN, '0'+((s.puzzle[x+y*width])%10));
			//glTranslatef(-x/width+0.5, -y/height+0.5, 0);
			glPopMatrix();
		}
	}

	glBegin(GL_LINES);
	for (unsigned int y = 0; y <= height; y++)
	{
		for (unsigned int x = 0; x <= width; x++)
		{
			glVertex3f(x*2.0/w-1.0, -1, -0.001);
			glVertex3f(x*2.0/w-1.0, 1, -0.001);
			glVertex3f(-1, (y)*2.0/h-1.0, -0.001);
			glVertex3f(1, (y)*2.0/h-1.0, -0.001);
		}
	}
	glEnd();

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);
	//glEnable(GL_LINE_SMOOTH);
	//output(200, 225, "This is antialiased.");

	//int width, height;
	//std::vector<int> puzzle;
}

uint64_t MNPuzzle::Factorial(int val)
{
	static uint64_t table[21] =
	{ 1ll, 1ll, 2ll, 6ll, 24ll, 120ll, 720ll, 5040ll, 40320ll, 362880ll, 3628800ll, 39916800ll, 479001600ll,
	6227020800ll, 87178291200ll, 1307674368000ll, 20922789888000ll, 355687428096000ll,
	6402373705728000ll, 121645100408832000ll, 2432902008176640000ll };
	if (val > 20)
		return (uint64_t)-1;
	return table[val];
}

void MNPuzzle::LoadPDB(char *fname, const std::vector<int> &tiles, bool )
{
	std::vector<int> values(256);
	uint64_t COUNT = Factorial(width*height)/Factorial(width*height-tiles.size());
	PDB.resize(PDB.size()+1);
	PDB.back().resize(COUNT);
	FILE *f = fopen(fname, "r");
	if (f)
	{
		fread(&(PDB.back()[0]), sizeof(uint8_t), COUNT, f);
		fclose(f);
	}
	PDBkey.push_back(tiles);
	for (unsigned int x = 0; x < COUNT; x++)
	{
		values[(PDB.back()[x])]++;
	}
	for (int x = 0; x < 256; x++)
		printf("%d:\t%d\n", x, values[x]);
}

double MNPuzzle::DoPDBLookup(MNPuzzleState &state)
{
	double val = 0;
	for (unsigned int x = 0; x < PDB.size(); x++)
	{
		uint64_t index = GetPDBHash(state, PDBkey[x]);
		val = std::max(val, (double)PDB[x][index]);
		//val += (double)PDB[x][index];
	}
	if (width == height) // symmetry
	{
	}
	return val;
}

/**
Reads in MNPuzzle states from the given filename. Each line of the
input file contains a puzzle state of the given width and height.
The state is written in the form of listing the tile in each puzzle
position from 1 to width*height. The blank is represented by the 0
character.

If any line is found to not be legitimate, either because it does
not have the proper number of tiles, or the proper tiles, it is
simply not added to the list of puzzles.

The function will return 1 if reading from the given filename failed,
and 0 otherwise.

puzz_num_start - should be set as true if the first non-whitespace
element on each line is the puzzle number, otherwise should be set
to false.

max_puzzles - the maximum number of puzzles that will be added to
the puzzle list.
**/
int MNPuzzle::read_in_mn_puzzles(const char *filename, bool puzz_num_start, unsigned int width, unsigned int height, unsigned int max_puzzles, std::vector<MNPuzzleState> &puzzles) {

	std::ifstream ifs(filename, std::ios::in);

	if(ifs.fail()) {
		return 1;
	}

	std::string s, temp;

	std::vector<unsigned int> puzz_ints;
	std::vector<bool> tiles_in_puzzle(width*height);
	bool first = true;
	unsigned puzz_count = 0;

	while(!ifs.eof() && puzz_count < max_puzzles) {
		puzz_ints.clear();

		for(unsigned i = 0; i < tiles_in_puzzle.size(); i++) {
			tiles_in_puzzle[i] = false;
		}

		getline(ifs, s);
		for(unsigned int i = 0; i < s.length(); i++) {
			if(s.at(i) == ' ' || s.at(i) == '\t') {
				if(temp.length() > 0) {
					if(puzz_num_start && first) {
						temp = "";
						first = false;
					}
					else {
						puzz_ints.push_back(atoi(temp.c_str()));
						temp = "";
					}
				}
			}
			else {
				temp.push_back(s.at(i));
			}
		}

		if(temp.length() > 0) {
			puzz_ints.push_back(atoi(temp.c_str()));
			temp = "";
		}

		if(puzz_ints.size() > 0 && puzz_ints.size() == width*height) {
			MNPuzzleState new_state(width, height);
			for(unsigned int i = 0; i < puzz_ints.size(); i++) {
				new_state.puzzle[i] = puzz_ints[i];
				tiles_in_puzzle[puzz_ints[i]] = true;
				if(new_state.puzzle[i] == 0) {
					new_state.blank = i;
				}
			}

			bool is_good = true;
			for(unsigned int i = 0; i < tiles_in_puzzle.size(); i++) {
				if(tiles_in_puzzle[i] = false) {
					is_good = false;
					break;
				}
			}

			if(is_good) {
				puzz_count++;
				puzzles.push_back(new_state);
			}

		}
	}

	ifs.close();

	return 0;
}
