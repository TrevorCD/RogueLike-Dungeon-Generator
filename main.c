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

void _drawRoom(char ** board , Room * room) {
    
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
int collisionCheck(char ** board, Room * r1, Room * r2) {

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

double length(double x1, double y1, double x2, double y2) {
    double a = x1 - x2;
    double b = y1 - y2;
    return sqrt((a*a) + (b*b));
}

// finds the center point of the circumcircle of the triangle the 3 input
// vertices create.
void circumCircleCenter(Room * one, Room * two, Room * three, Coord * output) {
    
    double x1 = one->x;
    double y1 = one->y;
    double x2 = two->x;
    double y2 = two->y;
    double x3 = three->x;
    double y3 = three->y;


    double x, y; // coordinates of circumcircle
    double invSlope12, invSlope23; // perpendicular slope to lines 1,2 and 2,3
    Coord midpoint12, midpoint23;
    double b12, b23; // b in y = mx + b
    
    midpoint12.x = (x1 + x2) / 2;
    midpoint12.y = (y1 + y2) / 2;
    midpoint23.x = (x2 + x3) / 2;
    midpoint23.y = (y2 + y3) / 2;

    invSlope12 = -1 / ((y2 - y1) / (x2 - x1));
    invSlope23 = -1 / ((y3 - y2) / (x3 - x2));

    b12 = midpoint12.y - (invSlope12 * midpoint12.x);
    b23 = midpoint23.y - (invSlope23 * midpoint23.x);

    x = ((b23 - b12) * invSlope23) / invSlope12;
    y = (invSlope12 * x) + b12;
    
    output->x = x;
    output->y = y;
    
    return;
}

// debug function: draws each room's ID in center of room on board
// prints IDs in base NUMROOMS
void debug_drawRoomIds(char ** board, Room * rooms, int n) {
    char idChar;
    Room * room;
    
    for(int i = 0; i < n; i++) {
		room = &rooms[i];
	
		if(i < 10)
			idChar = i + 48;
		else
			idChar = i + 55;
	
		board[room->y][room->x] = idChar;
    }
    
}

// Rudimentary calculation. Does not take into account overlap between rooms
// Summation of all room sizes in list
int findTrueRoomSize(Room * room) {
    int size = room->w * room->l * 4;
    while(room->child) {
		room = room->child;
		size += (room->w * room->l * 4);
    }
    return size;
}

// given 3 nodes, init triangle defined by them. Also calcualtes circumcircle
// center and circumcircle radius
Tri * newTri(Node * one, Node * two, Node * three) {
    Tri * tri = malloc(sizeof(Tri));
    tri->nodes[0] = one;
    tri->nodes[1] = two;
    tri->nodes[2] = two;
    circumCircleCenter(one->room, two->room, three->room, &tri->center);
    tri->radius = length(one->room->x, one->room->y,
						  tri->center.x, tri->center.y);
    return tri;
}

// deallocation of tri struct
void delTri(Tri * tri) {
    free(tri);
    return;
}

void delaunayTriangulation(Node * nodes, int nNodes,
							Node * one, Node * two, Node * three) {
    Tri * tris[nNodes];
    int i;
    for(i = 1; i < nNodes; i++)
		tris[i] = 0;
    // init support tri
    Node * supports = malloc(sizeof(Node) * 3);
    Room * facadeRooms = malloc(sizeof(Room) * 3);
    facadeRooms[0].id = -1;
    facadeRooms[0].x = (0 - (TRUELEN / 2));
    facadeRooms[0].y = 0;
    supports[0].room = &facadeRooms[0];

    facadeRooms[1].id = -1;
    facadeRooms[1].x = (TRUELEN + (TRUELEN / 2));
    facadeRooms[1].y = 0;
    supports[1].room = &facadeRooms[1];

    facadeRooms[2].id = -1;
    facadeRooms[2].x = TRUELEN / 2;
    facadeRooms[2].y = TRUEWID * 2;
    supports[2].room = &facadeRooms[2];
    
    tris[0] = newTri(&supports[0], &supports[1], &supports[2]);
    int nTris = 1;

    char hits[nNodes]; // keeps track of collisions for each nx iteration
    for(i = 0; i < nNodes; i++)
		hits[i] = 0;
    
    Node * point;
    Tri * t;
    double distance;
    Room * room;

    int nx, tx, tax, nHits;
    for(nx = 0; nx < nNodes; nx++) {
		point = &nodes[nx];
		room = point->room;
		// identify which tris' circumcircles contain this point
		// tx is the index of the actual tri. tax is the index into the array
		// where that tri * exists. This way I don't have to use linked lists
		// or shift elements in the array
		tx = 0; tax = 0; nHits = 0;
		while(tx < nTris)  {
			if(tris[tax] != 0) {
				tx++;
				t = tris[tax];
				distance = length(room->x, room->y, t->center.x, t->center.y);
				// may need an epsilon here to ignore points in the tri
				if(distance < t->radius) { //hit!
					hits[tax] = 1;
					nHits++;
				}
				if(nHits) {
					// retriangulation

					// STOPPED WORKING HERE ------------------------------------
		    

					//
				}
			}
			tax++;
			/// TEMP DEBUG PRINT
			if(tax == nNodes) {printf("TRI BUFF OVERFLOW!!!\n"); exit(0);}
		}
		// restructure triangles
    }
    // destroy any triangles that have a support node

    free(supports); free(facadeRooms);
}

void generateSubRoom(Room * subRoom) {

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

	subRoom->parent = 0;	
	subRoom->child = 0;	
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
Room * generateRooms(char ** board) {

	Room * roomList = (Room *) malloc(sizeof(Room) * NUMROOMS);

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
	int roomListIndex = 0;
	int firstSubRoom = 0; // first sub room this room

	bool genSuccess = false;
	
	for(int i = 0; i < nRooms; i++) {

		for(int j = 0; j < subRoomsPerRoom[i]; j++) {
			genSuccess = false;
			
			while(genSuccess == false) {
				generateSubRoom(&roomList[roomListIndex]);
				// first subroom needs to be true before previous room check
				if(j == 0) genSuccess = true;
				
				// ensure collision with previous subroom in this room
				for(int i = firstSubRoom; i < roomListIndex; i++) {
					if(collisionCheck(
						   board,
						   &roomList[roomListIndex],
						   &roomList[i]) == 1)
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
						   &roomList[roomListIndex],
						   &roomList[i]) != 0)
					{
						// collision with previous room
						genSuccess = false;
						break;
					}
				}
			}
			
			roomListIndex++;
		}
		firstSubRoom += subRoomsPerRoom[i];
		
	}
	
	return roomList;
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
    
	Room * roomList = generateRooms(board);
	
    for(int i = 0; i < NUMROOMS ; i++) {		
		_drawRoom(board, &roomList[i]);
    }

	printBoard(board);
    
    /// DEBUG
    end = clock();
    cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("cpu time: %f\n", cpu_time);
    /// DEBUG
}
