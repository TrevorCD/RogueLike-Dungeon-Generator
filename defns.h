/* Trevor Ben Calderwood
 * Definitions for roguelike tilemap generator prototype
 *
 * Under GNU copyleft
 *
 */
#ifndef DEFNS_H
#define DEFNS_H
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

typedef struct coordinate {

    double x;
    double y;
    
} Coord;

typedef struct roomNodeContainer {

    Room * room; // support nodes have facade rooms w/ id -1
    int size;

} Node;

typedef struct triangle {
    Node * nodes[3];
    Coord center;
    double radius;
} Tri;

#endif // DEFNS_H
