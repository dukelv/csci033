/* 
 * Write your solver program here.  
 */
#include <stdlib.h>
#include <stdio.h>
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

void fromFile(char *path, room maze[COLS][ROWS]);

int dfs(room maze[COLS][ROWS], int xstart, int ystart, int xend, int yend, FILE *stream);

int main(int argc, char **argv) {
	if(argc==7)
	{
	    char *inpath = argv[1];
	    char *outpath = argv[2];
	    int xstart = atoi(argv[3]);
	    int ystart = atoi(argv[4]);
	    int xend = atoi(argv[5]);
	    int yend = atoi(argv[6]);
	    if((xstart>=0)&&(xstart<COLS)&&(ystart>=0)&&(ystart<ROWS)&&(xend>=0)&&(xend<COLS)&&(yend>=0)&&(yend<ROWS))
	    {
		room maze[COLS][ROWS];
		room (*ptr)[ROWS] = maze;
		fromFile(inpath, ptr);
		FILE *stream = fopen(outpath, "w");
		#ifdef FULL
		fprintf(stream,"%s\n","FULL");
		#else
		fprintf(stream,"%s\n","PRUNED");
		#endif
		int success = dfs(ptr, xstart, ystart, xend, yend, stream);
		if(success==1)
		{
		    puts("ERROR: COULD NOT NAVIGATE FROM START TO END");
		    fclose(stream);
		    return 1;
		}
		else
		{
		    fclose(stream);
		    return 0;
		}
	    }
	    else
	    {
		puts("ERROR: BOUNDS VIOLATED BY START/END COORDINATES");
		return 1;
	    }
	}
	else
	{
	    puts("ERROR: MISSING FILEPATH OR START/END COORDINATES");
	    return 1;
	}
}

int dfs(room maze[COLS][ROWS], int x, int y, int xend, int yend, FILE *stream){
    int i;
    #ifdef FULL
    fprintf(stream, "%d, %d\n", x, y);
    #endif
    if((x==xend)&&(y==yend))
    {
	#ifndef FULL
	fprintf(stream, "%d, %d\n", x, y);
	#endif
	return 0;
    }
    maze[x][y].visited=1;
    char dir[4] = {'e','w','n','s'};
    for(i = 0; i < 4; i++)
    {
	if((dir[i]=='e')&&((x+1)<COLS))
	{
	    if((maze[x][y].east==0)&&(maze[x+1][y].visited==0))
	    {
		if(dfs(maze,(x+1),y,xend,yend, stream)==0)
		{
		    #ifndef FULL
		    fprintf(stream, "%d, %d\n", x, y);
		    #endif
		    return 0;
		}
	    }
	}
	else if((dir[i]=='w')&&((x-1)>=0))
	{
	    if((maze[x][y].west==0)&&(maze[x-1][y].visited==0))
	    {
		if(dfs(maze,(x-1),y,xend,yend, stream)==0)
		{
		    #ifndef FULL
		    fprintf(stream, "%d, %d\n", x, y);
		    #endif
		    return 0;
		}
	    }
	}
	else if((dir[i]=='n')&&((y-1)>=0))
	{
	    if((maze[x][y].north==0)&&(maze[x][y-1].visited==0))
	    {
		if(dfs(maze,x,(y-1),xend,yend,stream)==0)
		{
		    #ifndef FULL
		    fprintf(stream, "%d, %d\n", x, y);
		    #endif
		    return 0;
		}
	    }
	}
	else if((dir[i]=='s')&&((y+1)<ROWS))
	{
	    if((maze[x][y].south==0)&&(maze[x][y+1].visited==0))
	    {
		if(dfs(maze,x,(y+1),xend,yend,stream)==0)
		{
		    #ifndef FULL
		    fprintf(stream, "%d, %d\n", x, y);
		    #endif
		    return 0;
		}
	    }
	}
    }
    #ifdef FULL
    fprintf(stream, "%d, %d\n", x, y);
    #endif
    return 1;
}

void fromFile(char *path, room maze[COLS][ROWS]){
    int r,c;
    FILE *stream = fopen(path, "r");
    for(r = 0; r < ROWS; r++)
    {
	for(c = 0; c < COLS; c++)
	{
	    maze[c][r].visited = 0;
	    maze[c][r].xcord = r;
	    maze[c][r].ycord = c;
	    unsigned int hex;
	    fscanf(stream, "%1x", &hex);
	    maze[c][r].east = hex/8;
	    hex = hex%8;
	    maze[c][r].west = hex/4;
	    hex = hex%4;
	    maze[c][r].south = hex/2;
	    hex = hex%2;
	    maze[c][r].north = hex;
	}
    }
    fclose(stream);
}