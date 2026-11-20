#include "header.h"
#include "cJSON.h"

#define CMD_CREATE "CREATE TABLE IF NOT EXISTS userinfo (ID INTEGER PRIMARY KEY NOT NULL, NAME CHAR(30),CODE CHAR(30));"
#define CMD_SELECT "select *from userinfo;"
#define frs file_recv_size
#define Date long

int keyid;
user_t *head;



int number = 0;//在线人数
int flg = 0; //私聊标志,置1为私聊

sqlite3 *db=NULL;
int rc = -1;

//数据库遍历验证用户名与密码是否正确
int just_select(sqlite3*dq,char name[],char code[],user_t *head,user_t *node)
{
    int row;//行
    int column;//列
    char **result=NULL;
    if(sqlite3_get_table(dq,CMD_SELECT,&result,&row,&column,NULL)!=0)
    {
        printf("错误信息:%s",sqlite3_errmsg(dq));
        return -1;
    }
    printf("row=%d,column=%d\n",row,column);

    int j = (row+1)*3;
    //找到用户
    for (int i = 4; i < j; i+=3)
    {
        //printf("%s\n", result[i]);
        if(strcmp(result[i],name) == 0)
        {
            if(strcmp(result[i+1],code) == 0)
            {
                printf("登陆成功\n");
                printf("number = %d\n",number );
                //printf("传入的node->cid：%d\n",node->cid );
                //登录成功将用户存入在线用户链表中
                assign_node(name,code,node);
                Show_List(head);
                printf("%s上线了\n", name);

 
                return 1;
            }
        }
    }
    printf("登陆失败\n");
    return -1;
}
//数据库查看数据或遍历数据
int do_select(sqlite3*dq)
{
    printf("查询结果如下：\n"); 
    int row;//行 
    int column;//列
    char **result=NULL;
    if(sqlite3_get_table(dq,CMD_SELECT,&result,&row,&column,NULL)!=0)
    {
        printf("错误信息:%s",sqlite3_errmsg(dq));
        return -1;
    }
    printf("row=%d,column=%d\n",row,column);
    int num = column;
    int user = row;
    printf("user=%d\n",user);
    for(int i = 0;i < row;i++)
    {
        for (int j = 0; j < column; j++)
        {
            printf("%s\t%s\t",result[j],result[num]);
            num++;
        }
        printf("\n");
    }
    return user+1;
}
//链表部分 初始化一个头结点
user_t *List_Init(void)
{
    user_t *head = malloc(sizeof(user_t));
    if(head == NULL)
    return NULL;
    head->next = NULL;
    return head;
}
//申请一个节点
user_t *Request_Node(int client_id)
{
    user_t *node = malloc(sizeof(user_t));
    if(node == NULL)
        return NULL;
    //printf("申请到的节点的套接字为%d\n",client_id );
    node->cid = client_id;
    //printf("存入链表的套接字为%d\n", node->cid);
    node->online = 0;
    node->next    = NULL;   
    return node;
}

//登录完给节点赋值
void assign_node(char name[],char code[],user_t*node)
{
    //printf("传入的node->cid(为了寻找该套接字的节点)：%d\n",node->cid );
    bzero(node->name, sizeof(node->name)); 
    bzero(node->code, sizeof(node->code)); 
    //node->cid = cid;         //保存id
    strcpy(node->code ,code);     //保存code
    strcpy(node->name ,name);     //保存用户名
    node->online = 1;


}
//插入节点
int Insert_Node(user_t *head, user_t *node)
{
    user_t *p = NULL;
    for(p = head; p->next != NULL; p = p->next);
    p->next = node;
}
//遍历链表
void Show_List(user_t *head)
{
    

    char user_data[100];
    //sprintf(user_data, "\nID\tname\tcode\t\tonline\n");
    //send(connfd, user_data, strlen(user_data), 0);
    user_t *p = NULL;
    for(p = head->next; p != NULL; p = p->next)
    {
        printf("id:%d name:%s code:%s online:%d\n",p->cid,p->name,p->code,p->online);
    }
}

int Find_user(user_t *head,char *buf)//通过昵称查找用户
{
    user_t *p = NULL;
    printf("%s is finding\n", buf);
    for(p = head->next; p != NULL; p = p->next)
    {
        if(strcmp(buf, p->name) == 0)
        {
            return p->cid;
            
        }   
    }
    printf("NOT find\n");
}

