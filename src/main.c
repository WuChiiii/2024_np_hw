#include"config.h"

command_t *parser(char *commandStr);
int tot;
int cur;

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

void donumpipe(command_t *cmd){
    int input_fds = open("./data.txt", O_RDONLY);
    // dup2(output_fds, STDOUT_FILENO);
    if(cmd->exeflag == 1){//接一個nonbuildin
        pid_t childPid;
        switch (childPid = fork()) {
            case -1:
                perror("fork\n");
                exit(EXIT_FAILURE);
            case 0:{
                dup2(input_fds, STDIN_FILENO);
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
                while ((pos = strsep(&parcpy, tmp)) != NULL) {
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

                //pipe接起來
                // printf("123");
                int retnum = execv(path,argVec);
                if(retnum == -1){
                    printf("error ls\n");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_SUCCESS);
                break;
            }
            default:
                sleep(0.5); /* Give child a chance to execute childpid大於0代表你是父親*/
            break;
        }
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
    else if(cmd->exeflag == 2){//接pipe
        //字串前處理
                char *argVec[10];
                char *argVec1[10];//多一個cnt個二維陣列
                char path1[1024];
                char path2[1024];//path，2個都要snprintf();
                char buff[1024];
                char buffer[1024];
                char *tmpvar = malloc(strlen(cmd->paramater) + 1);
                char *tmp = " ";
                char *start,*pos2,*command2;
                int cnt=1;
                int fd[2];
                int fd1[2];
                ssize_t bytesRead;
                strcpy(tmpvar,cmd->paramater);
                argVec[0] = malloc(strlen(cmd->command) + 1);//前半指令放command
                strcpy(argVec[0], cmd->command);
                command2 = strchr(tmpvar, '|');//後半指令
                command2+=2;
                // printf("now is step1 %s\n",tmpvar);
                while ((start = strsep(&tmpvar, tmp)) != NULL) {
                    if(start>command2) break;
                    if (*start != '\0') {
                        if(strcmp(start,"|") == 0) break;
                        // printf("test bug %s\n",start);
                        argVec[cnt] = malloc(strlen(start) + 1);
                        strcpy(argVec[cnt], start);
                        cnt++;
                    }
                }
                // printf("now is step2 %s\n",argVec[cnt-1]);
                argVec[cnt] = NULL;
                getcwd(buff, sizeof(buff));
                snprintf(path1, sizeof(path1), "%s/%s", buff, cmd->command);//第一階段path+argv
                cnt = 0;
                // printf("now is step3 \n");
                while ((start = strsep(&command2, tmp)) != NULL) {
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
                //fork()執行
                if ( (pipe(fd) == -1) )  {
                    perror("pipe");
                    return;
                }
                if ( (pipe(fd1) == -1) )  {
                    perror("pipe1");
                    return;
                }
                pid_t childPid;
                while ((bytesRead = read(input_fds, buffer, sizeof(buffer))) > 0) {
                        write(fd[WRITE_END], buffer, bytesRead);
                }
                close(fd[WRITE_END]);
                switch (fork()) {
                    case -1:
                        perror("fork\n");
                        exit(EXIT_FAILURE);
                    case 0:{
                        close(fd[WRITE_END]);
                        dup2(fd[READ_END], STDIN_FILENO);
                        close(fd[READ_END]);
                        dup2(fd1[WRITE_END], STDOUT_FILENO);
                        if(strcmp(cmd->command, "printenv") == 0 || strcmp(cmd->command, "help") == 0){//pipe 配上 buildin
                            exeBuildin(cmd);
                            exit(EXIT_SUCCESS);
                        }
                        if (execv(path1, argVec) < 0) {
                            perror("execv child");
                            exit(EXIT_FAILURE);
                        }
                        exit(EXIT_SUCCESS);
                        break;
                    }
                    default:
                        close(fd[WRITE_END]);
                        close(fd[READ_END]);
                        close(fd1[WRITE_END]);
                        
                        switch(fork()) {
                            case -1:
                                perror("fork parent fork");
                                exit(EXIT_FAILURE);
                                break;
                            case 0:
                                dup2(fd1[READ_END], STDIN_FILENO);
                                close(fd1[READ_END]);
                                if (execv(path2, argVec1) < 0) {
                                    perror("execv parent");
                                    exit(EXIT_FAILURE);
                                };
                                exit(EXIT_SUCCESS);
                                break;
                            default:
                                close(fd[READ_END]);
                                close(fd[WRITE_END]);
                                close(fd1[WRITE_END]);
                                close(fd1[READ_END]);
                            break;
                        }
                        break;
                }
                    int status;
                while(1){//\printf("No more children - bye!\n");
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

    }
}


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
                break;
            }
            case 2:{
                //字串前處理
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
                    cur--;
                    break;
                }
                //fork()執行
                if ( (pipe(fd) == -1) )  {
                    perror("pipe");
                    break;
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
                break;
            }
            case 3:{
                //preprocess
                char *argVec[10];
                char path1[1024];
                char buff[1024];
                char buffer[1024];
                char *tmpvar = malloc(strlen(cmd->paramater) + 1);
                char *tmp = " ";
                char *start,*command2;
                int cnt=1;
                int fd[2];
                strcpy(tmpvar,cmd->paramater);
                argVec[0] = malloc(strlen(cmd->command) + 1);//前半指令放command
                strcpy(argVec[0], cmd->command);
                command2 = strchr(tmpvar, '|');//後半指令
                command2+=1;
                tot=command2[0]-'0';//
                // printf("now is step1 %d\n",tot);
                while ((start = strsep(&tmpvar, tmp)) != NULL) {
                    if(start>command2) break;
                    if (*start != '\0') {
                        if(start[0] == '|') break;
                        argVec[cnt] = malloc(strlen(start) + 1);
                        strcpy(argVec[cnt], start);
                        cnt++;
                    }
                }
                // printf("now is step2 %s\n",argVec[cnt-1]);
                argVec[cnt] = NULL;
                getcwd(buff, sizeof(buff));
                snprintf(path1, sizeof(path1), "%s/%s", buff, cmd->command);
                if ( (pipe(fd) == -1) )  {
                    perror("pipe");
                }
                // printf("now is step2.5 \n");
                int output_fds = open("./data.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                
                pid_t childPid;
                switch (fork()) {
                    case -1:
                        perror("fork\n");
                        exit(EXIT_FAILURE);
                    case 0:{//改丟file 但要先open
                        dup2(fd[WRITE_END], STDOUT_FILENO);
                        if(strcmp(cmd->command, "printenv") == 0 || strcmp(cmd->command, "help") == 0){//pipe 配上 buildin
                            exeBuildin(cmd);
                            exit(EXIT_SUCCESS);
                        }
                        if (execv(path1, argVec) < 0) {
                            perror("execv child");
                            exit(EXIT_FAILURE);
                        }
                        // printf("now is step6 \n");
                        exit(EXIT_SUCCESS);
                        break;
                    }
                    default:
                        sleep(0.5);
                        close(fd[WRITE_END]); // Close unused write end

                        // Read from pipe and write to file
                        char buffer[1024];
                        ssize_t bytesRead;

                        // Now we can read from the pipe and write to data.txt
                        while ((bytesRead = read(fd[READ_END], buffer, sizeof(buffer))) > 0) {
                            write(output_fds, buffer, bytesRead);
                        }

                        close(fd[READ_END]); // Close read end
                        close(output_fds);
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
                cur=0;
                // printf("%d %d",tot,cur);
                // printf("nuber pipe susscefully delievered\n");
                break;
            }
        }
    } while(strcmp(commandStr,"quit")!=0);
    return 0;
}
