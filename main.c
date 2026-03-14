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
// prints IDs in base NUMROOMS
void debug_drawRoomIds(char ** board, SubRoom * rooms, int n) {
    char idChar;
    SubRoom * room;
    
    for(int i = 0; i < n; i++) {
		room = &rooms[i];
	
		if(i < 10)
			idChar = i + 48;
		else
			idChar = i + 55;
	
		board[room->y][room->x] = idChar;
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
    // + 1 char in row for EOL
    // each index of board points to a new row in trueBoard
    char * trueBoard = (char *) malloc(sizeof(char) * (TRUESIZE + TRUEWID));
	
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
SubRoom * generateRooms(char ** board) {

	SubRoom * subRoomList = (SubRoom *) malloc(sizeof(SubRoom) * NUMROOMS);

	/* Get the number of subrooms per actual room (max of 10 subrooms) */
	
	const int maxSubRooms = 10;
	int subRoomsLeft = NUMROOMS;
	int subRoomsPerRoom[25];
	int roomIndex = 0;
	int subrooms;

	while(subRoomsLeft > 0) {
		
		subrooms = rand() % maxSubRooms;
		subrooms++; // to have range 1 - maxSubRooms

		if(subrooms > subRoomsLeft) subrooms = subRoomsLeft;

		subRoomsPerRoom[roomIndex] = subrooms;
		roomIndex++;

		subRoomsLeft -= subrooms;
	}

	int nRooms = roomIndex;
	
	for(int i = 0; i < nRooms; i++) {
		printf("Room %d: %d sub rooms\n", i, subRoomsPerRoom[i]);
	}

	
	/* Generate each actual room via subroom generation */
	int subRoomListIndex = 0;
	int firstSubRoom = 0; // first sub room this room

	bool genSuccess = false;
	
	for(int i = 0; i < nRooms; i++) {

		for(int j = 0; j < subRoomsPerRoom[i]; j++) {
			genSuccess = false;
			
			while(genSuccess == false) {
				generateSubRoom(&subRoomList[subRoomListIndex]);
				// first subroom needs to be true before previous room check
				if(j == 0) genSuccess = true;
				
				// ensure collision with previous subroom in this room
				for(int i = firstSubRoom; i < subRoomListIndex; i++) {
					if(collisionCheck(
						   board,
						   &subRoomList[subRoomListIndex],
						   &subRoomList[i]) == 1)
					{
						// successful collision with subroom in this room
						genSuccess = true;
						break;
					}
				}
				
				// ensure no collision with previous rooms
				for(int i = 0; i < firstSubRoom; i++) {
					if(collisionCheck(
						   board,
						   &subRoomList[subRoomListIndex],
						   &subRoomList[i]) != 0)
					{
						// collision with previous room
						genSuccess = false;
						break;
					}
				}
			}
			subRoomList[subRoomListIndex].id = subRoomListIndex;
			subRoomListIndex++;
		}
		firstSubRoom += subRoomsPerRoom[i];
		
	}
	
	return subRoomList;
}

int main() {

    /// DEBUG
    clock_t start, end;
    double cpu_time;
    start = clock();
    /// DEBUG
    
    system("clear");
    
	char ** board = initBoard();
	
    srand(time(NULL));
    
	SubRoom * subRoomList = generateRooms(board);
	
    for(int i = 0; i < NUMROOMS ; i++) {		
		_drawRoom(board, &subRoomList[i]);
    }

	printBoard(board);
	debug_drawRoomIds(board, subRoomList, NUMROOMS);
		
    /// DEBUG
    end = clock();
    cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("cpu time: %f\n", cpu_time);
    /// DEBUG
}
