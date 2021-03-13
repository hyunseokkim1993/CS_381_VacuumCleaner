#include <ctime> //for random seed
#include <cmath> 
#include "agent.h"
#include <iostream>

Agent::Agent(int random_seed) { 
  //supplying your own seed may help debugging, same seed will cause 
  //same random number sequence 
  if (random_seed==0) std::srand( static_cast<unsigned>(std::time(0))); // random seed from time
  else                std::srand( random_seed ); // random seed from user

  Home.PosX = 0;
  Home.PosY = 0;
  Home.type = Floor;
  Map.push_back(Home);
} 

Action Agent::GetAction(Percept p)
{
	//If there is dirt, suck
    if (p.dirt)
    {
		DebugPrint();
        return SUCK;
    }

	if(ShouldGoHome)
	{
		if (currX == Home.PosX && currY == Home.PosY)
			return SHUTOFF;
	}
	
	//Turning right one more time for 180 turn
	if(ShouldTurnAround)
	{
		ShouldTurnAround = false;
		TurnRight();
		DebugPrint();
		return RIGHT;
	}

	//Go Forward after turning
	if (ShouldGoAhead)
	{
		ShouldGoAhead = false;
		MoveForward();
		DebugPrint();
		return FORWARD;
	}
	

	////////////////////////////////////////////////////////////////////////////////////
	/// If Collision
	////////////////////////////////////////////////////////////////////////////////////
	if (p.bump)
	{
		////////////////////////////////////////////////////////////////////////////////////
		/// Add current position to map
		////////////////////////////////////////////////////////////////////////////////////
		//Update Map
		if (!IsVisited(currX, currY))
		{
			Tile newTile;
			newTile.PosX = currX;
			newTile.PosY = currY;
			newTile.type = Wall;
			Map.push_back(newTile);
		}

		////////////////////////////////////////////////////////////////////////////////////
		/// Search neighbors
		////////////////////////////////////////////////////////////////////////////////////
		//TYPE frontType;
		TYPE leftType = Wall;
		TYPE rightType = Wall;

		AdjustPositionAfterBump();
		
		switch (direction)
		{
		case N:
			//frontType = GetType(currX, currY + 1);
			leftType = GetType(currX - 1, currY);
			rightType = GetType(currX + 1, currY);
			break;
		case E:
			//frontType = GetType(currX + 1, currY);
			leftType = GetType(currX, currY + 1);
			rightType = GetType(currX, currY - 1);
			break;
		case S:
			//frontType = GetType(currX, currY - 1);
			leftType = GetType(currX - 1, currY);
			rightType = GetType(currX + 1, currY);
			break;
		case W:
			//frontType = GetType(currX - 1, currY);
			leftType = GetType(currX, currY - 1);
			rightType = GetType(currX, currY + 1);
			break;
		default:
			break;
		}

		//Unknown
		if (leftType == Unknown && rightType == Unknown)
		{
			TurnRight();
			DebugPrint();
			return RIGHT;
		}
		
		//Blocked
		if (leftType == Wall && rightType == Wall)
		{
			TurnAround();
			DebugPrint();
			return RIGHT;
		}

		//Left Open
		if (leftType >= Floor && rightType == Wall)
		{
			TurnLeft();
			DebugPrint();
			return LEFT;
		}

		//Right Open
		if (rightType >= Floor && leftType == Wall)
		{
			TurnRight();
			DebugPrint();
			return RIGHT;
		}
	}////////////////////End of Collision}////////////////////

	
	
	////////////////////////////////////////////////////////////////////////////////////
	/// Search neighbors
	////////////////////////////////////////////////////////////////////////////////////
	TYPE frontType	= Wall;
	TYPE leftType	= Wall;
	TYPE rightType	= Wall;

	switch (direction)
	{
	case N:
		frontType	= GetType(currX, currY + 1);
		leftType	= GetType(currX - 1, currY);
		rightType	= GetType(currX + 1, currY);
		break;
	case E:
		frontType	= GetType(currX + 1, currY);
		leftType	= GetType(currX, currY + 1);
		rightType	= GetType(currX, currY - 1);
		break;
	case S:
		frontType	 = GetType(currX, currY - 1);
		leftType	 = GetType(currX - 1, currY);
		rightType	 = GetType(currX + 1, currY);
		break;
	case W:
		frontType	 = GetType(currX - 1, currY);
		leftType	 = GetType(currX, currY - 1);
		rightType	 = GetType(currX, currY + 1);
		break;
	default:
		break;
	}

	//Update Map
	if(!IsVisited(currX, currY))
	{
		Tile newTile;
		newTile.PosX = currX;
		newTile.PosY = currY;
		newTile.type = Floor;
		Map.push_back(newTile);
	}
	

	//Everything is unknown
	if(frontType == Unknown && leftType == Unknown && rightType == Unknown)
	{
		MoveForward();
		DebugPrint();
		return FORWARD;
	}

	//If path ahead is known
	if(frontType != Unknown)
	{
		//Unknown tile takes priority
		if(rightType == Unknown)
		{
			TurnRight();
			DebugPrint();
			return RIGHT;
		}
		if(leftType == Unknown)
		{
			TurnLeft();
			DebugPrint();
			return LEFT;
		}

		//Every node around is visited, GO HOME
		if(frontType != Unknown && rightType != Unknown && leftType != Unknown)
		{
			ShouldGoHome = true;

			//go to oldest node (first to come out on Map)
			for(auto& it : Map)
			{
				if(it.PosX == currFront.first && it.PosY == currFront.second)
				{
					if (it.type != Wall)
					{
						MoveForward();
						DebugPrint();
						return FORWARD;
					}
				}
				if (it.PosX == currBack.first && it.PosY == currBack.second)
				{
					if (it.type != Wall)
					{
						TurnAround();
						DebugPrint();
						return RIGHT;
					}
				}
				if (it.PosX == currLeft.first && it.PosY == currLeft.second)
				{
					if (it.type != Wall)
					{
						TurnLeft();
						DebugPrint();
						ShouldGoAhead = true;
						return LEFT;
					}
				}
				if (it.PosX == currRight.first && it.PosY == currRight.second)
				{
					if (it.type != Wall)
					{
						TurnRight();
						DebugPrint();
						ShouldGoAhead = true;
						return RIGHT;
					}
				}
			}
		}//Front/left/right = floor
		
		if (frontType == Wall)
		{
			//If both right and left are floor, go to older floor (placed earlier in map)
			if (rightType == Floor && leftType == Floor)
			{
				int nextX = -1;
				//int nextY = -1;

				for (auto& it : Map)
				{
					if (it.PosX == currRight.first && it.PosY == currRight.second)
					{
						nextX = it.PosX;
						//nextY = it.PosY;
						break;
					}
					if (it.PosX == currLeft.first && it.PosY == currLeft.second)
					{
						nextX = it.PosX;
						//nextY = it.PosY;
						break;
					}
				}

				if (nextX > currX)
				{
					TurnRight();
					DebugPrint();
					return RIGHT;
				}
				else
				{
					TurnLeft();
					DebugPrint();
					return LEFT;
				}
			}//IF BOTH RIGHT/LEFT ARE FLOOR

			//if right is open
			if (rightType == Floor && leftType == Wall)
			{
				TurnRight();
				DebugPrint();
				return RIGHT;
			}

			//if left is open
			if (rightType == Wall && leftType == Floor)
			{
				TurnLeft();
				DebugPrint();
				return LEFT;
			}
		}

		if(rightType == Wall)
		{
			bool isFrontOlder = false;

			for (auto& it : Map)
			{
				if (it.PosX == currFront.first && it.PosY == currFront.second)
				{
					if (it.type != Wall)
					{
						isFrontOlder = true;
						break;
					}
				}
				if (it.PosX == currLeft.first && it.PosY == currLeft.second)
				{
					if (it.type != Wall)
					{
						isFrontOlder = false;
						break;
					}
				}
			}

			if(isFrontOlder)
			{
				MoveForward();
				DebugPrint();
				return FORWARD;
			}
			else
			{
				TurnLeft();
				DebugPrint();
				return LEFT;
			}
		}

		if (leftType == Wall)
		{
			bool isFrontOlder = false;

			for (auto& it : Map)
			{
				if (it.PosX == currFront.first && it.PosY == currFront.second)
				{
					if(it.type != Wall)
					{
						isFrontOlder = true;
						break;
					}
				}
				if(it.PosX == currRight.first && it.PosY == currRight.second)
				{
					if (it.type != Wall)
					{
						isFrontOlder = false;
						break;
					}
				}
			}

			if (isFrontOlder)
			{
				MoveForward();
				DebugPrint();
				return FORWARD;
			}
			else
			{
				TurnRight();
				DebugPrint();
				return RIGHT;
			}
		}
	}//IF PATH AHEAD IS KNOWN


	if(frontType == Unknown)
	{
		MoveForward();
		DebugPrint();
		return FORWARD;
	}
	
	MoveForward();
	DebugPrint();
	return FORWARD;
}

