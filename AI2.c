#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<stdbool.h>
#define GridSize 6
#pragma warning(disable: 4996)
int history[200][2];
typedef enum { CHEATER, STUPID, RANDOM, HUMAN, INTELLIGENT } Mode;

// There are possible states in the simple reflex environment. 
typedef enum { UNKNOWN, SAFE, WALLS, BUMPS, GOALS, GOLDS, BREEZES, PITS, STENCHES, WUMPI } WhatWorld;

// To maintain my direction.
typedef enum { E, W, N, S } Direction;

typedef enum { NONE, TURN_LEFT, SHOOT, GOFORWARD, TURN_RIGHT, GRAB, CLIMB } actionList;
typedef struct WorldEnviron {
	bool unknown;
	bool safe;
	bool walls;
	bool bumps;
	bool goals;
	bool glitter;
	bool golds;
	bool breezes;
	bool pits;
	bool stenches;
	bool wumpi;
	bool scream;
}WorldEnv;

typedef struct agent_state {
	WorldEnv Grid[6 * 6];
	WorldEnv cur_percept;
	int x_pos;
	int y_pos;
	Direction MyHeading;
	actionList LastAction;

	int arrow;
	bool gold;
	bool alive;
}agentMode;


void SetBumps(int x, int y, WorldEnv* Grid) { Grid[GridSize * x + y].bumps = true; }
void SetGoals(int x, int y, WorldEnv* Grid) { Grid[GridSize * x + y].goals = true; }
void SetGolds(int x, int y, WorldEnv* Grid) { Grid[GridSize * x + y].golds = true; }
void SetBreezes(int x, int y, WorldEnv* Grid) { Grid[GridSize * x + y].breezes = true; }
void SetPits(int x, int y, WorldEnv* Grid) { Grid[GridSize * x + y].pits = true; }
void SetStenches(int x, int y, WorldEnv* Grid) { Grid[GridSize * x + y].stenches = true; }
void SetWumpi(int x, int y, WorldEnv* Grid) { Grid[GridSize * x + y].wumpi = true; }
void SetSafe(int x, int y, WorldEnv* Grid) { Grid[GridSize * x + y].safe = true; }
WorldEnv GetState(int x, int y, WorldEnv* Grid) { return Grid[GridSize * x + y]; }

void SetMyPos(int x, int y, agentMode* ag) { ag->x_pos = x; ag->y_pos = y; }

void realWorldSet(WorldEnv* RealWorld);
actionList ChooseAction(agentMode* agent);
void UpdateState(agentMode* agentSt);
bool checkAlive(agentMode* agent);
void StateSet(agentMode* agentSt);
void percept(agentMode* agent, WorldEnv* RealWorld);
void showMap(WorldEnv* Map, int x_pos, int y_pos);


int main() {

	agentMode agentMan;
	memset(agentMan.Grid, false, sizeof(WorldEnv) * 36); // agent 초기화
	StateSet(&agentMan);

	WorldEnv RealWorld[6 * 6] = { 0, };
	memset(RealWorld, false, sizeof(WorldEnv) * 36);
	realWorldSet(RealWorld);
	int i = 0;

	while (1)
	{
		char test[100];
		percept(&agentMan, RealWorld);
		if (!checkAlive(&agentMan))
		{
			StateSet(&agentMan); // agent 죽으면 초기화
			agentMan.MyHeading = E;
			agentMan.x_pos = 1;
			agentMan.y_pos = 1;
			for (i = 0; i < 200; i++) {
				history[i][0] = 0;
				history[i][1] = 0;
			}
			i = 0;
			continue;

		}
		history[i][0] = agentMan.x_pos;
		history[i][1] = agentMan.y_pos;
		UpdateState(&agentMan);
		i++;

		puts("\n====RealWorld Map========");
		showMap(RealWorld, 9, 9);
		puts("\n=======Agent Map=========");
		showMap(agentMan.Grid, agentMan.x_pos, agentMan.y_pos);

		if (agentMan.gold) {
			i--;
			while (i > 0) {
				i--;
				agentMan.x_pos = history[i][0];
				agentMan.y_pos = history[i][1];
				puts("\n====RealWorld Map========");
				showMap(RealWorld, 9, 9);
				puts("\n=======Agent Map=========");
				showMap(agentMan.Grid, agentMan.x_pos, agentMan.y_pos);
				if (!agentMan.cur_percept.bumps)
					printf("\n[current posi : %d,%d]=======\n", agentMan.x_pos, agentMan.y_pos);
				gets_s(test, 3);
			}
			percept(&agentMan, RealWorld);
			agentMan.cur_percept.goals = true;
			agentMan.LastAction = ChooseAction(&agentMan);
			break;
		}

		if (!agentMan.cur_percept.bumps)
			printf("\n[current posi : %d,%d]=======\n", agentMan.x_pos, agentMan.y_pos);
		
		agentMan.LastAction = ChooseAction(&agentMan);

		if (agentMan.cur_percept.bumps)
			printf("\n[current posi : %d,%d]=======\n", agentMan.x_pos, agentMan.y_pos);

		gets_s(test, 3);
	}

}

