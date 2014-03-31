#ifndef MAZE_H
#define MAZE_H
#define ROWS 10
#define COLS 25
typedef struct {
    int visited;
    int xcord;
    int ycord;
    int east;//0 for nothing, 1 for wall, 2 for opening
    int west;//0 for nothing, 1 for wall, 2 for opening
    int south;//0 for nothing, 1 for wall, 2 for opening
    int north;//0 for nothing, 1 for wall, 2 for opening
} room;

#endif