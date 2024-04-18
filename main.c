/* Trevor Ben Calderwood
 * Roguelike tilemap generator prototype
 *
 * Under GNU copyleft
 *
 * TODO:
 *   - Calculate size of each room post gen and collisions
 *   - Delaunay triangulation of largest rooms
 *   - MST using triangulation edges
 *   - Connect rest of rooms
 *   - Cellular automata for different tile types
 *      - Ex. Grass, stone, water, etc...
 *   - Sample perlin noise for tilemap variations
 *      - Ex. Height of grass for smooth transitions
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

// test values:

#define TRUELEN 150
#define TRUEWID 50
#define NUMROOMS 25


// stress test values
/*
#define TRUELEN 700
#define TRUEWID 200
#define NUMROOMS 500
*/

#define MAX_W 10
#define MAX_L 10
#define MIN_W 3
#define MIN_L 3

// DO NOT CHANGE THESE
#define TRUESIZE ( TRUELEN * TRUEWID )
#define MAX_X ( TRUELEN - ( MAX_W + 1 ) )
#define MAX_Y ( TRUEWID - ( MAX_L + 1 ) )
#define MIN_X ( MAX_L + 1 )
#define MIN_Y ( MAX_L + 1 )


typedef struct room {

    struct room * parent; //
    struct room * child;  // for linked list
    
    int l;         // length across x dimension
    int w;         // width across y dimension
    int x;         // horizontal dimension, grows right
    int y;         // vertical dimension, grows down
    
    char id;       //initial gen room id, index of roomList in main
    
} Room;

void _drawRoom( char ** board , Room * room ) {
    
    //printf("%d, %d\n", room->y, room->x );
    
    int startx = ( room->x - room->w );
    int endx = ( room->x + room->w  );
    int starty = ( room->y - room->l );
    int endy = ( room->y + room->l );
    
    //top row
    for( int i = startx; i <= endx; i++ ) {
	if( board[starty][i] == ' ' ) {
	    board[starty][i] = '#';
	}
    }
    //bottom row
    for( int i = startx; i <= endx; i++ ) {
	if( board[endy][i] == ' ' ) {
	    board[endy][i] = '#';
	}
    }
    //sides
    for( int i = starty + 1; i < endy; i++ ) {
	if( board[i][startx] == ' ' ) {
	    board[i][startx] = '#';
	}
	if( board[i][endx] == ' ' ) {
	    board[i][endx] = '#';
	}
    }
    //inside
    for (int i = starty + 1; i < endy; i++ ) {
	for( int j = startx + 1; j < endx; j++ ) {
	    board[i][j] = '.';
	}
    }
}

void printBoard( char ** board ) {

    for( int i = 0; i < TRUEWID; i++ ) {
	printf( board[i]);

	printf("\n");
    }

}

// Given lists r1 and r2, combine both into one sorted list
// Preconditions:
//  - (r1->id) < (r2->id)
//  - r1 and r2 are both sorted ( lowest at root )
void listZipper( Room * r1, Room * r2 ) {

    // go to roots of each 
    while( r2->parent != 0 )
	r2 = r2->parent;
    
    while( r1->parent != 0 )
	r1 = r1->parent;
    
    Room * tempChild;
    Room * tempRoom;
       
    while( r2 != 0 ) {

	if( r1 == r2 )
	    return;
	
	while( r2->id > r1->id ) {
	    if( r1->child == 0 ) {
		r2->parent = r1;
		r1->child = r2;
		return;
	    }
	    r1 = r1->child;
	}

	tempChild = r2->child;
	r2->parent = r1->parent; //r2 will always point to top of second list
	r2->child = r1;
	r1->parent = r2;
	r2 = tempChild;
	
    }
    return;
}

void collisionCheck( char ** board, Room * rooms, int n ) {

    char xCol;
    char yCol;
    
    Room * r1;
    Room * r2;

    int r1x, r1y, r1w, r1l, r2x, r2y, r2w, r2l;
    
    for( int i = 0; i < n; i++ ) {
	
	r1 = &rooms[i];
	r1x = r1->x; r1y = r1->y; r1w = r1->w; r1l = r1->l;
	
	for( int j = i + 1; j < n; j++ ) {
	    
	    r2 = &rooms[j];
	    r2x = r2->x; r2y = r2->y; r2w = r2->w; r2l = r2->l;
	    xCol = 0; yCol = 0;

	    // x collisions
	    if( r1x < r2x ) { //check RHS of r1 for collision
		if( r1x + r1w > r2x - r2w )
		    xCol = 1;
	    }
	    else if( r1x > r2x ) { //case 1: check LHS of r1 for collision
		if( r1x - r1w < r2x + r2w )
		    xCol = 1;
	    }
	    else //case 2: r1x == r2x
		xCol = 1;

	    
	    // y collisions
	    if( r1y < r2y ) { //check bottom of r1 for collision
		if( r1y + r1l > r2y - r2l )
		    yCol = 1;
	    }
	    else if( r1y > r2y ) { //case 1: check top of r1 for collision
		if( r1y - r1l < r2y + r2l )
		    yCol = 1;
	    }
	    else //case 2: r1y == r2y
		yCol = 1;

	    
	    if( yCol && xCol )
		listZipper( r1, r2 );
	    
	}// end for j loop	
    }// end for i loop
}

