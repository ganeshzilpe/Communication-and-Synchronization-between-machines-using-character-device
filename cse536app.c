//##############################################################
//########## Application Program : CSE536app  ##################
//##############################################################

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/fs.h>


#define SERVER_PORT 23456
#define MAX_PENDING 5
#define MAX_LINE 256

char monitorAddress[20];
//new packet structure for handling event and ack
struct datagramBuffer
{
	uint32_t record_id;
	uint32_t final_clock;
	uint32_t original_clock;
	__be32 source_ip;
	__be32 destination_ip;
	uint8_t data[236];
};

FILE *openfile(char *opts)
{
	FILE *fd = NULL;
	fd = fopen("/dev/cse5361", opts);
	if (!fd)
	{	printf("File error opening file\n");
	}
	return fd;
}

void sendToMonitor(char *data)
{
   struct sockaddr_in client, server;
   struct hostent *hp;
   char buf[MAX_LINE];
   int len, ret, n;
   int s, new_s;

   bzero((char *)&server, sizeof(server));
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = htons(0);

   s = socket(AF_INET, SOCK_DGRAM, 0);
   if (s < 0)
   {
		perror("simplex-talk: UDP_socket error");
		exit(1);
   }

   if ((bind(s, (struct sockaddr *)&server, sizeof(server))) < 0)
   {
		perror("simplex-talk: UDP_bind error");
		exit(1);
   }
	if(monitorAddress == "")
   		hp = gethostbyname( "192.168.0.2"  );// Monitor Address Change in class
	else
		hp = gethostbyname( monitorAddress );
   if( !hp )
   {
      	fprintf(stderr, "Unknown host %s\n", "localhost");
      	exit(1);
   }

   bzero( (char *)&server, sizeof(server));
   server.sin_family = AF_INET;
   bcopy( hp->h_addr, (char *)&server.sin_addr, hp->h_length );
   server.sin_port = htons(SERVER_PORT);
   ret = sendto(s, data, 256, 0,(struct sockaddr *)&server, sizeof(server));
   if( ret <= 0)
   {
	fprintf( stderr, "Datagram Send error %d\n", ret );
   }
}

main(int argc, char *argv[])
{
	FILE *fd = NULL;
	char buffer[257], data[256], *remoteip;
	size_t count;
	int quit = 0, ch, input=0;
        struct datagramBuffer datagram1,datagram2,datagram3;

	while(quit == 0){

		printf("\n============== Menu ==============\n");
		printf(" || 1. || Set destination address\n");
		printf(" || 2. || Write to the device\n");
		printf(" || 3. || Read from the device\n");
		printf(" || 4. || Set monitor IP address\n");
		printf(" || 5. || Exit\n");
		printf("\n============== Menu ==============\n");

		scanf("%d", &input);

		while( (ch = fgetc(stdin)) != EOF && ch != '\n' ){}	//clear stream

		switch(input)
		{
			case 1:
				printf("Enter Destination IP address (ex. 192.168.0.36):");
				scanf("%256s", data);
				while( (ch = fgetc(stdin)) != EOF && ch != '\n' ){}	//clear stream
				memset(buffer, 0, 257);
				buffer[0] = 0;
				memcpy(buffer+1, data, strlen(data)+1);
				fd = openfile("wb");
				if (fd)
				{
					fwrite(buffer, 1, strlen(data)+1, fd);
					printf("Destination address set to: %s\n", data);
					fclose(fd);
				}
			break;
			case 2:
				memset(&datagram1, 0, sizeof(datagram1));
				memset(&datagram2, 0, sizeof(datagram2));
                                datagram1.record_id = 1;
                                datagram1.final_clock = 0;
                                datagram1.original_clock = 0;
                                datagram1.source_ip = 0;
                                datagram1.destination_ip = 0;
                                printf("Enter message without space:\n");
				scanf("%256s", datagram1.data);
				while( (ch = fgetc(stdin)) != EOF && ch != '\n' ){}	//clear stream
				//buffer[0] = 2;
				//memcpy(buffer+1, data, strlen(data)+1);
				fd = openfile("wb");
				if (fd)
				{
                                        fwrite((&datagram1), 1, sizeof(datagram1), fd);
                                        fclose(fd);
                                        fd = openfile("rb"); 
                                        fread(&datagram2, 1, sizeof(datagram2), fd);
                                        sendToMonitor((char *)&datagram2);
					printf("Message sent: %s\n", datagram2.data);
					fclose(fd);
				}
				break;
			case 3:
				printf("Data Read:\n");
				fd = openfile("rb");
				if (fd)
				{      
					count = fread((&datagram3), 1, sizeof(datagram3),fd);
					if (!count)
						printf("No data read\n");
					else
                                        {
                                                if(datagram3.record_id == 1)
                                                 	printf("%s\n", datagram3.data);
 						else
						{
                                                	printf(" Got Acknowledgement and sent to Monitor \n ");
						 	sendToMonitor((char *)&datagram3);    
						}
                                        }
					fclose(fd);
				}
				break;
			case 4:
				printf("Enter Monitor address:");
				{
					scanf("%s", monitorAddress);
				}
				break;
			case 5:
				{
					exit(0);
				}
				break;
			default :
				printf("This option is not available, please try available option.\n");
				break;
		}
	}

}
