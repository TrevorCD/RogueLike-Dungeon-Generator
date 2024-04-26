/* Trevor Ben Calderwood
 * Roguelike tilemap generator prototype
 *
 * Under GNU copyleft
 *
 * TODO:
 *   - Finish implementing edge case logic for corner case in collision checker
 *           PRIORITY 1 ^^^
 *
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

#include "defns.h"

void _drawRoom( char ** board , Room * room ) {
    
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

void collisionCheck( char ** board, Room * rooms, int nRooms ) {

    char xCol;
    char yCol;
    
    Room * r1;
    Room * r2;

    int r1x, r1y, r1w, r1l, r2x, r2y, r2w, r2l;
    int xCorner, yCorner; // for literal edge case

    for( int i = 0; i < nRooms; i++ ) {
	
	r1 = &rooms[i];
	r1x = r1->x; r1y = r1->y; r1w = r1->w; r1l = r1->l;
	
	for( int j = i + 1; j < nRooms; j++ ) {
	    
	    r2 = &rooms[j];
	    r2x = r2->x; r2y = r2->y; r2w = r2->w; r2l = r2->l;
	    xCol = 0; yCol = 0; xCorner = 0; yCorner = 0;

	    // x collisions
	    if( r1x < r2x ) { //check RHS of r1 for collision
		if( r1x + r1w > r2x - r2w ) {
		    xCol = 1;
		    if( r1x + r1w == ( r2x - r2w1 ) + 1 )
			xCorner = 1; printf("xCorner!\n");
		}
	    }
	    else if( r1x > r2x ) { //case 1: check LHS of r1 for collision
		if( r1x - r1w < r2x + r2w ) {
		    xCol = 1;
		    if( (r1x - r1w) + 1 == r2x + r2w )
			xCorner = 1; printf("xCorner!\n");
		}
	    }
	    else //case 2: r1x == r2x
		xCol = 1;

	    
	    // y collisions
	    if( r1y < r2y ) { //check bottom of r1 for collision
		if( r1y + r1l > r2y - r2l ) {
		    yCol = 1;
		    if( r1y + r1l == (r2y - r2l ) + 1 )
			yCorner = 1; printf("yCorner!\n");
		}
	    }
	    else if( r1y > r2y ) { //case 1: check top of r1 for collision
		if( r1y - r1l < r2y + r2l ) {
		    yCol = 1;
		    if( ( r1y - r1l) + 1 == r2y + r2l )
			yCorner = 1; printf("yCorner!\n");
		}
	    }
	    else //case 2: r1y == r2y
		yCol = 1;

	    
	    if( yCol && xCol ) {
		if( xCorner && yCorner ) {
		    // IMPLEMENT HERE
		}
		listZipper( r1, r2 );
	    }
	}// end for j loop	
    }// end for i loop
}

double calcLength( double x1, double y1, double x2, double y2 ) {
    double a = x1 - x2;
    double b = y1 - y2;
    return sqrt( (a*a) + (b*b) );
}

// finds the center point of the circumcircle of the triangle the 3 input
// vertices create.
void circumCircleCenter( double x1, double y1, double x2, double y2, 
			 double x3, double y3, Coord * output ) {
    
    double x, y; // coordinates of circumcircle
    double invSlope12, invSlope23; // perpendicular slope to lines 1,2 and 2,3
    Coord midpoint12, midpoint23;
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

// Rudimentary calculation. Does not take into account overlap between rooms
// Summation of all room sizes in list
int findTrueRoomSize( Room * room ) {
    int size = room->w * room->l * 4;
    while( room->child ) {
	room = room->child;
	size += ( room->w * room->l * 4 );
    }
    return size;
}

void addEdge( Node * a, Node * b ) {
    
    Node * edge;

    int i = 0;
    while( ( edge = a->edges[i] ) != 0 ) i++;
    if( i >= NUMEDGES )
	printf("AAA FUCK! NO EDGES LEFT!");
    else
	a->edges[i] = b;
    i = 0;
    while( ( edge = b->edges[i] ) != 0 ) i++;
    if( i >= NUMEDGES )
	printf("AAA FUCK! NO EDGES LEFT!");
    else
	b->edges[i] = a;
}

void delaunayTriangulation( Node * nodes, int n,
			    Node * one, Node * two, Node * three ) {
    Coord midpoint;
    double radius;
    double distance;
    circumCircleCenter( (double) one->room->x, (double) one->room->y,
			(double) two->room->x, (double) two->room->y,
			(double) three->room->x, (double) three->room->y,
			&midpoint );
    radius = calcLength( (double) one->room->x, (double) one->room->y,
		       (double) midpoint.x, (double) midpoint.y );
    int id1 = one->room->id;
    int id2 = two->room->id;
    int id3 = three->room->id;

    Node * c;
    int id;
    int i = 0;
    for( ; i < n; i++ ) {
	c = &nodes[i];
	id = c->room->id;
	if( ( id != id1 ) && ( id != id2 ) && ( id != id3 ) ) {
	    distance = calcLength( (double) c->room->x, (double) c->room->y,
				 (double) midpoint.x, (double) midpoint.y );
	    if( distance > radius )
		break;
	}
    }
    if( i == n )
	return;
    // edging..?
    addEdge( one, c );
    addEdge( two, c );
    addEdge( three, c );
    
    // recurse
    // this is probably wrong vvv
    delaunayTriangulation( nodes, n, one, two, c     );
    delaunayTriangulation( nodes, n, one, c,   three );
    delaunayTriangulation( nodes, n, c,   two, three );
}

int main() {

    /// DEBUG
    clock_t start, end;
    double cpu_time;
    start = clock();
    /// DEBUG
    
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
    
    if( NUMROOMS <= 36 ) // 0 through 9 and A through Z -> 36 chars
	debug_drawRoomIds( board, roomList, NUMROOMS );
    
    printBoard( board );
    // count true number of rooms
    int nRoomsPostGen = 0;
    for( int i = NUMROOMS - 1; i >= 0; i-- ) {
	if( roomList[i].parent == 0 ) {
	    nRoomsPostGen++;
	}
    }
    Node * roomsPostGen = malloc( sizeof( Node ) * nRoomsPostGen );
    nRoomsPostGen = 0;
    // insert rooms and their sizes into node array
    for( int i = NUMROOMS - 1; i >= 0; i-- ) {
	if( roomList[i].parent == 0 ) {
	    roomsPostGen[nRoomsPostGen].room = &roomList[i];
	    roomsPostGen[nRoomsPostGen].size = findTrueRoomSize( &roomList[i] );
	    nRoomsPostGen++;
	}
    }
    printf("Num rooms post generation and collision: %d\n", nRoomsPostGen );
    
    int nMainRooms = nRoomsPostGen / 2;
    int nSideRooms = nRoomsPostGen - nMainRooms;
    
    // support node init
    // choose 3 points for a triangle that surrounds all points
    Node * supports = malloc( sizeof( Node ) * 3 );
    Room * facadeRooms = malloc( sizeof( Room ) * 3 );
    facadeRooms[0].id = -1;
    facadeRooms[0].x = ( 0 - ( TRUELEN / 2 ) );
    facadeRooms[0].y = 0;
    supports[0].room = &facadeRooms[0];
    supports[0].edges[0] = &supports[1];
    supports[0].edges[1] = &supports[2];

    facadeRooms[1].id = -1;
    facadeRooms[1].x = ( TRUELEN + ( TRUELEN / 2 ) );
    facadeRooms[1].y = 0;
    supports[1].room = &facadeRooms[1];
    supports[1].edges[0] = &supports[2];
    supports[1].edges[1] = &supports[0];

    facadeRooms[2].id = -1;
    facadeRooms[2].x = TRUELEN / 2;
    facadeRooms[2].y = TRUEWID * 2;
    supports[2].room = &facadeRooms[2];
    supports[2].edges[0] = &supports[0];
    supports[2].edges[1] = &supports[1];

    // delaunay triangulation of m rooms
    
    // MST of m rooms' edges

    // 1/2 of remaining edges activated

    // any other rooms intersecting edge of MST get added to tree

    // nearest neighbors of m rooms added to tree

    // not sure what to do for remaining rooms



    
    /// DEBUG
    end = clock();
    cpu_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("cpu time: %f\n", cpu_time );
    /// DEBUG
}
