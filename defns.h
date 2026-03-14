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
#define N_SUBROOMS 25    /* total number of subrooms per level */
#define MAX_SUBROOMS 10  /* maximum number of subrooms per room */

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


typedef struct subroom {

    int l;         // length across x dimension
    int w;         // width across y dimension
    int x;         // horizontal dimension, grows right
    int y;         // vertical dimension, grows down
    
    char id;       //initial gen room id, index of roomList in main
    
} SubRoom;

typedef struct room {

	int x;
	int y;
	int nSubRooms;
	
	SubRoom * subRooms;
	
	char id;
} Room;

typedef struct level {

	int levelId;
	int nRooms;
	SubRoom * subRoomList;
	Room * roomList;
	char ** board;
	
} Level;
	
#endif // DEFNS_H
