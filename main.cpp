#include "TLV.h"
#include <stdio.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_thread.h>
#define Port 1313
struct serInfo{
	SDL_mutex* mut;
	TCPsocket tcp;
	byte* cacheBuf;
	uint lenCacheBuf;
	SDL_Thread* thread;
	byte active;
};
int server(void* id){
	serInfo*ser=(serInfo*)id;
	IPaddress ip;
	ip.host=INADDR_ANY;
	ip.port=Port;
	ser->tcp=SDLNet_TCP_Open(&ip);
	ser->lenCacheBuf=128;
	ser->cacheBuf=(byte*)malloc(128);
	int res;
	TCPsocket list;
	while(ser->active){
		if(list=SDLNet_TCP_Accept(ser->tcp))
		{
			SDLNet_TCP_Send(list, &res, 1);
			res=SDLNet_TCP_Recv(list,ser->cacheBuf,ser->lenCacheBuf);
			if(res<=0){
				puts("Connection lost!");
				continue;
			}
			size len=decode<size>(ser->cacheBuf);
			if(ser->lenCacheBuf<len){
				free(ser->cacheBuf);
				len++;
				ser->cacheBuf=(byte*)malloc(ser->lenCacheBuf=len);
			}
			SDLNet_TCP_Send(list, &res, 1);
			res=SDLNet_TCP_Recv(list,ser->cacheBuf,ser->lenCacheBuf);
			if(res<=0){
				puts("Connection lost!");
				continue;
			}
			ser->cacheBuf[len] = 0;
			printf("Receive data with length %lu: %s\n",len,ser->cacheBuf);
			fflush(stdout);
			SDLNet_TCP_Close(list);
		}
	}
	return 0;
}
#define SDL_main main
int main(int argc, char **argv) {
	puts("Run as server(Y/n)");
	byte j;
	scanf("%c",&j);
	SDLNet_Init();
	if((j=='y')||(j=='Y')){
		puts("Please, enter count of server threads:");
		uint c;
		scanf("%u",&c);
		serInfo*si=(serInfo*)malloc(sizeof(serInfo)*c);
		for(uint i=0;i<c;i++){
			si[i].mut=SDL_CreateMutex();
			si[i].active = 1;
			si[i].thread = SDL_CreateThread(server, 0, si+i);
		}
		puts("Please, any key and enter to exit...");
		scanf("%*s");
		for(uint i=0;i<c;i++){
			si[i].active = 0;
			SDL_DetachThread(si[i].thread);
			SDL_DestroyMutex(si[i].mut);
			SDLNet_TCP_Close(si[i].tcp);
			if(si[i].cacheBuf){
				free(si[i].cacheBuf);
			}
		}
		free(si);
	}else{
		IPaddress ip;
		ip.port=Port;
		byte*buf=(byte*)malloc(0xffff+1);
		do{
			puts("Please,enter server IP address(in format x.x.x.x):");
			uint a,b,c,d;
			scanf("%u.%u.%u.%u",&a,&b,&c,&d);
			*((byte*)&ip.host)=a;
			*((byte*)&ip.host+1)=b;
			*((byte*)&ip.host+2)=c;
			*((byte*)&ip.host+3)=d;
			while(1){
				byte*data=buf;
				size h;
				puts("Send random data to server(1/0)?");
				scanf("%d",&a);
				if(a){
					puts("Please, enter size of data:");
					scanf("%lu",&h);
					byte*data=(byte*)malloc(h+1);
					*data=0;
					for(uint i=1;i<=h;i++){
						data[i]=rand();
					}
				}else{
					puts("Please, enter data for transmit(max len 0xFFFF):");
					fgetc(stdin);
					while(!fgets((char*)buf+1,0xffff,stdin));
					byte*g=buf+1;
					h=0;
					while(*(g++)){
						h++;
					}
					*buf=1;
					data=buf;
				}
				TCPsocket soc=SDLNet_TCP_Open(&ip);
				if(!soc){
					puts("Can not connect to this ip!");
					continue;
				}	
				byte j;
				byte*len=encode(h,&j);
				SDLNet_TCP_Recv(soc, &a, 1);
				int res=SDLNet_TCP_Send(soc,len,j);
				free(len);
				if(res<j){
					if(!*data){
						free(buf);
					}
					break;
				}
				SDLNet_TCP_Recv(soc, &a, 1);
				res=SDLNet_TCP_Send(soc,data,h);
				if(res<h){
					if(!*data){
						free(buf);
					}
					break;
				}
				if(!*data){
					free(buf);
				}
				SDLNet_TCP_Close(soc);
				printf("Transmited %d bytes!\n",h);
			}
			puts("Connection error!");
		}while(1);
	}
	puts("Goodbuy!");
	SDLNet_Quit();
	return 0;
}
