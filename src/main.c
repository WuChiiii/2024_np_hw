#include"config.h"

command_t *parser(char *commandStr);
int tot;
int cur;



int main(){
    tot=0;cur=0;
    char commandStr[256]={0};
    command_t *cmd=NULL;
    clear();
    do{
        cur++;
        printf("MYshell%% ");
        fgets(commandStr, sizeof(commandStr), stdin);
        // printf("123%s123\n",commandStr);
        commandStr[strlen(commandStr)-1]=0;
        if(strlen(commandStr) == 0){
            cur--;
            continue;
        }
        cmd= parser (commandStr);
        // printf("cmd->command = %s\n",cmd->command);
        // printf("cmd->parameter = %s\n", cmd->paramater);
        // execflag returns
        // -1 if there is no command,
        // 0 if it is a buildin command,
        // 1 if it is a simple command,
        // 2 if it is including a pipe,
        // 3 if it is including a num pipe
        if(cur == tot && cmd->exeflag != -1){
            //做pipe的第二部分
            // printf("bruno\n");
            donumpipe(cmd);
            cur=0;
            tot=0;
            continue;
        }
        switch(cmd->exeflag){
            case -1:
                cur--;
                printf("Unknown command: [%s].\n",cmd->command);
                break;
            case 0:
                exeBuildin(cmd);
                break;
            case 1:{
                exenonbuildin(cmd);
                break;
            }
            case 2:{
                //字串前處理
                if(exepipe(cmd)==-1){
                    cur--;
                }
                break;
            }
            case 3:{
                tot=pre_num_pipe(cmd);
                cur=0;
                // printf("%d %d",tot,cur);
                // printf("nuber pipe susscefully delievered\n");
                break;
            }
        }
    } while(strcmp(commandStr,"quit")!=0);
    return 0;
}
