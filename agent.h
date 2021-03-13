#ifndef AGENT_H
#define AGENT_H
#include "definitions.h"
#include <vector>

enum TYPE { Wall,  Floor, Unknown};
enum DIRECTION {N,E,S,W};
//enum Status{Unknown, Blocked, };

struct Tile
{
	int PosX = 0;
	int PosY = 0;
	TYPE type = Floor;

	//std::vector<Tile*> neighbors;
};

class Agent {
  public:
    Agent(int random_seed);
    Action GetAction(Percept p);

	std::vector<Tile>Map;
	Tile Home;

	int currX = 0;
	int currY = 0;

	std::pair<int, int> currRight	= std::make_pair(currX + 1, currY);
	std::pair<int, int> currLeft	= std::make_pair(currX - 1, currY);
	std::pair<int, int> currFront	= std::make_pair(currX, currY + 1);
	std::pair<int, int> currBack	= std::make_pair(currX, currY - 1);
	
	int direction = N;

	bool ShouldTurnAround = false;
	bool ShouldGoHome = false;

	bool ShouldGoAhead = false;
	
	bool IsBlocked(int x, int y);
	TYPE GetType(int x, int y);

	bool IsVisited(int x, int y);

	void TurnRight();
	void TurnLeft();
	void TurnAround();
	void MoveForward();

	void AddToMap(int x, int y, TYPE type);
	
	void DebugPrint();
	
	void AdjustPositionAfterBump();
};

#endif

