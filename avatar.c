#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	      // read, write, close
#include <string.h>	
#include <stdbool.h>      
#include <netdb.h>	      // socket-related structures
#include <sys/socket.h> 
#include "amazing.h"

//create a new avatar struct (as defined in amazing.h)
Avatar *avatar_new(char* p, int aID, int nAv, int diff, char *host, int mPort, char* log, int sock)
{
    Avatar *avatar = malloc(sizeof(Avatar)); 
    avatar->program = p; 
    avatar->AvatarId = aID; 
    avatar->nAvatars = nAv;
    avatar->Difficulty = diff; 
    avatar->hostname = host;
    avatar->MazePort = mPort;
    avatar->logfilename = log;
    avatar->fd = sock; 
    avatar->endgame = false; 

    //if (avatar->program == NULL || avatar->AvatarId == NULL || avatar->nAvatars == NULL || avatar->Difficulty == NULL || avatar->hostname == NULL || avatar->MazePort == NULL || avatar->logfilename == NULL){
    if (avatar == NULL){
        return NULL; 
    }
    else {
        return avatar; 
    }
}

bool is_end_game(Avatar *avatar)
{
  if (avatar->endgame == true){
    //close the comm_sock
    printf("endgame error\n"); 
    return true; 
  }
  else {
    return false; 
  }
}
    
//helper functions to handle messages 
static bool error_msgs(AM_Message resp); 
static bool end_program(AM_Message resp); 
static bool maze_solved(AM_Message resp, Avatar *avatar); 

