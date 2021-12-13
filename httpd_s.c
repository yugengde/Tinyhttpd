#include<stdio.h>
#include<sys/socket.h>
//#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
//#include<ctype.h>
//#include<strings.h>
#include<string.h>
#include<sys/stat.h>
#include<pthread.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<stdint.h>

#define ISspace(x) isspace((int)(x))
#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"


typedef unsigned short u_short;

int startup(u_short *port);
void error_die(const char*);
void accept_request(void *);
int get_line(int, char*, int);
void unimplemented(int);
void not_found(int);
void serve_file(int, const char*);
void execute_cgi(int, const char*, const char*, const char*);
void headers(int, const char*);
void cat(int, FILE *);

void execute_cgi(int client, const char *path, const char *method, const char* query_string){

}

void headers(int client, const char *filename)
{
    char buf[1024];
    (void)filename;  /* could use filename to determine file type */

    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
}

void cat(int client, FILE *resource)
{
    char buf[1024];

    fgets(buf, sizeof(buf), resource);
    while(!feof(resource))
    {
        send(client, buf, strlen(buf), 0);
        fgets(buf, sizeof(buf), resource);
    }
}


void serve_file(int client, const char* filename ){
    FILE *resource = NULL;
    int numchars = 1;
    char buf[1024];

    buf[0] = 'A'; buf[1] = '\0';

    while((numchars >0) && strcmp("\n", buf))
        numchars = get_line(client, buf,sizeof(buf));

    resource = fopen(filename, "r");
    if(resource == NULL)
        not_found(client);
    else
    {
        headers(client, filename);
        cat(client, resource);
    }

    fclose(resource);

}

void not_found(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}


void unimplemented(int client){
    char buf[1024];

    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</TITLE></HEAD>");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></THML>\r\n");
    send(client, buf, strlen(buf), 0);
}

void error_die(const char* sc){
    perror(sc);
    exit(1);
}

int startup(u_short *port){

    int httpd = 0;
    int on = 1;

    struct sockaddr_in name;

    httpd = socket(PF_INET, SOCK_STREAM, 0);
    if(httpd ==-1){
        error_die("socket");
    }

    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(*port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
        error_die("setsockopt failed");

    if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
        error_die("bind");

    if(*port==0){
        socklen_t namelen = sizeof(name);
        if(getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
            error_die("getsockname");
        *port = ntohs(name.sin_port);
    }

    if(listen(httpd, 5) < 0)
        error_die("listen");

    return (httpd);
}

int get_line(int sock, char* buf, int size){
    int i=0;
    char c = '\0';
    int n;

    while((i<size-1) && (c!='\n'))
    {
        n = recv(sock, &c, 1, 0);
        printf("%02X\n", c);

        if(n>0)
        {
            if (c=='\r'){
                n = recv(sock, &c, 1, MSG_PEEK);

                if ((n>0) && (c=='\n'))
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';

            }
            buf[i] = c;
            i++;
        }
        else
            c = '\n';
    }
    buf[i] = '\0';

    return (i);

}

void accept_request(void *arg){
    int client = (intptr_t)arg;

    char buf[1024];
    size_t numchars;
    char method[255];
    char url[255];
    char path[512];
    size_t i,j;
    struct stat st;
    int cgi = 0;

    char *query_string = NULL;

    numchars = get_line(client, buf, sizeof(buf));
    i = 0; j=0;

    while(!ISspace(buf[i]) && (i < sizeof(method) -1))
    {
        method[i] = buf[i];
        i++;
    }
    j=i;
    method[i] = '\0';

    if(strcasecmp(method, "GET") && strcasecmp(method, "POST"))
    {
        unimplemented(client);

        return ;
    }

    if(strcasecmp(method, "POST") ==0)
        cgi = 1;

    i = 0;
    while(!ISspace(buf[j]) && (i<sizeof(url) -1) && (j<numchars))
    {
        url[i] = buf[j];
        i++; j++;
    }
    url[i] = '\0';

    if(strcasecmp(method, "GET") ==0)
    {
        query_string = url;
        while((*query_string !='?') && (*query_string != '\0'))
            query_string ++;
        if(*query_string == '?')
        {
            cgi = 1;
            *query_string = '\0';
            query_string ++;
        }
    }

    sprintf(path, "htdocs%s", url);

    if(path[strlen(path) -1] == '/')
        strcat(path, "index.html");
    if(stat(path, &st) == -1){
        while((numchars > 0) && strcmp("\n", buf))
            numchars = get_line(client, buf, sizeof(buf));
        not_found(client);
    }
    else
    {
        if((st.st_mode & S_IFMT) == S_IFDIR)
            strcat(path, "/index.html");
        if((st.st_mode & S_IXUSR) ||
                (st.st_mode & S_IXGRP) ||
                (st.st_mode & S_IXOTH)
          )
            cgi = 1;

        if(!cgi)
            serve_file(client, path);
        else
            execute_cgi(client, path, method, query_string);

    }

    close(client);


}

int main(){


    int server_sock = -1;
    u_short port = 4000;
    int client_sock = -1;

    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);
    pthread_t newthread;

    server_sock = startup(&port);
    printf("httpd running on port %d\n", port);

    while(1)
    {
        client_sock = accept(server_sock, (struct sockaddr *)&client_name, &client_name_len);
        if(client_sock ==-1)
            error_die("accept");

        if(pthread_create(&newthread, NULL, (void*)accept_request, (void *)(intptr_t)client_sock) !=0)
            perror("pthread_create");
    }

    close(server_sock);

    return 0;

}
