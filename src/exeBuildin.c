#include"config.h"

list * envpair=NULL;
char init_path[100];
char init_evn[100];

const unsigned long hash(const *str){
    unsigned long hash = 5381;
    int c;

    while((c= *str++))
        hash = ((hash << 5) +hash )+c;
        // printf("%lu\n",hash);
    return hash;
}

void init_list(){
    strcpy(init_path, "PATH\0");
    strcpy(init_evn, "bin:.\0");
    envpair = (list *)malloc(sizeof(list));
    envpair->head = (env_node *)malloc(sizeof(env_node));
    envpair->head->prev = NULL;
    envpair->head->next = NULL;
    envpair->head->env = &init_evn;
    envpair->head->path = &init_path;
    envpair->count = 1;
}

void quit(){
    printf("\nGodbye\n");
    exit(EXIT_SUCCESS);
}

void changeDir(char *dir){
    chdir(dir);
}

void mysetenv(char *arg){//set完成
    char *space_pos = strchr(arg, ' ');
    if (space_pos == NULL) {
        printf("error env input\n");
        return;
    }
    *space_pos = '\0';
    char *var = arg;
    char *value = space_pos + 1;
    if(value == NULL) return;
    if(envpair == NULL){//初始化linklist
        init_list();
        // printf("shouldn't been here");
    }
    env_node *tmpnode = envpair->head;
    while (tmpnode!=NULL){//找到最後一個節點
        if(strcmp(tmpnode->path,var) == 0){//設定出現過的path
            tmpnode->env = value;
            // printf("shouldn't been here");
            return;
        }
        if(tmpnode->next == NULL){//最後一個
            env_node *newnode = (env_node *)malloc(sizeof(env_node));
            newnode->path = var;
            newnode->env = value;
            newnode->next = NULL;
            newnode->prev = tmpnode;
            tmpnode->next = newnode;
        }
        tmpnode = tmpnode->next;
    }
}

void printenv(char *arg){
    if(envpair == NULL){//初始化linklist
       init_list();
    }
    if (arg == NULL || strlen(arg) == 0) {// 列出所有環境變數
        env_node *tmpnode = envpair->head;
        while(tmpnode != NULL){
            printf("%s\n", tmpnode->env);
            tmpnode = tmpnode->next;
        }
    } 
    else {//找到是哪個path
        env_node *tmpnode = envpair->head;
        while(tmpnode != NULL){
            if(strcmp(tmpnode->path,arg) == 0){
                printf("%s\n", tmpnode->env);
                return;
            }
            tmpnode = tmpnode->next;
        }
    }

}
void help(){
    puts("\n***WELCOME TO SHELL HELP"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>setenv"
        "\n>printenv"
        "\n>quit"
        "\n>all other general commands available in bin"
        "\n>normal pipe handling"
        "\n>number pipe handling");
}

void exeBuildin(command_t *cmd){
    char tmpstr[1024];
    char tmp[1024];
    char * start;
    char * para_read;
    strcpy(tmp, cmd->paramater);
    para_read=tmp;
    if(cmd->paramater != NULL){
        if((start = strsep(&para_read," "))!=NULL){
            printf("%s\n",start);
            if(strcmp(start, "|") != 0){
                strcpy(tmpstr, start);
            }
            else{
                strcpy(tmpstr, "");
            }
        }
    }
    switch(hash(cmd->command)){
        case CD:
            changeDir(tmpstr);
            break;
        case HELP:
            help();
            break;
        case PRINTENV:
            printenv(tmpstr);
            break;
        case SETENV:
            mysetenv(cmd->paramater);
            break;
        case QUIT:
            quit();
            break;
        default:
            // printf("%lu vs %s\n",hash(cmd->command),PRINTENV);
            printf("error cmd\n");
            break;
    }
}