user_t *Find_iduser(user_t *head,int myid)//通过ID查找用户
{
    user_t *p = NULL;
    printf("%d is finding\n", myid);
    for(p = head->next; p != NULL; p = p->next)
    {
        if(myid - p->cid == 0)
        {
            return p;
        }   
    }
    return p;
    printf("not find\n");
}

void mass_send(char *sendbuf) // 转发给所有人
{
    user_t *p = NULL;
    for(p = head; p != NULL; p = p->next)
    {
        if(p->cid > 0)
        {
            send(p->cid,sendbuf,strlen(sendbuf),0);//发送包装好的的信息
        }   
    }
}

//你要把数据添加到哪一个表当中
void sqlite_user_add(char *table,char* name,char* code1)
{
    char cmd[128];

    memset(cmd, 0 , sizeof(cmd));

    keyid = do_select(db);
  

    printf("keyid:%d\n", keyid);
    sprintf(cmd,"INSERT INTO %s VALUES(%d,'%s','%s');",table,keyid,name ,code1);

    printf("添加的CMD %s\n", cmd);

    sqlite3_exec(db, cmd, NULL, NULL, NULL);
}


int recv_file(int connfd)
{
    int fd = open("accept.txt", O_RDWR|O_TRUNC|O_CREAT, 0777);
    if(fd == -1)
    {
        perror("open file error\n");
        send(connfd, "recv error", 10, 0);
        return -1;
    }
    send(connfd, "准备接收\n", 10, 0);
    char buf[1024];
    int c = 0;
    int ret = 0;
    Date file_size = 0;
    Date frs = 0;
    //接收 
    bzero(buf, sizeof(buf));
    //总字节数
    recv(connfd, buf, sizeof(buf), 0);
    file_size = atol(buf);
    c = file_size/10;
    printf("需要接收 %ld bytes\n", file_size);
    while(1)
    {
        bzero(buf, sizeof(buf));

        ret=recv(connfd, buf, sizeof(buf), 0);
        
        write(fd, buf, ret);
        frs += ret;
        printf("已接收 %ld 字节 \n", frs);
        if(frs == file_size)
        {
            printf("== 100%c ==\n", 37);
            break;
        }
        
    }
    printf("总共接收 %ld bytes\n", frs);

    return 0;
}

void parse(char *res, int *ok, int *len)
{
    char *retcode = res + strlen("HTTP/1.x ");

    switch(atoi(retcode))
    {
    case 200 ... 299:
            *ok = 1;
            printf("查询成功\n");
            break;

    case 400 ... 499:
            *ok = 0;
            printf("客户端错误\n");
            exit(0);

    case 500 ... 599:
            *ok = 0;
            printf("服务端错误\n");
            exit(0);
    }

    char *p;
    if(p = strstr(res, "Content-Length: "))
    {
        *len = atoi(p + strlen("Content-Length: "));
    }
}

