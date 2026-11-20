#include "header.h"

#define send_file_size sfs
//1.socket   建立套接字文件
//2.connect() 请求连接
//3.read/write  send/recv
//4.shutdown()  
#define    dirname     "/root/download"
//服务端先接再发
//客户端先发再收
int flg = 0; //私聊标志,置1为私聊
char name[16]= "";
char filename[30] = "";
char func[30] = "";
int login(int tcp_client)
{
    int num = 0;
    char name[16]= "";
    char code[32]= "";
    char sendbuff[256]= "";
    char recvbuff[256]= "";
    char recvmark[256] = {0};
    while (1)
    {
        printf("==============欢迎加入聊天室==============\n");
        printf("              1.注册\n");
        printf("              2.登录\n");
        scanf("%d",&num);
        if(num == 1)
        {
            printf("请输入你的用户名：");
            scanf("%s",name);
            printf("请输入你的密码：");
            scanf("%s",code);
            strcpy(sendbuff,"@");
            strcat(sendbuff," ");
            strcat(sendbuff,name);
            strcat(sendbuff," ");
            strcat(sendbuff,code);
            send(tcp_client,sendbuff,strlen(sendbuff),0);//发送到服务器
            recv(tcp_client,recvmark,256,0);
            if(strcmp(recvmark,"-") == 0)
            {
                printf("注册成功\n");
            }
            else
            {
                printf("注册失败\n");
            }
            bzero(sendbuff,256);
            bzero(recvmark,256);
        }
        else if(num == 2)
        {
            printf("请输入你的用户名：");
            scanf("%s",name);
            printf("请输入你的密码：");
            scanf("%s",code);
            sprintf(sendbuff, "%s %s", name, code); 
            //printf("发送给服务器的数据%s\n",sendbuff);
            send(tcp_client,sendbuff,strlen(sendbuff),0);//发送到服务器
            recv(tcp_client,recvmark,256,0);
            //printf("接收到的rev为%s\n",recvmark );

            if (strstr(recvmark,"加入了聊天室") != NULL)
            {
                bzero(recvmark,256);
                recv(tcp_client,recvmark,256,0);
                //printf("接收到的rev为%s\n",recvmark );
            }

            if(strcmp(recvmark,"-") == 0 )
            {
                printf("登录成功\n");
                bzero(sendbuff,256);
                bzero(recvmark,256);
                break;
            }
            else
            {
                printf("登录失败\n");
                bzero(sendbuff,256);
                bzero(recvmark,256);
            }
            
        }
    }
    
}

int send_file(int sockfd, char filename[]) 
{
    char size[1024];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) 
    {
        perror("open file error");
        return -1;
    }

    // 确定文件大小
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) 
    {
        perror("lseek error");
        close(fd);
        return -1;
    }
    printf("文件大小：%ld\n", file_size);
    lseek(fd, 0, SEEK_SET);
 
    // 发送文件大小
    sprintf(size,"%ld",file_size);
    if (send(sockfd, size, strlen(size), 0) == -1) 
    {
        perror("send file size error");
        close(fd);
        return -1;
    }

    // 发送文件内容
    char buf[1024];
    ssize_t bytes_read, bytes_sent;
    while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) 
    {
        bytes_sent = send(sockfd, buf, bytes_read, 0);
        printf("文件发送成功.\n");
        if (bytes_sent == -1) 
        {
            perror("send error");
            close(fd);
            return -1;
        }
    }

    if (bytes_read == -1) 
    {
        perror("read error");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}
void * Download( char filename[])
{
    char buf[20];
    char buf_write[20];
    char work_path[256];
    DIR * dp=opendir(dirname);
    int a,b;
    if(dp == NULL)
    {
        perror("打开目录失败\n");
    }
    chdir(dirname);
    getcwd(work_path, sizeof(work_path));
    printf("当前工作路径：%s\n",work_path);
    //需要下载的文件名
    int fd1=open(filename,O_RDWR, 0777);
    //新建的文件
    int fd2=open("Download.txt",O_CREAT|O_TRUNC|O_RDWR, 0777);
    if(fd1 < 0 || fd2 < 0 )
    {
        perror("打开文件失败\n");
    }
    int read_len=lseek(fd1, 0, SEEK_END);
    printf("文件总字节数：%d\n",read_len);
    int write_len=0;
    lseek(fd1, 0, SEEK_SET);
    while(1)
    {
        bzero(buf,sizeof(buf));
        bzero(buf_write,sizeof(buf_write));
        a=read(fd1, buf, sizeof(buf));
        printf("读取到的内容是：%s\n",buf);
        strcpy(buf_write, buf);
        b=write(fd2, buf_write, strlen(buf_write));
        write_len += b;
        printf("已写入的内容是：%s\n",buf_write);
        if(a == -1||b == -1)
        {
            perror("读写出错\n");
            break;
        }
        else if(write_len == read_len)
        {
            printf("复制完成,总字节数：%d\n",write_len);
            break;
        }    
    } 
    closedir(dp);
    close(fd1);
    close(fd2);
}



