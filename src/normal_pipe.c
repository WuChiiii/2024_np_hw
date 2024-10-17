#include "config.h"


void dochild(int fd[], char *commandArgv[],char *path, command_t *cmd){
    // printf("now is step5 \n");
    close(fd[READ_END]);
    dup2(fd[WRITE_END], STDOUT_FILENO);
    // printf("fff %s\n", cmd->command);
    if(strcmp(cmd->command, "printenv") == 0 || strcmp(cmd->command, "help") == 0){//pipe 配上 buildin
        exeBuildin(cmd);
        exit(EXIT_SUCCESS);
    }
    if (execv(path, commandArgv) < 0) {
        perror("execv child");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void doparent(int fd[], char * commandArgv[],char *path, command_t *cmd){
    // printf("now is step7 \n");
    switch(fork()) {
        case -1:
            perror("fork parent fork");
            exit(EXIT_FAILURE);
        case 0:
            //input from sibling 
            close(fd[WRITE_END]);
            dup2(fd[READ_END], STDIN_FILENO);
            close(fd[READ_END]);
            if (execv(path, commandArgv) < 0) {
                perror("execv parent");
                exit(EXIT_FAILURE);
            };
            exit(EXIT_SUCCESS);
        default:
            close(fd[READ_END]);
            close(fd[WRITE_END]);
            break;
    }
}


int exepipe(command_t *cmd){
    char *argVec[10];
    char *argVec1[10];//多一個cnt個二維陣列
    char path1[1024];
    char path2[1024];//path，2個都要snprintf();
    char buff[1024];
    char *tmpvar = malloc(strlen(cmd->paramater) + 1);
    char *tmp;
    char *start,*pos2,*command2;
    int cnt=1;
    int fd[2];
    getcwd(buff, sizeof(buff));
    strcpy(tmpvar,cmd->paramater);
    argVec[0] = malloc(strlen(cmd->command) + 1);//前半指令放command
    strcpy(argVec[0], cmd->command);
    command2 = strchr(tmpvar, '|');//後半指令
    command2+=2;
    
    // printf("now is step1 %s\n",tmpvar);
    while ((start = strsep(&tmpvar, " ")) != NULL) {
        if(start>command2) break;
        if (*start != '\0') {
            if(strcmp(start,"|") == 0) break;
            argVec[cnt] = malloc(strlen(start) + 1);
            strcpy(argVec[cnt], start);
            cnt++;
        }
    }
    // printf("now is step2 %s\n",argVec[cnt-1]);
    argVec[cnt] = NULL;
    snprintf(path1, sizeof(path1), "%s/%s", buff, cmd->command);//第一階段path+argv
    cnt = 0;
    // printf("now is step3 \n");
    while ((start = strsep(&command2, " ")) != NULL) {
        if (*start != '\0') {
            if(cnt == 0){
                snprintf(path2, sizeof(path2), "%s/%s", buff, start);//第一階段path+argv
            }
            argVec1[cnt] = malloc(strlen(start) + 1);
            strcpy(argVec1[cnt], start);
            cnt++;
        }
    }
    argVec1[cnt] = NULL;

    if (access(path2, X_OK) != 0) {
        printf("Unknown command: [%s].\n",argVec1[0]);
        
        return -1;
    }
    //fork()執行
    if ( (pipe(fd) == -1) )  {
        perror("pipe");
        return -1;
    }
    // printf("now is step4 \n");
    pid_t childPid;
    switch (fork()) {
        case -1:
            perror("fork\n");
            exit(EXIT_FAILURE);
        case 0:{
            dochild(fd,argVec,path1,cmd);
            break;
        }
        default:
            sleep(0.5);// Give child a chance to execute childpid大於0代表你是父親
            doparent(fd,argVec1,path2,cmd);
            break;
    }
        int status;
    while(1){//
        childPid = waitpid(-1, &status, WNOHANG);
        if (childPid == -1) {
            if (errno == ECHILD) {
                // printf("No more children - bye!\n");
                break;
            } else { // Some other (unexpected) error 
                perror("wait\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return 1;
}