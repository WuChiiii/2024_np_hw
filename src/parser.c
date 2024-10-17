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
    char  tmpstr[1024];
    int x;
    if(cmd->exeflag==-1){
        pos = strchr(cmd->paramater, '|');
        if(pos==NULL){//沒有pipe
            return;
        }
        printf("Unknown command: [%s].\n",cmd->command);
        if(pos[1]!=' ') return;
        pos+=2;
        check = strchr(pos, '|');
        if(check == NULL){//tp | ls
            sscanf(pos, "%s", cmd->command);
            x = strlen(cmd->command)+1;
            if(strlen(cmd->command)==strlen(pos)){
            strcpy(cmd->paramater,"");
            }
            else
                sscanf(pos+x, "%[^\n]", cmd->paramater);
            // printf("%s\n%s\n%s\n",pos,cmd->command,cmd->paramater);
            cmd->exeflag = -1;
            isBuildin(cmd);
            isNonBuildin(cmd);
            return;
        }
        // printf("help %s\n%s\n%s\n",pos,cmd->command,cmd->paramater); 
        sscanf(pos, "%s", cmd->command);
        x = strlen(cmd->command)+1;
        if(strlen(cmd->command)==strlen(pos)){
            strcpy(cmd->paramater,"");
        }
        sscanf(pos+x, "%[^\n]", cmd->paramater);
        isBuildin(cmd);
        isNonBuildin(cmd);
        // printf("help %s\n%s\n%s\n",pos,cmd->command,cmd->paramater);                            
        if(cmd->exeflag == -1){
            tmp = strchr(cmd->paramater, '|');
            tmp += 2;
            // printf("test bug:\n%s\n%s\n%s\nend\n",tmp,cmd->paramater,cmd->command);
            printf("Unknown command: [%s].\n",cmd->command);
            sscanf(tmp, "%s", cmd->command);
            x = strlen(cmd->command)+1;
            if(strlen(cmd->command)==strlen(tmp)){
                strcpy(cmd->paramater,"");
            }else{
                sscanf(tmp+x, "%[^\n]", cmd->paramater);
            }
            isBuildin(cmd);
            isNonBuildin(cmd);
        }
        else
            cmd->exeflag =2;
        return;
        
    }
    // printf("it's inout\n");
    
    //check if it is pipe or a numpipe (if numpipe else pipe)
    pos = strchr(cmd->paramater, '|');
    if(pos != NULL){
        if(pos[1] == ' '){
            cmd->exeflag =2;
            pos+=2;
            check = strchr(pos, '|');
            if(check == NULL){
                return;
            }
            check+=2;
            char buff[1024];
            char path[1024];
            sscanf(pos, "%s", tmpstr);//popo(第二層command)
            sscanf(check, "%s", cmd->command);
            x = strlen(cmd->command)+1;
            if(strlen(cmd->command)==strlen(check)){
                strcpy(cmd->paramater,"");
            }
            else
                sscanf(check+x, "%[^\n]", cmd->paramater);
            getcwd(buff, sizeof(buff));
            snprintf(path, sizeof(path), "%s/%s", buff, cmd->command);

            if (access(path, X_OK) == 0) {
                printf("Unknown command: [%s].\n",tmpstr);
                cmd->exeflag =1;
            } else {
                cmd->exeflag =-1;
            }
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


   