TYPE Agent::GetType(int x, int y)
{
	for(auto& it : Map)
	{
		if(it.PosX == x && it.PosY == y)
		{
			return it.type;
		}
	}
	return Unknown;
}

void Agent::TurnAround()
{
	//Turn switch on
	ShouldTurnAround = true;

	//Update Neighbor reference location
	std::pair<int, int>	tempRight = currRight;
	std::pair<int, int>	tempLeft = currLeft;
	std::pair<int, int>	tempFront = currFront;
	std::pair<int, int>	tempBack = currBack;

	currFront = tempRight;
	currBack = tempLeft;
	currRight = tempBack;
	currLeft = tempFront;

	//Update direction
	
	switch (direction)
	{
	case N:
		direction = E;
		break;
	case E:
		direction = S;
		break;
	case S:
		direction = W;
		break;
	case W:
		direction = N;
		break;
	default:
		break;
	}
}
void Agent::TurnLeft()
{
	//Update Neighbor reference location
	std::pair<int, int>	tempRight	= currRight;
	std::pair<int, int>	tempLeft	= currLeft;
	std::pair<int, int>	tempFront	= currFront;
	std::pair<int, int>	tempBack	= currBack;

	currFront	= tempLeft;
	currBack	= tempRight;
	currRight	= tempFront;
	currLeft	= tempBack;

	//Update direction
	switch (direction)
	{
	case N:
		direction = W;
		break;
	case E:
		direction = N;
		break;
	case S:
		direction = E;
		break;
	case W:
		direction = S;
		break;
	default:
		break;
	}
}
void Agent::TurnRight()
{
	//Update Neighbor reference location
	std::pair<int, int>	tempRight	= currRight;
	std::pair<int, int>	tempLeft	= currLeft;
	std::pair<int, int>	tempFront	= currFront;
	std::pair<int, int>	tempBack	= currBack;

	currFront	= tempRight;
	currBack	= tempLeft;
	currRight	= tempBack;
	currLeft	= tempFront;

	//Update direction
	switch (direction)
	{
	case N:
		direction = E;
		break;
	case E:
		direction = S;
		break;
	case S:
		direction = W;
		break;
	case W:
		direction = N;
		break;
	default:
		break;
	}
}
void Agent::MoveForward()
{
	switch (direction)
	{
	case N:
		currY += 1;
		currBack.second		+= 1;
		currFront.second	+= 1;
		currRight.second	+= 1;
		currLeft.second		+= 1;
		break;
	case E:
		currX += 1;
		currBack.first	+= 1;
		currFront.first += 1;
		currRight.first += 1;
		currLeft.first	+= 1;
		break;
	case S:
		currY -= 1;
		currBack.second		-= 1;
		currFront.second	-= 1;
		currRight.second	-= 1;
		currLeft.second		-= 1;
		break;
	case W:
		currX -= 1;
		currBack.first	-= 1;
		currFront.first -= 1;
		currRight.first -= 1;
		currLeft.first	-= 1;
		break;
	default:
		break;
	}
}

