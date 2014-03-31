/*
 * Write your maze generator here. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "maze.h"

/*typedef struct {
    int visited;
    int xcord;
    int ycord;
    int east;//0 for nothing, 1 for wall, 2 for opening
    int west;//0 for nothing, 1 for wall, 2 for opening
    int south;//0 for nothing, 1 for wall, 2 for opening
    int north;//0 for nothing, 1 for wall, 2 for opening
} room;*/

void init (room maze[COLS][ROWS]);

int roomToNum (room r1);

void swap (char dir[4], int i, int r);

void shuffle(char dir[4]);

void dw(room maze[COLS][ROWS], int x, int y);

void toFile(room maze[COLS][ROWS], char *path);

int main(int argc, char **argv) {
    if(argc==2)
    {
	srand(time(NULL));
	room maze[COLS][ROWS];
	room (*ptr)[ROWS] = maze;
	init(ptr);
	dw(ptr, 0,0);
	toFile(ptr, argv[1]);
	return 0;
    }
    else
    {
	puts("NEED FILE PATH");
	return 1;
    }
}

void toFile(room maze[COLS][ROWS], char *path){
    int r, c;
    FILE *stream = fopen(path, "w");
    for(r = 0; r < ROWS; r++)
    {
	for(c = 0; c < COLS; c++)
	{
	    fprintf(stream, "%x", roomToNum(maze[c][r]));
	}
	fprintf(stream, "\n");
    }
    fclose(stream);
}

void dw (room maze[COLS][ROWS], int x, int y){
    maze[x][y].visited = 1;
    char dir[4] = {'e','w','n','s'};
    char (*cptr) = dir;
    shuffle(cptr);
    int i;
    for(i = 0; i<4; i++)
    {
	if(dir[i]=='e')
	{
	    if((x+1)>(COLS-1))
	    {
		maze[x][y].east = 1;
	    }
	    else if(maze[x+1][y].visited==0)
	    {
		maze[x][y].east = 2;
		dw(maze, x+1, y);
	    }
	    else
	    {
		if(maze[x+1][y].west!=0)
		{
		    maze[x][y].east = maze[x+1][y].west;
		}
		else
		{
		    maze[x][y].east = 1;
		}
	    }
	}
	else if(dir[i]=='w')
	{
	    if((x-1)<0)
	    {
		maze[x][y].west = 1;
	    }
	    else if(maze[x-1][y].visited==0)
	    {
		maze[x][y].west = 2;
		dw(maze, x-1, y);
	    }
	    else
	    {
		if(maze[x-1][y].east!=0)
		{
		    maze[x][y].west = maze[x-1][y].east;
		}
		else
		{
		    maze[x][y].west = 1;
		}
	    }
	}
	else if(dir[i]=='n')
	{
	    if((y-1)<0)
	    {
		maze[x][y].north = 1;
	    }
	    else if(maze[x][y-1].visited==0)
	    {
		maze[x][y].north = 2;
		dw(maze, x, y-1);
	    }
	    else
	    {
		if(maze[x][y-1].south!=0)
		{
		    maze[x][y].north = maze[x][y-1].south;

		}
		else
		{
		    maze[x][y].north = 1;
		}
	    }
	}
	else if(dir[i]=='s')
	{
	    if((y+1)>(ROWS-1))
	    {
		maze[x][y].south = 1;
	    }
	    else if(maze[x][y+1].visited==0)
	    {
		maze[x][y].south = 2;
		dw(maze, x, y+1);
	    }
	    else
	    {
		if(maze[x][y+1].north!=0)
		{
		    maze[x][y].south = maze[x][y+1].north;
		}
		else
		{
		    maze[x][y].south = 1;
		}
	    }
	}
	else
	{
	    puts("SOMETHING IS BREAKING");
	}
    }
}

void shuffle(char *dir){
    int i;
    for(i=0; i<4; i++)
    {
	int r = rand() % 4;
	swap(dir,i,r);
    }
}

void swap (char *dir, int i, int r){
    char tmp = dir[i];
    dir[i] = dir[r];
    dir[r] = tmp;
}

int roomToNum(room r1){
    int e,w,n,s;
    if(r1.east==1)
    {
	e=1;
    }
    else
    {
	e=0;
    }
    if(r1.west==1)
    {
	w=1;
    }
    else
    {
	w=0;
    }
    if(r1.north==1)
    {
	n=1;
    }
    else
    {
	n=0;
    }
    if(r1.south==1)
    {
	s=1;
    }
    else
    {
	s=0;
    }
    int dec = 8*e+4*w+2*s+n;
    return dec;
}

void init (room maze[COLS][ROWS]){
    int r, c;
    for(r=0; r<COLS; r++)
    {
	for(c=0; c<ROWS; c++)
	{
	    maze[r][c].visited = 0;
	    maze[r][c].xcord = r;
	    maze[r][c].ycord = c;
	    maze[r][c].east = 0;
	    maze[r][c].west = 0;
	    maze[r][c].south = 0;
	    maze[r][c].north = 0;
	}
    }
}