int weather(user_t * node)
{
    // 利用DNS服务查询指定域名的IP
    struct hostent *he = gethostbyname("api.seniverse.com");
    if(he == NULL)
    {
        perror("DNS查询失败");
        exit(0);
    }

    printf("IP: %s\n", inet_ntoa(*(struct in_addr*)((he->h_addr_list)[0])));

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;
    addr.sin_addr   = *(struct in_addr*)((he->h_addr_list)[0]);
    addr.sin_port   = htons(80);

    // 创建TCP套接字(因为HTTP是基于TCP的)，并发起连接
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(fd, (struct sockaddr *)&addr, len) == 0)
    {
        printf("连接服务器成功！\n");
    }

    // 准备好HTTP的请求报文
    char *s = "GET /v3/weather/now.json?key=SAewqnjWlC7dvMLfL&location=yichang&language=zh-Hans&unit=c HTTP/1.1\r\n"
                    "Host:api.seniverse.com\r\n\r\n";
    write(fd, s, strlen(s));


    // 接收对方的响应头部
    char res[1024];
    int total = 0;
    while(1)
    {
        int n = read(fd, res+total, 1);

        if(n <= 0)
        {
            perror("读取HTTP头部失败");
            exit(0);
        }

        total += n;

        if(strstr(res, "\r\n\r\n"))
            break;
    }


    // 分析响应码，并获取正文的长度
    int ok, jsonlen=0;
    parse(res, &ok, &jsonlen);
    if(!ok || jsonlen == 0)
    {
        exit(0);
    }

    
    //接收服务器返回的数据信息
    char read_server_data[10000] = {0};
    recv(fd, read_server_data, 10000,0);

    //打印应答信息
    //printf("读取到的信息为:%s\n",read_server_data);
     cJSON *root = cJSON_Parse(read_server_data);
    if (root == NULL) 
    {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return 1;
    }

    // 提取并打印有用的信息
    cJSON *results = cJSON_GetObjectItem(root, "results");
    if (results == NULL || cJSON_GetArraySize(results) == 0) 
    {
        printf("No results found in JSON\n");
        cJSON_Delete(root);
        return 1;
    }

    cJSON *location = cJSON_GetObjectItem(cJSON_GetArrayItem(results, 0), "location");
    cJSON *now = cJSON_GetObjectItem(cJSON_GetArrayItem(results, 0), "now");
    cJSON *last_update = cJSON_GetObjectItem(cJSON_GetArrayItem(results, 0), "last_update");

    if (location && now && last_update) 
    {
        cJSON *location_name = cJSON_GetObjectItem(location, "name");
        cJSON *weather_text = cJSON_GetObjectItem(now, "text");
        cJSON *temperature = cJSON_GetObjectItem(now, "temperature");

        if (location_name && location_name->valuestring &&
            weather_text && weather_text->valuestring &&
            temperature && temperature->valuestring &&
            last_update && last_update->valuestring) 
        {
            char hint[1024] = {0};

            printf("地点：%s\n", location_name->valuestring);
            strcat(hint, location_name->valuestring);
            printf("天气状况：%s\n", weather_text->valuestring);
            strcat(hint," ");
            strcat(hint, weather_text->valuestring);
            printf("温度：%s 度\n", temperature->valuestring);
            strcat(hint, temperature->valuestring);
            strcat(hint, "℃  ");
            printf("最后更新时间：%s\n", last_update->valuestring);
            strcat(hint,"最后更新时间:" );
            strcat(hint,last_update->valuestring );
            strcat(hint,"\n" );
            send(node->cid,hint,strlen(hint),0);

            

        }
    }

    // 释放资源
    cJSON_Delete(root);
    

    return 0;
}
//子线程
void * pthread_func(void * arg)
{

    number++;
    printf("在线人数:%d\n",number);

    //取出node
    user_t *node = (user_t *)arg;

    // 定义接收和发送缓存
    char sendbuf[256] = {0};
    char recvbuf[256] = {0};
    char recvmark[256] = {0};
    char login[16] = {0};
    char name[16] = {0}; 
    char code1[32] = {0};
    int login1 = 0;
    // 读取客户端发来的名字
    while (1)
    {
        bzero(recvbuf, sizeof(recvbuf));
        recv(node->cid, recvbuf, sizeof(recvbuf),0);
        //打包的信息第一个为@ 代表注册
        if(strncmp(recvbuf,"@",1) == 0)
        {
            //拆分接收到的数据包
            sscanf(recvbuf, " %s %s %s",login, name, code1);
            printf("接受的recv为%s \n", recvbuf);
            //加入到数据库表userinfo里面
            sqlite_user_add("userinfo",name,code1);
            //do_select(db);
            printf("%s加入了数据库\n", name);
            //注册成功将标志-发送至客户端
            send(node->cid,"-",sizeof("-"),0);
            bzero(login, sizeof(login));
            bzero(name, sizeof(name));
            bzero(code1, sizeof(code1));
            bzero(recvbuf, sizeof(recvbuf));
        }
        //如果打包信息中第一个不是@则代表登录模式
        else
        {
            if(login1 ==1)
            {
                break;
            }
            sscanf(recvbuf,"%s %s", name, code1);
            printf("接受的recv为%s \n", recvbuf);
            printf("登陆用户信息为%s %s\n", name,code1);
            //验证用户名密码是否正确，正确返回1
            int log = just_select(db,name,code1,head,node);
            //登录成功
            if (log == 1)
            {
                login1 = 1;
                send(node->cid,"-",sizeof("-"),0);
                break;
            }

            if(login1 == 0)
            {
                bzero(name, sizeof(name));
            }
            bzero(code1, sizeof(code1));
            bzero(recvbuf, sizeof(recvbuf));
            //登录失败 标志为--
            send(node->cid,"--",sizeof("--"),0);
            continue;
            
        }
    }
    recv(node->cid, recvbuf, sizeof(recvbuf),0);
    //发送加入群聊
    char hint[256] = {0};

    strcpy(hint, name);
    strcat(hint, "加入了聊天室,");
    strcat(hint,"当前在线列表：");
    printf("%d\n",number );
    user_t *q = NULL;
    for(q = head; q != NULL; q = q->next)
    {
        if(q->online == 1)
        {
            strcat(hint,q->name);//添加名字
            strcat(hint," ");
        }
    }
    strcat(hint,"\n");
    mass_send(hint);


    //添加消息头
    char namehead[256] = {0};
    strcpy(namehead , name);
    strcat(namehead , ":");

    int len = 0;
    while (1)
    {   
        //清空recvbuf
        bzero(recvbuf, sizeof(recvbuf));
        len = recv(node->cid,recvbuf,sizeof(recvbuf),0);//接收数据到buf
        if(len <= 0)
        {
            //群发
            user_t *p = NULL;
            for(p = head; p != NULL; p = p->next)//找到退出的用户
            {
                if(strcmp(p->name,name) == 0)
                {
                    p->online = 0;//下线

                    break;
                }
            }
            bzero(hint, sizeof(hint));
            strcpy(hint, name);
            strcat(hint, "退出了聊天室,");
            strcat(hint,"当前在线列表：");
            user_t *q = NULL;
            for(q = head; q != NULL; q = q->next)
            {
                if(q->online == 1)
                {
                    strcat(hint,q->name);//添加名字
                    strcat(hint," ");
                }
            }
            strcat(hint,"\n");
            mass_send(hint);
            printf("%s退出了聊天室\n", name);
            number--;
            printf("当前在线人数:%d\n",number);
    
            sleep(1);
            close(node->cid);
            break;
        }
        if(strncmp(recvbuf,"weather",7) == 0)//检测到天气
        {
            flg = 4;
        }
        if(strncmp(recvbuf,"download",8) == 0)//检测到文件
        {
            flg = 3;
        }
         //传输文件功能
        if(strncmp(recvbuf,"file",4) == 0)//检测到文件
        {
            flg = 2;
   
        }
        if(strncmp(recvbuf,"@",1) == 0)
        {
            flg = 1;
        }
        //私发  

        if(flg == 1)
        {
            printf("进入flg1\n");
            printf("recvbuf：%s\n",recvbuf );
            char buf1[16] = "";//@
            char buf2[16] = "";//私聊的名字
            char buf3[256] = "";//私聊的信息
            int ret = sscanf(recvbuf, "%s %s %s", buf1, buf2,buf3);
            if(ret != 3)
            {
                char error[256] ="请输入正确格式:@ 名字 要发送的消息\n";
                send(node->cid,error,strlen(error),0);
            }
            else
            {
                printf("%s私聊\n",name);
                memset(sendbuf,0,256);
                strcpy(sendbuf,name);;//装入私聊发起者名字
                strcat(sendbuf,"私聊你:");//装入
                strcat(sendbuf,buf3);//包装好聊天信息
                strcat(hint,"\n");
                user_t *p = NULL;
                for(p = head; p != NULL; p = p->next)
                {
                    if (strcmp(p->name,buf2) == 0)
                    {
                        send(p->cid,sendbuf,strlen(sendbuf),0);//发送包装好的的信息
                        memset(sendbuf,0,256);
                        strcpy(sendbuf,"你私聊");
                        strcat(sendbuf,p->name);
                        strcat(sendbuf,"说:");//装入
                        strcat(sendbuf,buf3);//包装好聊天信息
                        strcat(hint,"\n");
                        send(node->cid,sendbuf,strlen(sendbuf),0);
                        break;
                    }
                }
                flg = 0;
            }
        }

        else if(flg == 2)
        {
            printf("进入flg2\n");
            printf("recvbuf：%s\n",recvbuf );
            char file[16] = "";
            char filename[16] = "";
            int ret2 = sscanf(recvbuf, "%s %s", file, filename);
            if(ret2 != 2)
            {
                char error2[256] ="请输入正确格式:file 文件名\n";
                send(node->cid,error2,strlen(error2),0);
            }
            else
            {

                recv_file(node->cid);
                flg = 0;
            }
        }
        else if(flg == 3)
        {
            printf("进入flg3\n");
            printf("recvbuf：%s\n",recvbuf );
            char fuc[16] = "";
            char filename[16] = "";
            int ret2 = sscanf(recvbuf, "%s %s", fuc, filename);
            if(ret2 != 2)
            {
                char error2[256] ="请输入正确格式:download 文件名\n";
                send(node->cid,error2,strlen(error2),0);
            }
            flg = 0;
            
        }
        else if(flg == 4)
        {
            printf("进入flg4\n");
            printf("recvbuf：%s\n",recvbuf );
            
            weather(node);

            flg = 0;
            
        }

        //群发
        else
        {
            if(strncmp(recvbuf,"quit",4) == 0)//检测到退出，关闭套接字
            {
                printf("%s退出了聊天室..\n",name);
                number--;
                user_t *p = NULL;
                for(p = head; p != NULL; p = p->next)//找到退出的用户，清除数据
                {
                    if(strcmp(p->name,name) == 0)
                    {
                        p->cid = 0;
                        bzero(p->name, 16);
                        break;
                    }
                }
                bzero(hint, sizeof(hint));
                strcpy(hint, name);
                strcat(hint, "退出了聊天室,");
                strcat(hint,"当前在线列表：");
               user_t *q = NULL;
                for(q = head; q != NULL; q = q->next)
                {
                    if(q->name != NULL)
                    {
                        strcat(hint,q->name);//添加名字
                        strcat(hint," ");
                    }
                }
                strcat(hint,"\n");
                mass_send(hint);
                printf("当前在线人数:%d\n",number);
                close(node->cid);
                break;
            }
            
            memset(sendbuf,0,256);
            strcat(sendbuf,namehead);//装入名字
            strcat(sendbuf,recvbuf);//包装好聊天信息
            mass_send(sendbuf);//群发
            printf("%s",sendbuf);
        }
    }
}


