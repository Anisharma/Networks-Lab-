//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux 
#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
    
#define TRUE   1 
#define FALSE  0 
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
    char *binary = malloc(sizeof(s)*8);
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

char* BinaryToString(char* s)
{
    if(s == NULL) return 0;
    char *str = malloc(sizeof(s)/8+1);
    strcpy(str,"");
    char *ptr;
    int i,size=0;
    for(ptr = s ; *ptr != 0; ++ptr)
    {
        int x=0;
        for(i = 7; i >= 0; --i)
        {
            if(*ptr=='1')
            {
                x=x+power(2,i);
            }
            ++ptr;
        }
        --ptr;
        char ch=x;
        str[size]=ch;
        size++;
    }
    str[size]='\0';
    return str;
}

char *Base64_encoder(char *s)
{
char *ss=malloc(sizeof(s)+10);
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
char *str = malloc(sizeof(ss)*8/6+1);
char *s1=stringToBinary(ss);
strcpy(str,"");
char *ptr;
int i,size=0;
for(ptr = s1 ; *ptr != 0; ++ptr)
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
return str;
}

int give_index(char ch)
{
    int i;
    for(i=0;i<64;i++)
    {
        if(encoding_table[i]==ch)
            return i;
    }
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

char *Base64_decoder(char *s)
{
int n=strlen(s);
char *str=malloc(n*6+1);
int i,j;
for(i=0;i<n;i++)
{
    int x=give_index(s[i]);
    for(j=5;j>=0;j--)
    {
        if(x%2)
            str[6*i+j]='1';
        else
            str[6*i+j]='0';
        x=x/2;
    }
}
str[6*i]='\0';
char *ans=BinaryToString(str);
n=strlen(ans);
if(ans[n-2]=='=')
    ans[n-2]='\0';
else if(ans[n-1]=='=')
    ans[n-1]='\0';
return ans;
}

int main(int argc , char *argv[])  
{  
    int PORT = 0 ; 
    int i = 0;
    for (i = 0; i < 4; ++i){
        PORT = PORT*10 + (argv[1][i] - '0');
    } 
    int opt = TRUE;  
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, valread , sd;  
    int max_sd;  
    struct sockaddr_in address;  
        
    char buffer[1025];  //data buffer of 1K 
        
    //set of socket descriptors 
    fd_set readfds;  
        
    //a message 
    char *message = "ECHO Daemon v1.0 \r\n";  
    
    //initialise all client_socket[] to 0 so not checked 
    for (i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0;  
    }  
        
    //create a master socket 
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
    
    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  
    
    //type of socket created 
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( PORT );  
        
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", PORT);  
        
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(master_socket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
        
    //accept the incoming connection 
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  
        
    while(TRUE)  
    {  
        //clear the socket set 
        FD_ZERO(&readfds);  
    
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
            
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_socket[i];  
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &readfds);
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        }  
    
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
      
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }  
            
        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
            
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
          
            //send new connection greeting message 
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )  
            {  
                perror("send");  
            }  
                
            puts("Welcome message sent successfully");  
                
            //add new socket to array of sockets 
            for (i = 0; i < max_clients; i++)  
            { 
                //if position is empty 
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                        
                    break;  
                }  
            }  
        }  
            
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)  
        {  
            sd = client_socket[i];                 
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read( sd , buffer, 1024)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                        
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    client_socket[i] = 0;  
                }  
                    
                //Echo back the message that came in 
                else
                {  
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    buffer[valread] = '\0';  
                    char* str = Base64_decoder(buffer);
                    printf("Encoded message : %s\n", buffer);
                    printf("Original message : %s\n", str+5);
                    if(str[0]== '1')
                    {
                        str = "ACK";
                        str=packetConstruction(str,2);
                        char *sent = Base64_encoder(str);
                        send(sd , sent , strlen(sent) , 0 );
                        printf("Acknowledgement Sent\n");
                    }
                    else
                    {
                        str = "Disconnection_Message";
                        str=packetConstruction(str,2);
                        char *sent = Base64_encoder(str);
                        send(sd , sent , strlen(sent) , 0 );
                        printf("Acknowledgement Sent\n");
                    }   
                }

            }  
        }  
    }   
    return 0;  
}