void showMap(WorldEnv* Map, int x_pos, int y_pos)
{
	bool print = false;
	char buf[15] = { 0 };
	for (int y = 5; y >= 0; y--)
	{

		printf("\n  ");
		for (int x = 0; x < 6; x++)
		{
			print = false;
			memset(buf, 0, sizeof(char) * 15);
			if (Map[GridSize * x + y].walls)
			{
				strcat(buf, "W.");
				printf("%6s |", buf);

				continue;
			}

			if (x == x_pos && y == y_pos) {
				strcat(buf, "??");
			}
				

			if (Map[GridSize * x + y].breezes)
			{
				strcat(buf, "B.");
			}
			if (Map[GridSize * x + y].pits)
			{
				strcat(buf, "P.");
			}

			if (Map[GridSize * x + y].safe)
			{
				strcat(buf, "Safe.");
			}
			if (Map[GridSize * x + y].golds)
			{
				strcat(buf, "G.");
			}
			if (Map[GridSize * x + y].glitter) {
				strcat(buf, "Gl.");
			}
			if (Map[GridSize * x + y].stenches)
			{
				strcat(buf, "ST.");
			}
			if (Map[GridSize * x + y].wumpi)
			{
				strcat(buf, "WP.");
			}

			if ((int)*buf == 0)
				strcat(buf, "==?==");

			printf("%6s |", buf);
		}

		printf("  \n");
	}

}

void realWorldSet(WorldEnv* RealWorld) {

	// X Walls
	for (int i = 0; i < 6; i++)
	{
		RealWorld[6 * 0 + i].walls = true;
		RealWorld[6 * 5 + i].walls = true;
	}

	// Y Walls
	for (int i = 0; i < 6; i++)
	{
		RealWorld[6 * i + 0].walls = true;
		RealWorld[6 * i + 5].walls = true;
	}

	RealWorld[6 * 1 + 1].safe = true;  // 1,1 safe
	RealWorld[6 * 1 + 2].safe = true;  // 1,2 safe
	RealWorld[6 * 2 + 1].safe = true;  // 2,1 safe
	
	srand((unsigned int)time(NULL));


	for (int i = 1; i < 5; i++)
		for (int j = 1; j < 5; j++)
		{
			
			int percent = rand() % 100;
			if (percent < 15 && RealWorld[6 * i + j].golds != true){ 
				if (i != 1 && j != 1) {
					RealWorld[6 * i + j].golds = true;
					RealWorld[6 * (i - 1) + j].glitter = true;
					RealWorld[6 * (i + 1) + j].glitter = true;
					RealWorld[6 * i + (j - 1)].glitter = true;
					RealWorld[6 * i + (j + 1)].glitter = true;
					printf("golds : (%d,%d)\n", i, j);
				}
			}
			
			
			
			percent = rand() % 100;
			if (percent < 15 && RealWorld[6 * i + j].golds != true && RealWorld[6 * i + j].safe != true) {
				RealWorld[6 * i + j].wumpi = true;

				// set stenches
				RealWorld[6 * (i - 1) + j].stenches = true;
				RealWorld[6 * (i + 1) + j].stenches = true;
				RealWorld[6 * i + (j + 1)].stenches = true;
				RealWorld[6 * i + (j - 1)].stenches = true;

				printf("wumpus : (%d,%d)\n", i, j);

			}


			//Pits
			percent = rand() % 100;
			if (percent < 15 && RealWorld[6 * i + j].golds != true && RealWorld[6 * i + j].safe != true) {
				RealWorld[6 * i + j].pits = true;

				// set breeze
				RealWorld[6 * (i - 1) + j].breezes = true;
				RealWorld[6 * (i + 1) + j].breezes = true;
				RealWorld[6 * i + (j + 1)].breezes = true;
				RealWorld[6 * i + (j - 1)].breezes = true;

				printf("pits : (%d,%d)\n", i, j);
			}

		}

}