int main(int argc,char ** argv)
{
    head = List_Init();
    user_t *node = NULL;

    system("clear");
    if(argc <3 )
    {
        perror("使用: ./server + 自己IP地址 + 自己端口号\n");
        return -1;
    }
    int ret = 0;
    //1创建套接字
    int tcp_server = socket(AF_INET,SOCK_STREAM,0);
     if (tcp_server == -1)
    {
        perror("创建服务器套接字失败\n");
        exit(1);
    }
    //1.1打开指定的数据库文件，如果不存在将创建一个同名的数据库文件
    rc = sqlite3_open("user.db",&db);
    if(rc < 0)
    {
        printf("create user failure:%s\n",sqlite3_errmsg(db));
        return -1;
    }

    else
    {
        printf("create user successfuly\n");
        //sqlite3_close(db);//关闭数据库
    }
    //创建表
    ret = sqlite3_exec(db, CMD_CREATE, NULL, NULL, NULL);
    if (ret != 0)
    {
        printf("创建表格失败\n");
        return -1;
    }
    //绑定自己ip和端口 
    struct sockaddr_in IPV4;
    IPV4.sin_family = AF_INET;
    IPV4.sin_port = htons(atoi(argv[2]));
    IPV4.sin_addr.s_addr = inet_addr(argv[1]);

    ret = bind(tcp_server,(struct sockaddr *)&IPV4,sizeof(IPV4));
    if( ret < 0 )
    {
        perror("绑定失败..\n");
        return -1;
    }

    //3监听
    listen(tcp_server,10);
    printf("聊天室启动成功，等待用户连接..\n");

    //4等待响应 
    struct sockaddr_in IPV4_client;//客户端的ip和端口
    socklen_t IPV4_len = sizeof(IPV4_client);

    pthread_t tid = 0;
    while (1)
    {
        int client_id = accept(tcp_server,(struct sockaddr*)&IPV4_client,&IPV4_len);//每连接一个客户端，会有新的套接字，开新的线程处理该次连接
        /****** 5处理通讯 ******/
        user_t *p = NULL;
        if(client_id > 0)
        {

            node = Request_Node(client_id);
            Insert_Node(head, node);
            printf("[%s:%d]连接到服务器\n",inet_ntoa(IPV4_client.sin_addr),ntohs(IPV4_client.sin_port));
               
            
            
            // 多线程处理 
            pthread_create(&tid,NULL,pthread_func,node);
            pthread_detach(tid);
        }
    }      
    close(tcp_server);
    return 0;
}