void* avatar_play(void *avatar_p)
{
  Avatar *avatar = avatar_p; 
  int a = 1; 
  void *p = (void*)&a; 
 // while (!is_end_game(avatar)){
  int port_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (port_sock < 0) {
    perror("opening socket");
    exit(9);
  }
  
  // Initialize the fields of the server address
  struct sockaddr_in server_address;                        // address of the server
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(avatar->MazePort);

  // connect the socket to the MazePort
  if (connect(port_sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    perror("connecting stream socket");
    exit(10);
  }
  printf("Connected to MazePort: %d\n", avatar->MazePort);
 
    FILE* fp = fopen(avatar->logfilename, "a");   //open the logfile 
    int bytes_read;       // #bytes read from socket
    AM_Message avatar_r; 
    memset(&avatar_r, 0, sizeof(avatar_r)); 
    avatar_r.type = htonl(AM_AVATAR_READY); 
    avatar_r.avatar_ready.AvatarId = htonl(avatar->AvatarId); //send AvatarId to server 
   
    // Sends avatar_ready to the server
    if (write(port_sock, (void*) &avatar_r, sizeof(avatar_r)) < 0){
      exit(5);
    } 
    AM_Message avatar_play; 
    int TurnID; 
    XYPos pos_array[avatar->nAvatars];
    // receives message back from server (after avatar_ready)
    //do {
      if ((bytes_read = read(port_sock, (void*) &avatar_play, sizeof(avatar_play))) < 0) {
        printf("err here\n"); 
        exit(5);
      } else { 
        //checks if it was successful 
        if(ntohl(avatar_play.type) == AM_NO_SUCH_AVATAR){     
          fprintf(stderr,"No such avatar\n"); 
        }
        else if (ntohl(avatar_play.type) == AM_AVATAR_TURN){   //gets the TurnID from the server and the XYPOS of each of the avatars 
            TurnID = ntohl(avatar_play.avatar_turn.TurnId); 
            printf("turnid: %d\n", TurnID); 
            //FILE* fp = fopen(avatar->logfilename, "a"); 
            for (int i = 0; i < avatar->nAvatars; i++){
              pos_array[i].x = ntohl(avatar_play.avatar_turn.Pos[i].x); //might not need ntohl
              pos_array[i].y = ntohl(avatar_play.avatar_turn.Pos[i].y);
            } 
            avatar->pos.x = ntohl(avatar_play.avatar_turn.Pos[avatar->AvatarId].x);  //update the avatar struct 
            avatar->pos.y = ntohl(avatar_play.avatar_turn.Pos[avatar->AvatarId].y);
            fprintf(fp,"Inserted avatar %d at %d,%d\n",TurnID, avatar->pos.x, avatar->pos.y); 
            printf("Inserted avatar %d at %d,%d\n",avatar->AvatarId, avatar->pos.x, avatar->pos.y); 
            //fprintf(fp, "avatar locations:\n"); // todo
        } 
      } 
  while (!is_end_game(avatar)){         //***** might need to move the while loop to where all the threads are recieving messages (with the updated turnID)
     AM_Message move_resp;
    //} while (bytes_read > 0);
    //if it's the avatar's turn to move
    if (avatar->AvatarId == TurnID){ 
      AM_Message avatar_m; 
      memset(&avatar_m, 0, sizeof(avatar_m));
      avatar_m.type = htonl(AM_AVATAR_MOVE);
      avatar_m.avatar_move.AvatarId = htonl(avatar->AvatarId);  
      /*
        insert algorithm function that determines the move 
        int direction = algorithm_function(); 
      */ 
      int direction = M_SOUTH; // **** just for testing ****
      avatar_m.avatar_move.Direction = htonl(direction); 

      if (write(port_sock, (void*) &avatar_m, sizeof(avatar_m)) < 0){  //send message to avatar 
        exit(5);
      }
      //AM_Message move_resp; 
        if ((bytes_read = read(port_sock, (void*) &move_resp, sizeof(move_resp))) < 0) {
          exit(5);
        } 
        else { 
          //checks if it was successful 
          if (!error_msgs(move_resp)){
            if (!end_program(move_resp) && !maze_solved(move_resp, avatar)){
              if (ntohl(avatar_play.type) == AM_AVATAR_TURN){   //gets the TurnID from the server and the XYPOS of each of the avatars  
                if (pos_array[TurnID].x == ntohl(move_resp.avatar_turn.Pos[TurnID].x) && pos_array[TurnID].y == ntohl(move_resp.avatar_turn.Pos[TurnID].y)){
                  // ******if the position of the avatar did not change, do something ***** 
                  //direction = M_EAST; 
                  //printf("avatar %d ran into wall at %d,%d\n"); 
                }
                pos_array[TurnID].x = ntohl(move_resp.avatar_turn.Pos[TurnID].x); //might not need ntohl
                pos_array[TurnID].y = ntohl(move_resp.avatar_turn.Pos[TurnID].y); //updates the x,y position of avatar
                avatar->pos.x = ntohl(move_resp.avatar_turn.Pos[TurnID].x);  //update the avatar struct 
                avatar->pos.y = ntohl(move_resp.avatar_turn.Pos[TurnID].y);
                printf("updated avatar %d at pos %d,%d\n",TurnID, avatar->pos.x, avatar->pos.y); 
                TurnID = ntohl(move_resp.avatar_turn.TurnId);  //the updated TurnID  
                printf("turnid: %d\n", TurnID); 
              }
            }
            else {
              printf("the game is over\n");
              avatar->endgame = true; 
              fclose(fp); 
              //printf("its all over\n"); 
              // todo: free memory 
              free(avatar); 
              close(port_sock);
              break; 
              //exit(0);
            }
          }
        }
    }
    else {  //if the avatarid != turnid, get the updated turnid 
         if ((bytes_read = read(port_sock, (void*) &move_resp, sizeof(move_resp))) < 0) {
          exit(5);
          } 
        else { 
          if (!error_msgs(move_resp)){
            if (!end_program(move_resp) && !maze_solved(move_resp, avatar)){
              if (ntohl(avatar_play.type) == AM_AVATAR_TURN){ 
                TurnID = ntohl(move_resp.avatar_turn.TurnId);  //the updated TurnID  
              } 
            }
          }
        }
    }
  }
    printf("returning p");
    return p; 
}
static bool error_msgs(AM_Message resp)
{
  if (ntohl(resp.type) == AM_SERVER_DISK_QUOTA){
    fprintf(stderr, "disk quota error\n"); 
  }
  if(ntohl(resp.type) == AM_NO_SUCH_AVATAR){
    fprintf(stderr, "no such avatar\n"); 
    return true; 
  }
  else if (ntohl(resp.type) == AM_UNKNOWN_MSG_TYPE){
    fprintf(stderr, "unknown message type: %d\n", ntohl(resp.unknown_msg_type.BadType)); 
    return true; 
  }
  else if (ntohl(resp.type) == AM_UNEXPECTED_MSG_TYPE){
    fprintf(stderr, "unexpected message type\n"); 
    return true; 
  }
  else if (ntohl(resp.type) == AM_AVATAR_OUT_OF_TURN){
    fprintf(stderr, "avatar out of turn\n"); 
    return true; 
  }
  return false; 
}

static bool end_program(AM_Message resp)
{
  if(ntohl(resp.type) == AM_SERVER_TIMEOUT){
    fprintf(stderr, "AM Server Timeout\n"); 
    return true; 
  }
  else if (ntohl(resp.type) == AM_SERVER_OUT_OF_MEM){
    fprintf(stderr, "AM Server out of memory\n"); 
    return true; 
  }
  else if(ntohl(resp.type) == AM_TOO_MANY_MOVES){
    fprintf(stderr, "too many moves\n"); 
    return true; 
  }
  return false; 
}

static bool maze_solved(AM_Message resp, Avatar *avatar)
{
  if(ntohl(resp.type) == AM_MAZE_SOLVED){
    printf("it's solved\n"); 
    FILE* fp = fopen(avatar->logfilename, "a"); //change this 
    fprintf(fp, "%d, %d, %d, %d\n", ntohl(resp.maze_solved.nAvatars), ntohl(resp.maze_solved.Difficulty), ntohl(resp.maze_solved.nMoves), ntohl(resp.maze_solved.Hash)); 
    fclose(fp); 
    // todo: need to write to the log for each move (up above) --> use "a"
    return true; 
  }
  return false; 
}