#include<stdio.h>
#include <sys/socket.h>
#include<netinet/ether.h>
#include<arpa/inet.h>//美国高级研究鼠


int main()
{
    int sock = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));//
    while(1)
    {
        unsigned char str[1600] = {0};
        recv(sock,str,1600,0);


        unsigned char dst_mac[18] = {0};
        unsigned char src_mac[18] = {0};

        sprintf(dst_mac,"%x:%x:%x:%x:%x:%x:",str[0],str[1],str[2],str[3],str[4],str[5]);
        sprintf(src_mac,"%x:%x:%x:%x:%x:%x:",str[6],str[7],str[8],str[9],str[10],str[11]);

        printf("src mac : %s      --->   dst  mac  :  %s\n",src_mac,dst_mac);
    }
    return 0;
}