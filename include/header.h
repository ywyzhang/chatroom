#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <syslog.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include "sqlite3.h"
#include <semaphore.h>


#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/un.h>



typedef struct user//用户信息结构体
{
    int cid;
    char name[16];
    char code[32];
    int online;
    struct user * next;
}user_t;

user_t *List_Init(void);

user_t *Request_Node(int client_id);
void assign_node(char name[],char code[],user_t*node);
int Insert_Node(user_t *head, user_t *node);

void Show_List(user_t *head);