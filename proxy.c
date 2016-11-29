//Includes

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>

//Main
int main(int argc, char *argv[]){
	//initialise variables
	
	//Validate Socket
	if(argc !=2 || atoi(argv[1]) < 1025 || atoi(argv[1]) > 65535 ){
		puts("Please Enter Valid Socket");
		exit(1);
	}

	puts("Valid Socket");

	//Create Socket
	/*	Socket
		Bind
		Listen
	*/
	int fdmain = 0;		//File Desc. of main socket

	struct sockaddr_in proxaddr;
	bzero(&proxaddr, sizeof(proxaddr));
    proxaddr.sin_family = AF_INET;
    proxaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    proxaddr.sin_port = htons(atoi(argv[1]));


	if((fdmain = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		puts("Socket Error");
    	exit(1);
	}
	puts("Socket sucessfully created");

	
    if((bind(fdmain, (struct sockaddr *) &proxaddr, sizeof(proxaddr)))==-1){
    	puts("Bind Error");
    	exit(1);
    }
	puts("Bind Sucessful");

	if((listen(fdmain, 1)) == -1){
		puts("Listen Error");
		exit(1);
	}
	puts("Listen Sucessful");

	char badResp[1000] = "HTTP/1.1 400 OK\n"
			"Content-Type: text/html/n"
			"Content-Length: 105\n"
			"Accept-Ranges:bytes\n"
			"Connection: close\n"
			"\n"
			"<html><body>Error: 400<br/>Bad Request</body></html>";
	int j=0;

	for(j ;j>=0 ;j++ ){
			
		
		//Accept
		struct sockaddr_in cliaddr;
		//bzero(&cliaddr, sizeof(cliaddr));

		int fdaccept;	//File Desc. of client conn. accept socket
		int clilen = sizeof(cliaddr);
		fdaccept = accept(fdmain, (struct sockaddr *) &cliaddr, &clilen);
		printf("FD accept: %d\n",fdaccept);
		if(fdaccept == -1){
			puts("Accept Error");
			exit(1);
		}
		puts("Connection Accepted");
		printf("\n\n Connection *** %d *** [%x, %d]\n", j, cliaddr.sin_addr.s_addr, cliaddr.sin_port);

		char requestBuf[1000000];
		bzero(requestBuf, 1000000);

		read(fdaccept, requestBuf, 1000000);
		printf("Request:  %s\n", requestBuf);



		//break;
		//Parsing

/*
		char requestbuf[2000000]=
		"GET http://gaia.cs.umass.edu:434343/wireshark-labs/HTTP-wireshark-file1.html HTTP/1.1\n"
		"Host: gaia.cs.umass.edu\n"
		"Connection: keep-alive\n"
		"Cache-Control: max-age=0\n"
		"Upgrade-Insecure-Requests: 1\n"
		"User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.143 Safari/537.36\n"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/
		/*;q=0.8\n"
		"Accept-Encoding: gzip, deflate, sdch\n"
		"Accept-Language: en-GB,en-US;q=0.8,en;q=0.6\n"
		"If-Modified-Since: Fri, 14 Oct 2016 05:59:01 GMT\n";
*/		
		char reqDomain[1024];
		bzero(reqDomain, 1024);
	
		char reqPort[10];
		bzero(reqPort, 10);
	
		char reqType[50];
		bzero(reqType, 50);
	
		char reqUriPrefix[10];
		bzero(reqUriPrefix, 10);
	
	
		char reqUri[100];
		bzero(reqUri, 100);
		
		int firstSpace = 0;
		int slashcnt = 0;
		int slash2 = 0;
		int i=0;
		for(i=0; i < strlen(requestBuf); i++){
			if(requestBuf[i] == ' '){
				firstSpace = i + 1;
				memcpy(reqType, requestBuf, i);
				break;
			}
	
		} //End For

		
		if(!strstr(reqType, "GET")){
			write(fdaccept, badResp, 1000);
			printf("Bad Request sent to Browser\n");
		} else {

			for(i=firstSpace; i < strlen(requestBuf); i++){
				if(requestBuf[i] == ':'){ 
					strncpy(reqUriPrefix, requestBuf + firstSpace, i - firstSpace);
					break;
				}


		
			} //End For
		
			for(i=0; i < strlen(requestBuf); i++){
				if(requestBuf[i] == '/'){
					slashcnt++; 
					if(slashcnt == 2){
						slash2 = i + 1;
					}
		
					if(slashcnt == 3){
						strncpy(reqUri, requestBuf + slash2, i - slash2);
						break;
					}
				}
		
			} //End For
		
			int cnt=0;
			for(i = 0; i < strlen(reqUri); i++){
				cnt = i;
				if(reqUri[i] == ':'){ 
					memcpy(reqDomain, reqUri, i);
					strncpy(reqPort, reqUri + i + 1, strlen(reqUri) - i);
		
					break;
				}
		
			} //End For
		
			if(cnt == strlen(reqUri) - 1){
				strcpy(reqDomain, reqUri);
				strcpy(reqPort, "80");
		
			}
		
			printf("cnt: [%d]\n",cnt );	
			printf("Type: [%s]\n",reqType );
			printf("Prefix: [%s]\n",reqUriPrefix );
			printf("Uri: [%s]\n",reqUri );
			printf("Domain: [%s]\n",reqDomain );
			printf("Port: [%s]\n",reqPort );
		
			//getAddrInfo()
			struct addrinfo hints, *serverinfo;
        	memset(&hints, 0, sizeof(hints));
        	hints.ai_family=INADDR_ANY;
	        hints.ai_socktype = SOCK_STREAM;

	        int getadri = getaddrinfo(reqDomain, reqPort, &hints, &serverinfo);


			if(getadri != 0){		//"GET ADDR INFO !=0"
				write(fdaccept, badResp, 1000);
				printf("Bad Request sent to Browser\n");
				//Send Error to Browser
			} else {
				printf("GET Addr: %x\n",serverinfo->ai_canonname);

				//Connect to server
				int fdserver = 0;	//File Desc. of dest. server socket
				fdserver = socket(AF_INET, SOCK_STREAM, 0);

				if(fdserver == -1){		//"Not connected"
					printf("Server socket creation failed\n");
					exit(1);
					//Send Error to Browser
				} else {
					int sconnect = 0;
					sconnect = connect(fdserver, serverinfo->ai_addr, serverinfo->ai_addrlen);

					if(sconnect == -1){
						write(fdaccept, badResp, 1000);
						printf("Bad Request sent to Browser\n");
					} else {
						//read() and write()
						int sendState = send(fdserver, requestBuf, sizeof(requestBuf), 0);
						if(sendState == -1){
							write(fdaccept, badResp, 1000);
							printf("Bad Request sent to Browser\n");
						} else {
							char readResp[5000000];
							ssize_t readSize;
							while((readSize = read(fdserver, readResp, 5000000)) > 0){
								write(fdaccept, readResp, 5000000);
							}
						}
					}
				}
			} //End getAddrInfo() check
		} //End ("Type != GET and != HTTP")
	} // End for
} // End main