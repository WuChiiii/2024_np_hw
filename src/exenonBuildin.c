#include"config.h"

void exenonbuildin(command_t *cmd){
    pid_t childPid;
    switch (childPid = fork()) {
        case -1:
            perror("fork\n");
            exit(EXIT_FAILURE);
        case 0:{
            char *argVec[10];
            char path[1024];
            char buff[1024];
            char *parcpy = malloc(strlen(cmd->paramater) + 1);
            char *tmp = " ";
            char *pos;
            int cnt=1;
            strcpy(parcpy,cmd->paramater);
            argVec[0] = malloc(strlen(cmd->command) + 1);
            strcpy(argVec[0], cmd->command);
            //**************add  cmd->paramater to argVec, and set NULL to the last pos*************
            while ((pos = strsep(&parcpy, tmp)) != NULL) {
                // printf("beenhere %d\n",cnt);
                if (*pos != '\0') {
                    argVec[cnt] = malloc(strlen(pos) + 1);
                    strcpy(argVec[cnt], pos);
                    cnt++;
                }
            }
            argVec[cnt] = NULL;
            getcwd(buff, sizeof(buff));
            snprintf(path, sizeof(path), "%s/%s", buff, cmd->command);//cwd+/+command
            // printf("%s\n%s\n%s\n",buff, path, cmd->command);
            int retnum = execv(path,argVec);
            if(retnum == -1){
                printf("error %s %s\n",path, argVec[0]);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
            break;
        }
        default:
            sleep(0.5); /* Give child a chance to execute childpid大於0代表你是父親*/
            break;
        }
        /* Both parent and child come here */
        // printf("PID=%ld ", (long) getpid());
        int status;
    while(1){//這邊只會有一個小孩
        childPid = waitpid(-1, &status, WNOHANG);
        if (childPid == -1) {
            if (errno == ECHILD) {
                // printf("No more children - bye!\n");
                break;
            } else { /* Some other (unexpected) error */
                perror("wait\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}