void Agent::DebugPrint()
{
	//std::cout << "	//----- Position : " << currX << ", " << currY << std::endl;

	//switch (direction)
	//{
	//case N:
	//	std::cout << "	//----- Direction : NORTH\n";
	//	break;
	//case E:
	//	std::cout << "	//----- Direction : EAST\n";
	//	break;
	//case S:
	//	std::cout << "	//----- Direction : SOUTH\n";
	//	break;
	//case W:
	//	std::cout << "	//----- Direction : WEST\n";
	//	break;
	//default:
	//	break;
	//}
}

bool Agent::IsVisited(int x, int y)
{
	for(auto& it : Map)
	{
		if (it.PosX == x && it.PosY == y)
			return true;
	}
	return false;
}

void Agent::AdjustPositionAfterBump()
{
	switch (direction)
	{
	case N:
		currY -= 1;
		currBack.second		-= 1;
		currFront.second	-= 1;
		currRight.second	-= 1;
		currLeft.second		-= 1;
		break;
	case E:
		currX -= 1;
		currBack.first		-= 1;
		currFront.first -= 1;
		currRight.first -= 1;
		currLeft.first -= 1;
		break;
	case S:
		currY += 1;
		currBack.second		+= 1;
		currFront.second	+= 1;
		currRight.second	+= 1;
		currLeft.second		+= 1;
		break;
	case W:
		currX += 1;
		currBack.first += 1;
		currFront.first += 1;
		currRight.first += 1;
		currLeft.first += 1;
		break;
	default:
		break;
	}
}

void Agent::AddToMap(int x, int y, TYPE type)
{
	Tile newTile;
	newTile.PosX = x;
	newTile.PosY = y;
	newTile.type = type;

	Map.push_back(newTile);
}
