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


const int batch_size = 512;

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

    // const char* wait_filename = "test.pcm";
    // const char* wait_filename = "/home/dev/vosvoice.20211206_211644.tar.xz";
    // const char* wait_filename = "/usr/local/src/git-source/c2cpp/study/Tinyhttpd/build/Makefile";
    const char* wait_filename = "/tmp/base64.c";

    result = connect(sockfd, (struct sockaddr *)&address, len);

    if (result == -1)
    {
        perror("oops: client1");
        exit(1);
    }

    unsigned long filesize = getFileSizeU(wait_filename);

    unsigned long epochs = ceil(filesize / (batch_size * 1.0)) ;
    unsigned long epoch = 0;

    unsigned int numRead = 0;
    char vbuf[batch_size];

    int inputFd;
    inputFd = open(wait_filename, O_RDONLY);


    while((numRead = read(inputFd, vbuf, batch_size)) > 0){
        printf("\n\n=======================\n");
        epoch ++;
        // printf("numRead: %d\n", numRead);
        printf("epoch = %lu/%lu\n", epoch, epochs);
        printf("raw: %s\n", vbuf);

        // printf("原始数据 = %s\n", vbuf);
        // 发送数据
        cJSON * data = cJSON_CreateObject();
        cJSON_AddStringToObject(data, "name", "voice");
        cJSON_AddNumberToObject(data, "epoch", epoch);
        cJSON_AddNumberToObject(data, "epochs", epochs);


        /*
        char* vbuf_base64 = (char* ) malloc(sizeof(char) * ((batch_size +2) * 1.0) * 4/2  +1);
        base64_encode(vbuf, batch_size, vbuf_base64);
        */

        //

        /*
        EVP_ENCODE_CTX * ectx = NULL;
        ectx = EVP_ENCODE_CTX_new();

        int outLen = 0;
        unsigned char out[1024];

        EVP_EncodeInit(ectx);

        EVP_EncodeUpdate(ectx, out, &outLen, vbuf, numRead);

        EVP_EncodeFinal(ectx, out+outLen, &outLen);

        printf("out  = %s\n", out);
        */

        //
        //

        //
        unsigned char base64msg[1024] = {0};
        EVP_EncodeBlock((unsigned char*)base64msg, (const unsigned char*)vbuf, numRead);
        printf("base64: %s\n", base64msg);
      
        //


        cJSON_AddStringToObject(data, "raw", base64msg);

        char *body = cJSON_Print(data);
        // printf("%s\n", body);



        SendData send_data;
        unsigned long send_len = sizeof(SendData) + strlen(body) + 1;

        // printf("1. voice binary data size: %d\n", batch_size);   // 二进制数据
        // printf("2. voice base64 string size: %lu\n", strlen(out) + 1);  // base64 string 数据
        // printf("3. body(json data) size: %lu\n", strlen(body)+ 1);
        // printf("4. total size (body + header) : %lu\n", send_len);
        // printf("msg: %s", body);
        // printf("raw size = %lu, data size = %lu send size = %lu \n", sizeof(vbuf_base64), strlen(body)+1, send_len);

        SendData *psd = (SendData *)calloc(send_len, sizeof(char));
        psd->length = strlen(body)+1;
        psd->crc = 1;
        psd->crc = 1;
        psd->id = 2;

        memcpy(psd->data, body, psd->length);

        write(sockfd, (const char*)psd, send_len);
        char a = 0;
        recv(sockfd, &a, 1, 0);
        // printf("a = %c\n", a);

        // sleep(2);
        memset(vbuf, 0, sizeof(vbuf));

    }



    // write(sockfd, &ch, 1);
    // read(sockfd, &ch, 1);
    // printf("char from server = %c\n", ch);
    close(sockfd);
    exit(0);
}