void percept(agentMode* agent, WorldEnv* RealWorld)
{
	int x, y;

	x = agent->x_pos;
	y = agent->y_pos;

	agent->cur_percept = RealWorld[6 * x + y];

	if (agent->LastAction == SHOOT)
	{
		agent->cur_percept.scream = true;
		switch (agent->MyHeading) {
		case E:
			agent->Grid[GridSize * (x + 1) + y].wumpi = false;
			RealWorld[GridSize * (x + 1) + y].wumpi = false;
			break;
		case W:
			agent->Grid[GridSize * (x - 1) + y].wumpi = false;
			RealWorld[GridSize * (x - 1) + y].wumpi = false;
			break;
		case N:
			agent->Grid[GridSize * x + y + 1].wumpi = false;
			RealWorld[GridSize * x + y + 1].wumpi = false;
			break;
		case S:
			agent->Grid[GridSize * x + y - 1].wumpi = false;
			RealWorld[GridSize * x + y - 1].wumpi = false;
			break;
		}

		puts("kill wumpi!");
	}

	if (RealWorld[6 * x + y].walls)
	{
		agent->cur_percept.bumps = true;
	
	}
	if (RealWorld[6 * x + y].glitter) {
		agent->cur_percept.glitter = true;
	}

	if (agent->gold)
		agent->cur_percept.golds = false;


	printf("\n percept space (%d,%d)\n", x, y);
}


bool checkAlive(agentMode* agent)
{
	int x, y;
	x = agent->x_pos;
	y = agent->y_pos;

	if (agent->cur_percept.pits || agent->cur_percept.wumpi)
	{

		agent->alive = false;
		puts("agent is dead");
		if (agent->cur_percept.pits)
			agent->Grid[GridSize * x + y].pits = true;
		if (agent->cur_percept.wumpi)
			agent->Grid[GridSize * x + y].wumpi = true;
		return false;
	}


}

void StateSet(agentMode* agentSt)
{
	WorldEnv* grid_array = agentSt->Grid;

	grid_array[6 * 1 + 1].safe = true; 
	agentSt->MyHeading = E;


	agentSt->arrow = 2;
	agentSt->gold = false;
	agentSt->x_pos = 1;
	agentSt->y_pos = 1;
}





void UpdateState(agentMode* agentSt)
{
	int x, y;
	WorldEnv perception;

	x = agentSt->x_pos;
	y = agentSt->y_pos;
	perception = agentSt->cur_percept;



	// 벽이 있는지 확인하는 조건문
	if (perception.bumps) {

		switch (agentSt->MyHeading) {
	
		case E: agentSt->Grid[6 * x + y].walls = true; break;
		case W: agentSt->Grid[6 * x + y].walls = true; break;
		case N: agentSt->Grid[6 * x + y].walls = true; break;
		case S: agentSt->Grid[6 * x + y].walls = true; break;
		}
		printf("[bump]  (%d,%d) is wall !!\n", x, y);
	}

	// agent가 gold를 줍고 위치가 (1, 1)에 도달한다면 goal
	if ((agentSt->gold) && (x == 1) && (y == 1)) 
	{
		agentSt->Grid[6 * x + y].goals = true;
		puts("goal!!!!!!!!!!!!!!!!!!!!!");
		exit(1);
	}

	// glitter를 percept하고 agent가 gold를 가지고 있지 않다면 그 위치로 이동해 get gold

	if (perception.glitter && agentSt->gold == false) {
		switch (agentSt->MyHeading) {

		case E: agentSt->Grid[6 * x + y].glitter= true; break;
		case W: agentSt->Grid[6 * x + y].glitter = true; break;
		case N: agentSt->Grid[6 * x + y].glitter = true; break;
		case S: agentSt->Grid[6 * x + y].glitter = true; break;
		}
		printf("[glitter]  (%d,%d) is glitter!!\n", x, y);
	}


	if (perception.stenches) agentSt->Grid[6 * x + y].stenches = true;
	if (perception.breezes) agentSt->Grid[6 * x + y].breezes = true;
	if (!perception.breezes && !perception.pits && !perception.stenches && !perception.wumpi)
		agentSt->Grid[6 * x + y].safe = true; 
}


