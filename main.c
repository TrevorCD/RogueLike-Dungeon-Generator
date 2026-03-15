/* Trevor Ben Calderwood
 * Roguelike tilemap generator prototype
 *
 * Under GNU copyleft
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "defns.h"

#define DEBUG 1

int distance(int x1, int y1, int x2, int y2) {

	int xdiff = abs(x1 - x2);
	int ydiff = abs(y1 - y2);
	double a2b2 = (double) ((xdiff * xdiff) + (ydiff * ydiff));
	int hypot = (int) sqrt(a2b2);
	return hypot;
}

void _drawRoom(char ** board , SubRoom * room) {
    
    int startx = (room->x - room->w);
    int endx = (room->x + room->w );
    int starty = (room->y - room->l);
    int endy = (room->y + room->l);
    
    //top row
    for(int i = startx; i <= endx; i++) {
		if(board[starty][i] == ' ') {
			board[starty][i] = '#';
		}
    }
    //bottom row
    for(int i = startx; i <= endx; i++) {
		if(board[endy][i] == ' ') {
			board[endy][i] = '#';
		}
    }
    //sides
    for(int i = starty + 1; i < endy; i++) {
		if(board[i][startx] == ' ') {
			board[i][startx] = '#';
		}
		if(board[i][endx] == ' ') {
			board[i][endx] = '#';
		}
    }
    //inside
    for (int i = starty + 1; i < endy; i++) {
		for(int j = startx + 1; j < endx; j++) {
			board[i][j] = '.';
		}
    }
}

void printBoard(char ** board) {
    for(int i = 0; i < TRUEWID; i++) {
		printf(board[i]);
		printf("\n");
    }
}

// returns 1 if r1 and r2 have a collision. Returns 2 if only corners collide
int collisionCheck(char ** board, SubRoom * r1, SubRoom * r2) {

    char xCol;
    char yCol;
    
    int r1x, r1y, r1w, r1l, r2x, r2y, r2w, r2l;
    int xCorner, yCorner; // for literal edge case

	r1x = r1->x; r1y = r1->y; r1w = r1->w; r1l = r1->l;
	
	r2x = r2->x; r2y = r2->y; r2w = r2->w; r2l = r2->l;
	xCol = 0; yCol = 0; xCorner = 0; yCorner = 0;

	// x collisions
	if(r1x < r2x) { //check RHS of r1 for collision
		if(r1x + r1w > r2x - r2w) {
			xCol = 1;
			if(r1x + r1w == (r2x - r2w) + 1) {
				xCorner = 1;
				//printf("xCorner %d %d!\n", r1->id, r2->id);
			}
		}
	}
	else if(r1x > r2x) { //case 1: check LHS of r1 for collision
		if(r1x - r1w < r2x + r2w) {
			xCol = 1;
			if((r1x - r1w) + 1 == r2x + r2w) {
				xCorner = 1;
				//printf("xCorner %d %d!\n", r1->id, r2->id);
			}
		}
	}
	else //case 2: r1x == r2x
		xCol = 1;

	    
	// y collisions
	if(r1y < r2y) { //check bottom of r1 for collision
		if(r1y + r1l > r2y - r2l) {
			yCol = 1;
			if(r1y + r1l == (r2y - r2l) + 1) {
				yCorner = 1;
				//printf("yCorner %d %d!\n", r1->id, r2->id);
			}
		}
	}
	else if(r1y > r2y) { //case 1: check top of r1 for collision
		if(r1y - r1l < r2y + r2l) {
			yCol = 1;
			if((r1y - r1l) + 1 == r2y + r2l) {
				yCorner = 1;
				//printf("yCorner %d %d!\n", r1->id, r2->id);
			}
		}
	}
	else //case 2: r1y == r2y
		yCol = 1;

	    
	if(yCol && xCol) {
		if(xCorner && yCorner) {
			return 2;
		}
		return 1;
	}
	return 0;
}

// debug function: draws each room's ID in center of room on board
// prints IDs in base N_SUBROOMS
void debug_drawRoomIds(Level * level) {
    char idChar;
    int n = level->nRooms;
	Room * room;
	
    for(int i = 0; i < n; i++) {
		room = &level->roomList[i];
	
		if(i < 10)
			idChar = i + 48;
		else
			idChar = i + 55;
	
		level->board[room->y][room->x] = idChar;
    }
    
}

void generateSubRoom(SubRoom * subRoom) {

	int l = rand() % MAX_L;
	if(l < MIN_L)
		l = MIN_L;
	subRoom->l = l;
	
	int w = rand() % MAX_W;
	if(w < MIN_W)
		w = MIN_W;
	subRoom->w = w;
	
	int x = rand() % MAX_X;
	if(x < MIN_X)
		x = MIN_X;
	subRoom->x = x;
      
	int y = rand() % MAX_Y;
	if(y < MIN_Y)
		y = MIN_Y;
	subRoom->y = y;

	subRoom->id = -1;
	
}

// creates and initializes board. Returns pointer to board
char ** initBoard() {

	//init 100x100 board to 0s
    char ** board = (char **) malloc(sizeof(char *) * TRUEWID);
	if(board == NULL) return NULL;
    // + 1 char in row for EOL
    // each index of board points to a new row in trueBoard
    char * trueBoard = (char *) malloc(sizeof(char) * (TRUESIZE + TRUEWID));
	if(trueBoard == NULL) {
		free(board);
		return NULL;
	}
	
    int currIndex = 0;
    for(int i = 0; i < TRUEWID ; i++) {
		board[i] = &trueBoard[currIndex];
		for(int j = 0 ; j < TRUELEN ; j++) {
			board[i][j] = ' ';
		}
		board[i][TRUELEN] = 0;
		currIndex += (TRUELEN + 1);
    }

	return board;
}

// generates rooms and returns list of rooms
int generateRooms(Level * level) {
	level->subRoomList = (SubRoom *) malloc(sizeof(SubRoom) * N_SUBROOMS);
	if(level->subRoomList == NULL) return -1;
	
	int subRoomsLeft = N_SUBROOMS;
	int subRoomsPerRoom[25];
	int roomIndex = 0;
	int subrooms;

	/* Get the number of subrooms per actual room (max of 10 subrooms) */
	while(subRoomsLeft > 0) {
		subrooms = rand() % MAX_SUBROOMS;
		subrooms++; // to have range 1 - MAX_SUBROOMS
		if(subrooms > subRoomsLeft)
			subrooms = subRoomsLeft;
		subRoomsPerRoom[roomIndex] = subrooms;
		roomIndex++;
		subRoomsLeft -= subrooms;
	}

	int nRooms = roomIndex;
	level->nRooms = nRooms;
	level->roomList = (Room *) malloc(sizeof(Room) * nRooms);
	if(level->roomList == NULL) {
		free(level->subRoomList);
		return -1;
	}	
	
	/* Generate each actual room via subroom generation */
	int subRoomListIndex = 0;
	int firstSubRoom = 0; // first sub room this room
	bool genSuccess = false;
	
	for(int i = 0; i < nRooms; i++) {
		for(int j = 0; j < subRoomsPerRoom[i]; j++) {
			genSuccess = false;
			while(genSuccess == false) {
				generateSubRoom(&level->subRoomList[subRoomListIndex]);
				// first subroom needs to be true before previous room check
				if(j == 0) genSuccess = true;
				
				// ensure collision with previous subroom in this room
				for(int i = firstSubRoom; i < subRoomListIndex; i++) {
					if(collisionCheck(
						   level->board,
						   &level->subRoomList[subRoomListIndex],
						   &level->subRoomList[i]) == 1) {
						// successful collision with subroom in this room
						genSuccess = true;
						break;
					}
				}
				// ensure no collision with previous rooms
				for(int i = 0; i < firstSubRoom; i++) {
					if(collisionCheck(
						   level->board,
						   &level->subRoomList[subRoomListIndex],
						   &level->subRoomList[i]) != 0) {
						// collision with previous room
						genSuccess = false;
						break;
					}
				}
			}
			level->subRoomList[subRoomListIndex].id = subRoomListIndex;
			subRoomListIndex++;
		}
		firstSubRoom += subRoomsPerRoom[i];
	}

	/* Populate roomList with room information */
    int numPrevSubRooms = 0;
	int subRoomEnd;
	int avgX, avgY;
	
	for(int i = 0; i < nRooms; i++) {
		avgX = 0; avgY = 0;
		
		level->roomList[i].id = i;
		level->roomList[i].nSubRooms = subRoomsPerRoom[i];
		level->roomList[i].subRooms = &level->subRoomList[numPrevSubRooms];

		subRoomEnd = numPrevSubRooms + subRoomsPerRoom[i];
		for(int j = numPrevSubRooms; j < subRoomEnd; j++) {
			avgX += level->subRoomList[j].x;
			avgY += level->subRoomList[j].y;
		}
		avgX /= subRoomsPerRoom[i];
		avgY /= subRoomsPerRoom[i];
		level->roomList[i].x = avgX;
		level->roomList[i].y = avgY;
		
		numPrevSubRooms += subRoomsPerRoom[i];
	}
	
	return 0;
}

int generateLevel(Level * level, int id) {	
	level->levelId = id;	
	level->board = initBoard();
	if(level->board == NULL) return -1;
	
	if(generateRooms(level) != 0) {
		// free board
		return -1;
	}
	
	// draw rooms to board
	for(int i = 0; i < N_SUBROOMS ; i++) {		
		_drawRoom(level->board, &level->subRoomList[i]);
    }
	
	// create halls

	// draw halls to board

	return 0;
}

int main() {

#if DEBUG
    clock_t start, end;
    double cpu_time;
    start = clock();
#endif
    system("clear");
	srand(time(NULL));
	
	Level level = {0};
	if(generateLevel(&level, 0) != 0) return -1;

#if DEBUG
	debug_drawRoomIds(&level);
#endif
    
	printBoard(level.board);
	
#if DEBUG
    end = clock();
    cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("cpu time: %f\n", cpu_time);
#endif
	
	return 0;
}
