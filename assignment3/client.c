#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
//#define PORT 8080

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
								'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
								'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
								'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
								'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
								'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
								'w', 'x', 'y', 'z', '0', '1', '2', '3',
								'4', '5', '6', '7', '8', '9', '+', '/'};


int power(int a,int b)
{
int x=1;
while(b--)
	{
	x=x*a;
	}
return x;
}

char* stringToBinary(char* s)
{
    if(s == NULL) return 0;
    char *binary = malloc(strlen(s)*8);
    strcpy(binary,"");
    char *ptr;
    int i;

    for(ptr = s ; *ptr != 0; ++ptr)
    {
        for(i = 7; i >= 0; --i)
        	{
            (*ptr & 1 << i) ? strcat(binary,"1") : strcat(binary,"0");
    		}
    }
    return binary;
}

char *Base64_encoder(char *s)
{
char *ss=malloc(strlen(s)+10);
strcpy(ss,s);
int n=strlen(ss);
if(n%3==2)
{
	strcat(ss,"=");
}
else if(n%3==1)
{
	strcat(ss,"==");
}

// printf("%s\n",ss );
char *s1=stringToBinary(ss);
// printf("%s\n",s1 );

char *str = malloc(strlen(ss)*8/6+1);

char *ptr;
int i,size=0;
for(ptr = s1 ; *ptr != '\0'; ++ptr)
    {
    int x=0;
    for(i = 5; i >= 0; --i)
    {
        if(*ptr=='1')
        {
            x=x+power(2,i);
        }
        ++ptr;
    }
    --ptr;
    char ch=encoding_table[x];
    str[size]=ch;
    //printf("%c",ch);
    size++;
    }
str[size]='\0';
// printf("%s\n",str );
return str;
}

char * packetConstruction(char * str, int type){
	int msg_len = strlen(str);
	char * packet = malloc(msg_len + 6);
	packet[msg_len + 5] = '\0';
	packet[0] = (char)(type+48);
	int temp = msg_len;
	if(msg_len >= 1000){
		packet[1] = (char)(temp/1000) + '0';
		packet[2] = (char)((temp/100)%10) + '0';
		packet[3] = (char)((temp/10)%10) + '0';
		packet[4] = (char)(temp%10) + '0';
	}
	else if(msg_len < 1000 && msg_len >= 100){
		packet[1] = '0';
		packet[2] = (char)((temp/100)) + '0';
		packet[3] = (char)((temp/10)%10) + '0';
		packet[4] = (char)(temp%10) + '0';
	}
	else if(msg_len < 100 && msg_len >= 10){
		packet[1] = '0';
		packet[2] = '0';
		packet[3] = (char)((temp/10)) + '0';
		packet[4] = (char)(temp%10) + '0';
	}
	else if(msg_len < 10 && msg_len >= 0)
	{
		packet[1] = '0';
		packet[2] = '0';
		packet[3] = '0';
		packet[4] = (char)(temp) + '0';
	}
	int i;
	for (i = 0; i < msg_len; ++i)
	{
		packet[i+5] = str[i];
	}
	return packet;

}

int main(int argc, char const *argv[])
{
	int PORT = 0 ; 
	int i = 0;
	for (i = 0; i < 4; ++i){
	 	PORT = PORT*10 + (argv[2][i] - '0');
	} 
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}
  
	memset(&serv_addr, '0', sizeof(serv_addr));
  
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	  
	// Convert IPv4 and IPv6 addresses from text to binary form
	/*if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}*/
  	if(inet_pton(AF_INET, (char *)argv[1], &serv_addr.sin_addr)<=0) 
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}else{
		printf("\nConnection Established \n");
	}
	while(1)
	{
		int pole ; 
		printf("Press 1 to enter the message and 3 to close the connection\n");
		scanf("%d",&pole); 
				if(pole ==1)
				{
					char hello[1024];
					printf("Enter message:");
					getchar();
					fgets(hello , 1024 , stdin);
					//scanf("%s",hello);
					char *str=packetConstruction(hello,pole);
					char *encrypted_message=Base64_encoder(str);
					send(sock , encrypted_message , strlen(encrypted_message) , 0 );
					printf("Message Sent!\n");
					valread = read( sock , buffer, 1024);
					printf("Message Received : ");
					printf("ACK\n");
				}
				else if(pole == 3)
				{
					char hello[1024]="DisconnectPlease";
					char *str=packetConstruction(hello,pole);
					char *encrypted_message=Base64_encoder(str);
					send(sock , encrypted_message , strlen(encrypted_message) , 0 );
					printf("Message Sent\n");
					valread = read( sock , buffer, 1024);
					printf("Message Received : ");
					printf("Disconnection_Message\n");
					break;
				}     
				else
					printf("Please enter from provided options\n");  
	}
	return 0;
}