#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>
// #include <readline/readline.h>
// #include <readline/history.h>


#define nBUILDCMD 5

#define CD 195048L
#define HELP 1886329613L
#define QUIT 1953245974L
#define SETENV 56176080006L
#define PRINTENV 63122149417L

#define clear() printf("\033[H\033[J")

#define WRITE_END 1
#define READ_END 0

typedef struct commandType {
    char command[100];
    char paramater[100];
    char exeflag;
} command_t;

typedef struct Node{
    char *path;
    char *env;
    struct Node *next;
    struct Node *prev;
} env_node;

typedef struct Linklist{
    env_node *head;
    int count;
} list;


