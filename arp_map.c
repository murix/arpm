/*
*         Programa pra scaniar a rede
*                 Desenvolvido por Marcos Alvares (socram85@linuxmail.org)
*                                       CORISCO 2004                                                    *              Argumentoa
*                          ./arpm adaptador login senha 
*                      EX:     ./arpm eth0 marcos socram            
*/
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/if_ether.h>
#include <unistd.h>
#include <fcntl.h>
#include <curl/curl.h>
#include "nome.h"
#include "post.h"


#define ARP_ETHER       1
#define ARP_REQUEST     1
#define ARP_REPLY       2


/* Variaveis globais */
unsigned long netmask, broadcast;
unsigned char hwaddr[ETH_ALEN];
struct sockaddr sock;

#include "includes.h"
#include "funcoes.h"


main (int argc,char *argv[]) { 
		struct ifreq ifr;
		struct sockaddr_in sin;
		unsigned long int ip;
		int fd, ret;
		char *dev, *login, *senha;

		if(argc != 4)
			{
			printf("\n\n");
			printf(" ARP-MAP  (CORISCO Linux Unit 2004)\n\n");
			printf("      Use : %s <adaptador> <login> <senha>\n\n",argv[0]);
			exit(0);
			}
		else 
			{
			int tam = strlen(argv[1]);
			dev = (char *) malloc(tam); strcpy(dev,argv[1]);
			}

		//Setando o socket a interface e pegado os dados
		strcpy(sock.sa_data,dev);
		sock.sa_family = AF_INET;
        
		fd = socket(AF_INET, SOCK_PACKET, htons(ETH_P_ARP));
		if (fd==-1) 
			{
			perror("Socket: "); 
			exit (1);
			}

		// HW Addr.
		strcpy(ifr.ifr_name,dev);
		ret = ioctl(fd,SIOCGIFHWADDR,&ifr);
		if (ret==-1) 
			{
			perror("Erro ao pegar MAC local");
			exit (1);
			}
	
		memcpy(hwaddr,ifr.ifr_hwaddr.sa_data,ETH_ALEN);

		// IP.
		ret = ioctl(fd,SIOCGIFADDR,&ifr);
	
		if (ret==-1) 
			{
			perror("Erro ao pegar o endereco de IP"); 
			exit (1);
			}
	
		memcpy(&sin,&ifr.ifr_addr,sizeof(struct sockaddr_in)); 
		ip = sin.sin_addr.s_addr;

		// Mascara de rede.
		ret = ioctl(fd,SIOCGIFNETMASK,&ifr);
		if (ret==-1) 
			{
			perror("Erro ao pegar a mascara"); 
			exit (1);
			}
		memcpy(&sin,&ifr.ifr_netmask,sizeof(struct sockaddr_in)); 
		netmask = sin.sin_addr.s_addr;

		// Broadcast.
		ret = ioctl(fd,SIOCGIFBRDADDR,&ifr);
		if (ret==-1) 
			{
			perror("Error getting broadcast"); 
			exit (1);
			}
		
		memcpy(&sin,&ifr.ifr_broadaddr,sizeof(struct sockaddr_in)); 
		broadcast = sin.sin_addr.s_addr;
	
		if(!fork())
		while(1) 
			{ 
			head = tail = NULL;
			netmap (fd,ip,argv[2],argv[3]) ;
			sleep(120);
			head = tail = NULL;
			}
}//main


