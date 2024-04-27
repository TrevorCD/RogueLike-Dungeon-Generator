# RogueLike-Dungeon-Generator
WIP dungeon generation written in C99

Under GNU copy left

Should compile on linux with 'make'

Dependencies:
	- gcc

Know Issues:

     - Rooms that intersect with wall corners over 1 floor each, like this:

       	     	  	    #....
			    #....
			 ####....
			 ....####
			 ....#
			 ....#

       are counted as collisions, but are not traversable
