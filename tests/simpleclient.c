#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <cJSON/cJSON.h>
#include <utils_cpp/utils.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>
#include <openssl/evp.h>
#include <stdio.h>


// const int batch_size = 512;
#define batch_size 128   // 每次发送的 binary data 数据大小

typedef struct SendDataInfo{
    unsigned int length;
    unsigned int id;
    unsigned int crc;
    unsigned char data[0];
}SendData;


int main(int argc, char *argv[])
{

    int sockfd;
    int len;
    struct sockaddr_in address;
    int result;
    char ch = 'A';

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("192.168.10.100");
    address.sin_port = htons(4000);
    len = sizeof(address);

    const char* wait_filename = "/tmp/base64.c";

    result = connect(sockfd, (struct sockaddr *)&address, len);

    if (result == -1)
    {
        perror("oops: client1");
        exit(1);
    }

    unsigned long long filesize = getFileSizeU(wait_filename);

    unsigned long long epochs = ceil(filesize / (batch_size * 1.0)) ;
    unsigned long epoch = 0;

    unsigned int numRead = 0;
    // @@@@ 1. 
    unsigned char buf[batch_size] = {0};

    int inputFd;
    inputFd = open(wait_filename, O_RDONLY);

    int send_total = 0;

    // 1 从文件中读取 batch_size 个字节的数据 到buf中
    
    ;
    while((numRead = read(inputFd, buf, batch_size)) > 0){
        epoch ++;
        printf("\n\n=======================\n");
        printf("epoch = %lu/%lu\n", epoch, epochs);
        // print_payload(buf, numRead);
        printf("numRead = %d\n", numRead);

        cJSON * data = cJSON_CreateObject();
        cJSON_AddStringToObject(data, "name", "voice");
        cJSON_AddNumberToObject(data, "epoch", epoch);
        cJSON_AddNumberToObject(data, "epochs", epochs);

        unsigned char base64msg[1024] = {0};
        EVP_EncodeBlock((unsigned char*)base64msg, buf, numRead);
        printf("base64: %s\n", base64msg);
      
        cJSON_AddStringToObject(data, "raw", base64msg);

        char *body = cJSON_Print(data);

        SendData send_data;
        unsigned long send_len = sizeof(SendData) + strlen(body) + 1;

        SendData *psd = (SendData *)calloc(send_len, sizeof(char));
        psd->length = strlen(body)+1;
        psd->crc = 1;
        psd->crc = 1;
        psd->id = 2;

        memcpy(psd->data, body, psd->length);

        write(sockfd, (const char*)psd, send_len);
        send_total += numRead;

        char a = 0;
        recv(sockfd, &a, 1, 0);

        // sleep();
        // usleep(1000);  // 1ms
        
        memset(buf, 0, strlen(buf));

        // numRead = read(inputFd, buf, batch_size);
    }
    printf("发送字节数: %d\n", send_total);
    printf("文件总大小(节数): %lu\n", filesize);

    close(sockfd);
    exit(0);
}