void display(char chat[][256],char* name)//聊天界面
{

 
    system("clear");
    printf("---------------------------聊天室------------------------------\n");
    printf("功能如下:\n");
    printf("1.默认为群聊模式,输入@开启私聊模式..\n");
    printf("2.文件传输 输入file 文件名..       \n");
    printf("3.下载文件 输入download 文件名..   \n");
    printf("4.查询天气 输入weather..           \n");
    int k=0;
    for(int i = 0;i<19+k;i++)
    {   
        if(strlen(chat[i])!=0)
        {
            if(strstr(chat[i],"加入了聊天室")!=NULL || strstr(chat[i],"退出了聊天室")!=NULL)
            {
                printf("%s",chat[i]);
            }
            else if(strstr(chat[i],"私聊你")!=NULL || strstr(chat[i],"你私聊")!=NULL)
            {
                printf("%s",chat[i]);
                printf("\n");
            }
            else
            {
                printf("%s",chat[i]);
            }
        }
        else
            printf("\n");
    }
    printf("%s-----------------------------------------------------------\n",name);
    //sleep(1);
}



int main(int argc,char ** argv)
{
    if(argc <3 )
    {
        perror("使用: ./client + 服务器IP地址 + 服务器端口号\n");
        return -1;
    }
    int ret = 0;
    //1建立套接字：客户端只有一个套接字。就是cid，把客户端cid与对面的绑定在一起。
    int tcp_client = socket(AF_INET,SOCK_STREAM,0);
    if (tcp_client < 0)
    {
        perror("socket error\n");
        return -1;
    }
    
    //链接服务器，把对面的地址和端口号包装起来
    struct sockaddr_in IPV4;
    memset(&IPV4,0,sizeof(IPV4));
    IPV4.sin_family = AF_INET;
    IPV4.sin_port = htons(atoi(argv[2]));
    IPV4.sin_addr.s_addr = inet_addr(argv[1]);

 
    //2.建立链接 --- 连接的是主机的，服务器的ip和端口号
    ret = connect(tcp_client,(struct sockaddr * )&IPV4,sizeof(IPV4));
    if(ret < 0 )
    {
        printf("连接失败..\n");

    }
    else
    {
        login(tcp_client);
        //select函数  
        fd_set rset = {0}; 
        struct timeval tm = {0};
        int count = 0;
        int len = 0;
        char recvbuf[256];
        char sendbuf[256];
        //聊天记录
        int i=0, j=0;
        char chat[20][256] = {0};
        //与对面建立连接以后，就开始发送数据
        //输入名字
        char name[16] = "";
        printf("请输入昵称：");
        scanf("%s",name);
        send(tcp_client,name,strlen(name),0);//发送名字到服务器
    
    
        printf("欢迎来到聊天室..\n");
        sleep(1);

        //3.处理聊天数据
        while (ret >= 0 )
        {
            FD_SET(STDIN_FILENO,&rset);
            FD_SET(tcp_client,&rset);
            tm.tv_sec = 5;
            count = select(tcp_client + 1,&rset,NULL,NULL,&tm);

            //收
            if(FD_ISSET(tcp_client,&rset))
            {
                memset(recvbuf,0,256);
                recv(tcp_client,recvbuf,256,0);
                if(i<18)
                {
                    strcpy(chat[i],recvbuf);
                    i++;
                }
                else
                {
                    for(j = 0;j < 17;j++)
                    {
                        strcpy(chat[j],chat[j+1]);
                    }
                    strcpy(chat[j],recvbuf);
                }
                display(chat,name);
            }

            //发   
            if(FD_ISSET(STDIN_FILENO,&rset))
            {
                bzero(func, sizeof(func));
                bzero(filename, sizeof(filename));
                memset(sendbuf,0,256);
                read(STDIN_FILENO,sendbuf,sizeof(sendbuf));//把数据从外存文件写到内存中

                send(tcp_client,sendbuf,strlen(sendbuf),0);
                if(strncmp(sendbuf,"quit",4) == 0 )
                {
                    break;
                }
                if(strncmp(sendbuf,"file",4) == 0 )
                {
                    sscanf(sendbuf,"%s %s", func, filename);
                    printf("sendbuf:%s\n", sendbuf);
                    send_file(tcp_client,filename);

                }
                if(strncmp(sendbuf,"download",8) == 0 )
                {
                    sscanf(sendbuf,"%s %s", func, filename);
                    printf("sendbuf:%s\n", sendbuf);
                    Download(filename);
                    
                }
            }
            if(ret < 0)
                break;
        }
    }
}
