sbreslow
CSCI0330

MAZE README

My program is organized into three files: generator.c, solver.c, and maze.h.  The generator and solver files function exactly as described in the assignment, while the maze.h file simply contains my maze size constants (25,10) and the struct I use to represent each room within the maze.  My generator and solver do not really share any functions, so there was no need to create a function c-file.  The only noticeable difference in terms of how the maze is treated between my generator file and my solver file is that my solver treats uninitialized walls as 0s, walls as 1s, and openings as 2s while the generator reads walls as 1s and openings as 0s.  This inconsistency is handled by the roomToNum method of the generator, which adjusts from the 1-2 scale to the 0-1 scale before each room is printed to the file.  As far as I know, there are no bugs at all, and the program should function entirely to spec.