double edgeLength( double x1, double y1, double x2, double y2 ) {
    double a = x1 - x2;
    if( a < 0 )
	a = a * -1;
    double b = y1 - y2;
    if( b < 0 )
	b = b * -1;
    return sqrt( (a*a) + (b*b) );
}

typedef struct Coordinate {

    double x;
    double y;
    
} coord;


void circumcircleCenter( double x1, double y1, double x2, double y2, 
			 double x3, double y3, coord * output ) {
    
    double x, y; // coordinates of circumcircle
    double invSlope12, invSlope23;  // perpendicular slope to lines 1,2 and 2,3
    coord midpoint12, midpoint23; // midpoints of lines 1,2 and 2,3
    double b12, b23; // b in y = mx + b
    
    midpoint12.x = ( x1 + x2 ) / 2;
    midpoint12.y = ( y1 + y2 ) / 2;
    midpoint23.x = ( x2 + x3 ) / 2;
    midpoint23.y = ( y2 + y3 ) / 2;

    invSlope12 = -1 / (( y2 - y1 ) / ( x2 - x1 ));
    invSlope23 = -1 / (( y3 - y2 ) / ( x3 - x2 ));

    b12 = midpoint12.y - ( invSlope12 * midpoint12.x );
    b23 = midpoint23.y - ( invSlope23 * midpoint23.x );

    x = ( ( b23 - b12 ) * invSlope23 ) / invSlope12;
    y = ( invSlope12 * x ) + b12;
    
    output->x = x;
    output->y = y;
    
    return;
}

// debug function: draws each room's ID in center of room on board
// prints IDs in base NUMROOMS
void debug_drawRoomIds( char ** board, Room * rooms, int n ) {
    char idChar;
    Room * room;
    
    for( int i = 0; i < n; i++ ) {
	room = &rooms[i];
	
	if( i < 10 )
	    idChar = i + 48;
	else
	    idChar = i + 55;
	
	board[room->y][room->x] = idChar;
    }
    
}


int main() {

    /// DEBUG
    clock_t start, end;
    double cpu_time;
    start = clock();
    ///DEBUG
    
    system("clear");
    
    //init 100x100 board to 0s
    char ** board = (char **) malloc( sizeof( char * ) * TRUEWID );
    // + 1 char in row for EOL
    // each index of board points to a new row in trueBoard
    char * trueBoard = (char *) malloc( sizeof( char ) * (TRUESIZE + TRUEWID));
	
    int currIndex = 0;
    for( int i = 0; i < TRUEWID ; i++ ) {
	board[i] = &trueBoard[currIndex];
	for( int j = 0 ; j < TRUELEN ; j++ ) {
	    board[i][j] = ' ';
	}
	board[i][TRUELEN] = 0;
	currIndex += (TRUELEN + 1);
    }
    
    srand(time(NULL));
    Room * roomList = (Room *) malloc( sizeof( Room ) * NUMROOMS );
    //random gen rooms
    for( int i = 0; i < NUMROOMS ; i++ ) {
	
	int l = rand() % MAX_L;
	if( l < MIN_L )
	    l = MIN_L;
	roomList[i].l = l;
	
	int w = rand() % MAX_W;
	if( w < MIN_W )
	    w = MIN_W;

	roomList[i].w = w;
	
	int x = rand() % MAX_X;
	if( x < MIN_X )
	    x = MIN_X;
	roomList[i].x = x;
      
	int y = rand() % MAX_Y;
	if( y < MIN_Y )
	    y = MIN_Y;
	roomList[i].y = y;

	roomList[i].parent = 0;	
	roomList[i].child = 0;	
	roomList[i].id = i;
	
	_drawRoom( board, &roomList[i] );
	
    }

    collisionCheck( board, roomList, NUMROOMS );

    //debug_drawRoomIds( board, roomList, NUMROOMS );
    
    printBoard( board );
    
    int nRoomsPostGen = 0;

    
    for( int i = NUMROOMS - 1; i >= 0; i-- ) {
	if( roomList[i].parent == 0 ) {
	    nRoomsPostGen++;
	}
    }
    Room * roomsPostGen[nRoomsPostGen];
    nRoomsPostGen = 0;;
	    
    for( int i = NUMROOMS - 1; i >= 0; i-- ) {
	if( roomList[i].parent == 0 ) {
	    roomsPostGen[nRoomsPostGen] = &roomList[i];
	    nRoomsPostGen++;
	}
    }
    
    printf("Num rooms post generation and collision: %d\n", nRoomsPostGen );

    /// DEBUG
    end = clock();
    cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("cpu time: %f\n", cpu_time );
    /// DEBUG

    /*
      int nMainRooms = nRoomsPostGen / 2;
      int nSideRooms = nRoomsPostGen - nMainRooms;
      Room * mainRooms[ nMainRooms ];
      Room * sideRooms[ nSideRooms ];

      Room * connectedRooms = malloc( sizeof( Room * ) * nRoomsPostGen );
     */
    // Find the m largest rooms in r rooms
    // m = (n / 2 ) ?

    // delaunay triangulation of m rooms
    
    // MST of m rooms' edges

    // 1/2 of remaining edges activated

    // any other rooms intersecting edge of MST get added to tree

    // nearest neighbors of m rooms added to tree

    // not sure what to do for remaining rooms
}
