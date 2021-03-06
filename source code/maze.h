/* 
 * maze.h - header file for CS50 maze module
 * 
 * Sebastian Saker, March 2020
 */

#ifndef __MAZE_H
#define __MAZE_H

#include <stdio.h>
#include <stdbool.h>

/**************** global types ****************/
typedef struct maze maze_t; // opaque to users of the module
typedef struct node node_t;
/**************** functions ****************/

/**************** maze_new ****************/
/* Create a new maze structure.
 *
 * We return:
 *   pointer to a new maze; NULL if error. 
 * Caller is responsible for:
 *   later calling maze_delete();
 */
maze_t *maze_new(const int height, const int width);

/**************** get_node ****************/
/* Get a node from the maze structure.
 * We need:
 *   -maze: the maze struct to get a node from
 *   -x: x coordinate of the node 
 *   -y: y coordinate of the node
 * We return:
 *   pointer to the specified node, NULL on any error
 */
node_t *get_node(maze_t *maze, int x, int y); 

/**************** get_avatar ****************/
//returns an integer that represents the avatar's ID of the avatar currently on the node
//if there is no avatar, return -1
int get_avatar(node_t *node);

/**************** set_avatar ****************/
//set an integer on a node that represents the avatar's ID of the avatar currently on the node
void set_avatar(node_t *node, int avatar);

/**************** get_height ****************/
//returns an integer that represents the height of the maze
int get_height(maze_t *maze);

/**************** get_width ****************/
//returns an integer that represents the width of the maze
int get_width(maze_t *maze);

/**************** get_node_x ****************/
//returns an integer that represents the X coordinate of a node
int get_node_x(node_t *node);

/**************** get_node_y ****************/
//returns an integer that represents the Y coordinate of a node
int get_node_y(node_t *node);

/**************** get_node_index ***********/
int get_node_index(maze_t *maze, node_t *node);

/***************** get_neighbor ************/
node_t *get_neighbor(maze_t *maze, node_t *node, int dir);

/***************** get_L1_distance **********/
int get_L1_distance(node_t *node1, node_t *node2);

/**************** check_node ****************/
int check_node_dir(maze_t *maze, node_t *node, int dir); //return an int based on what the node is pointing to
//return 0 if NULL
//return 1 if node is pointing to unknown placeholder node
//return 2 if node is pointing to wall placeholder node
//return 3 if node has a connection to another node that we already discovered/inferred

/**************** set_wall ****************/
void set_wall(node_t *node, maze_t *maze, const int dir); //dir will indicate which direction we want to place a wall

/**************** set_connection ****************/
void set_connection(node_t *node, maze_t *maze, const int dir); //dir will indicate which direction we want to me a connection                                                              
/**************** maze_delete ****************/
/* Delete the whole maze.
 *
 * Caller provides:
 *   a valid pointer to maze.
 * We do:
 *   we ignore NULL ctrs.
 *   we free all memory we allocate for this maze.
 */
void maze_delete(maze_t *maze);

#endif // __MAZE_H
