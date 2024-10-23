#include "config.h"

int count_pipes(char *str){
    int count = 0;  // 記錄 '|' 出現的次數
    while (*str != '\0') {  // 迴圈直到字串結束 ('\0' 字元)
        if (*str == '|') {
            count++;
        }
        str++;  // 移動到下一個字元
    }
    return count;
}

int check_access(char * path){
    if (access(path, X_OK) != 0) {
        // printf("Unknown command: [%s].\n",argVec1[0]);
        return -1;
    }
    return 0;
}


int exepipe(command_t *cmd){
    char countpipe[100];//多一個cnt個二維陣列
    char buff[100];
    char *tmpvar = malloc(100);
    char *pos;
    if (tmpvar == NULL) {
        perror("malloc failed");
        return -1;
    }
    char pointer[100];
    int cntnum=0;
    int curr=0;
    int number_of_pipe;
    int **fd;
    int firstflag=1;
    // //先數有幾個 | 出現 然後每次loop就丟一個新的 argv以及path
    getcwd(buff, sizeof(buff));
    strcpy(countpipe,cmd->paramater);
    number_of_pipe = count_pipes(countpipe);
    number_of_pipe++;
    char ***argVec;//[2][10];
    char **path;

    argVec=malloc(sizeof(char **) * number_of_pipe);
    path=malloc(sizeof(char *)*number_of_pipe);
    fd=malloc(sizeof(int *) * number_of_pipe);
    for(int i=0;i<number_of_pipe;i++){
        argVec[i]=malloc(sizeof(char *) * 10);
    }
    
    // printf("%d\n",number_of_pipe);
    // printf("%s\n%s\n",cmd->command,cmd->paramater);
    strcpy(tmpvar,cmd->paramater);
    for(int i=0;i<number_of_pipe;i++){
        cntnum=0;
        if(i==0){//做paramater的
            path[i] = malloc(sizeof(char) * 100);
            snprintf(path[i],100, "%s/%s", buff, cmd->command);
            if(check_access(path[i])==-1){
                printf("Unknown command: [%s].\n",cmd->command);
                pos = strchr(cmd->paramater, '|');
                if(pos == NULL){
                    return 1;
                }
                pos+=2;
                sscanf(pos, "%s", cmd->command);
                pos= pos +(strlen(cmd->command)); 
                if(strlen(pos) != 0){
                    pos+1;
                    strcpy(cmd->paramater,pos);
                    strcpy(tmpvar,cmd->paramater);
                }
                else {
                    strcpy(cmd->paramater,"");
                    strcpy(tmpvar,"");
                }
                free(path[i]);
                i--;
                number_of_pipe--;
                continue;
                
            }
            argVec[i][0] = malloc(strlen(cmd->command) + 1);
            strcpy(argVec[i][0],cmd->command);
            if(strcmp(tmpvar,"")==0){
                cntnum++;
                argVec[i][cntnum]=NULL;
                break;
            }

            sscanf(tmpvar, "%s", pointer);
            cntnum++;
            while(strcmp(pointer,"|")!=0){
                argVec[i][cntnum] = malloc(strlen(pointer) + 1);
                strcpy(argVec[i][cntnum],pointer);
                cntnum++;
                tmpvar = tmpvar+strlen(pointer)+1;
                sscanf(tmpvar, "%s", pointer);
            }
            argVec[i][cntnum]=NULL;
            continue;
        }
        sscanf(tmpvar, "%s", pointer);
        if(strcmp(pointer,"|")==0){
            tmpvar+=2;
            sscanf(tmpvar, "%s", pointer);
        }
        while(strcmp(pointer,"|")!=0){
            tmpvar = tmpvar+strlen(pointer)+1;
            if(cntnum == 0){
                path[i] = malloc(sizeof(char) * 100);
                snprintf(path[i], 100 , "%s/%s", buff, pointer);
                if(check_access(path[i])==-1){
                    printf("Unknown command: [%s].\n",pointer);
                    pos = strchr(tmpvar, '|');
                    if(pos == NULL){
                        break;
                    }
                    pos+=2;
                    sscanf(pos, "%s", cmd->command);
                    pos= pos +(strlen(cmd->command));
                    if(strlen(pos) != 0){
                        strcpy(cmd->paramater,pos);
                        strcpy(tmpvar,pos);
                    }
                    else {
                        strcpy(cmd->paramater,"");
                        strcpy(tmpvar,"");
                    }
                    for(int n=0;n<i;n++){
                        free(path[n]);
                        int cnt=0;
                        while(argVec[n][cnt]!=NULL){
                            free(argVec[n][cnt]);
                            cnt++;
                        }
                    }
                    number_of_pipe=number_of_pipe-i-1;
                    i=-1;
                    // firstflag=0;
                    break;
                }
            }
            argVec[i][cntnum] = malloc(strlen(pointer) + 1);
            strcpy(argVec[i][cntnum],pointer);
            cntnum++;
            if(strcmp(tmpvar,"\0")==0){
                argVec[i][cntnum]=NULL;
                break;
            }
            sscanf(tmpvar, "%s", pointer);
        }
    }

    if(number_of_pipe == 1){
        exenonbuildin(cmd);
            return 1;
    }
    else if(number_of_pipe == 0){
        printf("Unknown command: [%s].\n",cmd->command);
        return 1;
    }
    
    for(int i = 0; i< number_of_pipe-1; i++){
        fd[i] = (int *)malloc(sizeof(int) *2);
        if ( (pipe(fd[i]) == -1) )  {
            perror("pipe");
            return -1;
        }
    }

    //************************for迴圈接pipe每次執行完才給下一個process執行
    pid_t childPid;
    int cur=0;
    int whom;
    for(int i=0;i<number_of_pipe;i++){
        whom = fork();
        if(whom == -1){
             perror("fork\n");
            exit(EXIT_FAILURE);
        }
        else if(whom == 0){
            if(i==0){
                close(fd[cur][READ_END]);
                dup2(fd[cur][WRITE_END], STDOUT_FILENO);
            }
            else if(i == number_of_pipe-1){
                dup2(fd[cur][READ_END], STDIN_FILENO);
                close(fd[cur][READ_END]);
            }
            else{//
                dup2(fd[cur][READ_END], STDIN_FILENO);
                dup2(fd[cur+1][WRITE_END], STDOUT_FILENO);
                close(fd[cur][READ_END]);
            }
            if(strcmp(argVec[i][0], "printenv") == 0 || strcmp(argVec[i][0], "help") == 0){//pipe 配上 buildin
                if(strcmp(argVec[i][0], "printenv") == 0)
                    printenv(argVec[i][1]);
                else
                    help();
                exit(EXIT_SUCCESS);
            }
            if (execv(path[i], argVec[i]) < 0) {
                perror("execv child");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        else{
            sleep(0.1);
            if(i==0){
                close(fd[cur][WRITE_END]);
            }
            else if(i == number_of_pipe-1){
                close(fd[cur][WRITE_END]);
                close(fd[cur][READ_END]);
            }
            else{
                close(fd[cur][READ_END]);
                close(fd[cur+1][WRITE_END]);
                cur++;
            }
        }
        
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

