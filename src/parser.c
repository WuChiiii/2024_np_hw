#include "config.h"



char *buildCmd[nBUILDCMD]= {"CD","printenv","setenv","quit","help"};

void isBuildin(command_t *cmd){
    int i;
    for(int i=0;i<nBUILDCMD;i++){
        if(strcmp(cmd->command, buildCmd[i])==0){
            cmd->exeflag =0;
            return;
        }
    }

    return;
}

void isNonBuildin(command_t *cmd){//從./bin找出執行檔案與cmd->command一樣的
    char buff[1024];
    char path[1024];
    getcwd(buff, sizeof(buff));
    
    snprintf(path, sizeof(path), "%s/%s", buff, cmd->command);

    if (access(path, X_OK) == 0) {
        cmd->exeflag =1;
    } else {
        // printf("wrong：%s\n", path);
    }
}

void isPipe(command_t *cmd){
    if(cmd->paramater == NULL || strlen(cmd->paramater) < 2){
        return;
    }
    char *pos;
    char *check;
    char * tmp;
    char  tmpstr[100];
    int x;
    //check if it is pipe or a numpipe (if numpipe else pipe)
    pos = strchr(cmd->paramater, '|');
    if(pos != NULL){
        if(pos[1] == ' '){
            cmd->exeflag =2;
            return;
        }
        else{
            cmd->exeflag =3;
            // printf("it's a number pipe\n");
        }
    }
    //pipe needs to check if all the command can find in our /bin 
}

 command_t *parser(char * commandStr) {
    command_t *cmd = (command_t *)malloc(sizeof(command_t));
    sscanf(commandStr, "%s", cmd->command);
    int x = strlen(cmd->command)+1;
    sscanf(commandStr+x, "%[^\n]", cmd->paramater);
    cmd->exeflag = -1;
    isBuildin(cmd);
    isNonBuildin(cmd);
    isPipe(cmd);
    return cmd;
}


   