actionList ChooseAction(agentMode* agent)
{

	actionList action = NONE;
	int x, y;

	x = agent->x_pos;
	y = agent->y_pos;

	WorldEnv state = agent->cur_percept;


	if (state.goals)
	{
		action = CLIMB;
		puts("CLIMB!");
		return action;
	}
	if (state.golds)
	{
		puts("i got gold!");
		action = GRAB;
		agent->gold = true;
		return action;
	}
	if (state.breezes) {

		switch (agent->MyHeading) {
		case E:
			if (agent->Grid[6 * (x + 1) + y].pits) {
				action = TURN_LEFT;
				agent->MyHeading = N;
			}
			else {
				action = GOFORWARD;
				SetMyPos(x + 1, y, agent);
			}
			break;
		case W:
			if (agent->Grid[6 * (x - 1) + y].pits) {
				action = TURN_LEFT;
				agent->MyHeading = S;
			}
			else {
				action = GOFORWARD;
				SetMyPos(x - 1, y, agent);
			}
			break;
		case N:
			if (agent->Grid[6 * x + y + 1].pits) {
				action = TURN_LEFT;
				agent->MyHeading = W;
			}
			else {
				action = GOFORWARD;
				SetMyPos(x, y + 1, agent);
			}
			break;
		case S:
			if (agent->Grid[6 * x + y - 1].pits) {
				action = TURN_LEFT;
				agent->MyHeading = E;
			}
			else {
				action = GOFORWARD;
				SetMyPos(x, y - 1, agent);
			}
			break;
		}

	}
	if (state.stenches)
	{
		switch (agent->MyHeading) {
		case E:
			if ((GetState(x + 1, y, agent->Grid).wumpi) && (agent->arrow > 0)) action = SHOOT;
			else {
				action = GOFORWARD;
				SetMyPos(x + 1, y, agent);
			}
			break;
		case W:
			if ((GetState(x - 1, y, agent->Grid).wumpi) && (agent->arrow > 0)) action = SHOOT;
			else {
				action = GOFORWARD;
				SetMyPos(x - 1, y, agent);
			}
			break;
		case N:
			if ((GetState(x, y + 1, agent->Grid).wumpi) && (agent->arrow > 0)) action = SHOOT;
			else {
				action = GOFORWARD;
				SetMyPos(x, y + 1, agent);
			}
			break;
		case S:
			if ((GetState(x, y - 1, agent->Grid).wumpi) && (agent->arrow > 0)) action = SHOOT;
			else {
				action = GOFORWARD;
				SetMyPos(x, y - 1, agent);
			}
			break;
		}

	}

	if (state.bumps)
	{

		if (agent->LastAction == TURN_LEFT)
		{
			action = GOFORWARD;
			switch (agent->MyHeading) {
			case E: SetMyPos(x + 1, y, agent); break;
			case W: SetMyPos(x - 1, y, agent); break;
			case N: SetMyPos(x, y + 1, agent); break;
			case S: SetMyPos(x, y - 1, agent); break;
			}

		}
		else {

			action = TURN_LEFT;
			switch (agent->MyHeading) {
			case E: agent->MyHeading = N; SetMyPos(x - 1, y, agent); break;
			case W: agent->MyHeading = S; SetMyPos(x + 1, y, agent); break;
			case N: agent->MyHeading = W; SetMyPos(x, y - 1, agent);  break;
			case S: agent->MyHeading = E; SetMyPos(x, y + 1, agent); break;
			}
		}

	}

	if (action == NONE)
	{
		action = GOFORWARD;

		switch (agent->MyHeading) {
		case E:
			SetMyPos(x + 1, y, agent);
			break;
		case W: SetMyPos(x - 1, y, agent); break;
		case N: SetMyPos(x, y + 1, agent); break;
		case S: SetMyPos(x, y - 1, agent); break;
		}

	}

	switch (action) {
	case GOFORWARD: puts("GOFORWARD"); break;
	case TURN_LEFT: puts("TURN LEFT"); break;
	case TURN_RIGHT: puts("TURN RIGHT"); break;
	case SHOOT: puts("SHOOT Arrow"); break;
	}
	return action;
}