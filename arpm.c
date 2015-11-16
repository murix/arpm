/***************************************************************************
 *   Copyright (C) 2004 by                                                 *
 *    Marcos Alvares(socram85@linuxmail.org)                               *
 *    Murilo Pontes (murilo_pontes@yahoo.com.br)                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


/*
  Para compilar
  gcc arpm.c -lsqlite -lcurl -Wall -O3 -static -o arpm.bin

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
#include <sqlite.h>
#include <time.h>

#define URL "http://www.wi.not.br:81/index.php?op=atualiza_action"
#define ARP_ETHER       1
#define ARP_REQUEST     1
#define ARP_REPLY       2

/* Variaveis globais */
unsigned long netmask, broadcast;
unsigned char hwaddr[ETH_ALEN];
struct sockaddr sock;

struct pkt_struct {
        unsigned char eth_dst[6];
        unsigned char eth_src[6];
        unsigned short eth_proto;
        unsigned short int arp_hw_type;
        unsigned short int arp_proto;
        unsigned char arp_hw_len;
        unsigned char arp_proto_len; 
        unsigned short arp_oper;
        unsigned char arp_hw_src[6];
        unsigned char arp_ip_src[4];
        unsigned char arp_hw_dst[6];
        unsigned char arp_ip_dst[4];
};

struct iplist_struct {
        unsigned long int ip;
        struct iplist_struct * next;
};

struct list_struct {
        unsigned char hw[ETH_ALEN];
        struct iplist_struct * iplist;
        struct iplist_struct * lastip;
        struct list_struct * next;
} * head = NULL, * tail = NULL;

char *pegar_nome(char *ip){
    struct hostent *he;

    he=gethostbyname(ip);
    if(he == 0) return "@";

    he=gethostbyaddr(he->h_addr_list[0],he->h_length,he->h_addrtype);
    if(he == 0) return "@";

    else 
    return he->h_name;
}
    
    
void send_post(char *login , char *senha, char *lista_ip , char *lista_hw,char *nomes){
	struct HttpPost *post = NULL;
	struct HttpPost *last = NULL;
	CURL *curl_handle;

	curl_formadd( (struct curl_httppost **)&post ,(struct curl_httppost **) &last, CURLFORM_COPYNAME, "login",    CURLFORM_PTRCONTENTS, login, CURLFORM_END);
	curl_formadd((struct curl_httppost **)&post,(struct curl_httppost **) &last, CURLFORM_COPYNAME, "senha",   CURLFORM_PTRCONTENTS, senha, CURLFORM_END);	
	curl_formadd((struct curl_httppost **)&post,(struct curl_httppost **) &last, CURLFORM_COPYNAME, "lista",    CURLFORM_PTRCONTENTS, lista_ip, CURLFORM_END);
	curl_formadd((struct curl_httppost **)&post,(struct curl_httppost **) &last, CURLFORM_COPYNAME, "hw",     CURLFORM_PTRCONTENTS, lista_hw, CURLFORM_END);
	curl_formadd((struct curl_httppost **)&post,(struct curl_httppost **) &last, CURLFORM_COPYNAME, "nomes",     CURLFORM_PTRCONTENTS, nomes, CURLFORM_END);
	curl_formadd((struct curl_httppost **)&post,(struct curl_httppost **)&last,CURLFORM_COPYNAME, "submit",    CURLFORM_COPYCONTENTS, "send",CURLFORM_END);
	
	// inicializando a estrutura correspondente ao tipo curl em curl/curl.h
	curl_handle = curl_easy_init();

	// Setando qual a url que irah receber o post
	curl_easy_setopt(curl_handle, CURLOPT_URL, URL);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPPOST,(struct curl_httppost **) post);
	curl_easy_perform(curl_handle);

	/* Apagando a estrutura */
	curl_easy_cleanup(curl_handle);
	
	/* Liberando a memoria alocada com a estrutura de curl */
	curl_formfree((struct curl_httppost *)post);
}


	
void criar_db(char *ip, char *hw, char *nome){
	sqlite *banco;
	int i;
	char query[300];
	
	for (i=0;i<300;i++) query[i]='\0';

	sprintf (query, "insert into cliente values( \"%s\" , \"%s\" , \"%s\" );",ip , hw , nome );

	banco = sqlite_open("clientes.db",0,NULL);

	sqlite_exec(banco, query,NULL,NULL,NULL);

	sqlite_close(banco);
}
	

	
char * hwaddr_to_str (unsigned char * str) 
	{
	static char tmp[20];
	sprintf(tmp,"%02X:%02X:%02X:%02X:%02X:%02X",str[0],str[1],str[2],str[3],str[4],str[5]);
	return tmp;
	}

unsigned int hexstr_to_int(char *cptr) 
	{
	unsigned int i, j = 0;
        
	while (cptr && *cptr && isxdigit(*cptr)) 
		{
		i = *cptr++ - '0';
		if (9 < i) i -= 7;
		j <<= 4;
		j |= (i & 0x0f);
		} 
	return(j);
	}       

unsigned char * str_to_hwaddr (char * str) 
	{
	unsigned char tmp[2], strbuf[17], t[2];
	static unsigned char * buf, * tt;
	int e, i;

	buf = (unsigned char *) malloc(6);
	bzero(t,2); bzero(tmp,2); bzero(strbuf,17); bzero(buf,6);
	strncpy(strbuf,str,17); strbuf[17]='\0';
	tt = buf;
        
	e = 0;
	for (i=0; i < strlen(strbuf); i++) 
		{
		if ((strbuf[i]==':') && (e==0)) continue;
		tmp[e] = strbuf[i]; e++;
		if (e==2) 
			{
			unsigned int a;
			a = hexstr_to_int(tmp);
			memcpy(tt,&a,1); tt++;
			bzero(tmp,2); e = 0;
			}
		}
	return buf;
	}



void show_list (char *login , char *senha) 
	{

	
	
	int i;
	struct list_struct * tmp, * tmp2;
	tmp = head;
	//declarando as variaveis para armazenar a lista
	char lista_ip[1000];
	char lista_hw[1000]; 
	char lista_nomes[1000];
	char temp_ip[50];
	char temp_hw[50];
	char temp_nome[50];
	for(i=0;i<50;i++) temp_ip[i] = temp_hw[i]= temp_nome[i] = '\0';
	
	for(i=0;i<1000;i++) lista_ip[i] = lista_hw[i]=lista_nomes[i] = '\0';
	
	
	while (tmp!=NULL) 
		{
		struct iplist_struct * iptmp;
		iptmp = tmp->iplist;
                
		
		while (iptmp!=NULL) 
			{
			for(i=0;i<50;i++) temp_ip[i] = temp_hw[i]= temp_nome[i] = '\0';
						
			sprintf(temp_nome,"%s\n" ,pegar_nome((char *) inet_ntoa(iptmp->ip) ) );
			sprintf(temp_ip,"%s\n",((char *)inet_ntoa(iptmp->ip)) );
			sprintf(temp_hw,"%s\n", hwaddr_to_str(tmp->hw) );
			criar_db(temp_ip,temp_hw,temp_nome);
			
			strcat(lista_nomes,temp_nome);
			strcat(lista_ip,temp_ip);
			strcat(lista_hw,temp_hw);
			iptmp = iptmp->next;
			}
		free(iptmp); tmp2 = tmp->next;
		free(tmp); tmp = tmp2;
		} 
		send_post(login,senha,lista_ip,lista_hw,lista_nomes);
		
		return;
	}
              

void add_to_list (unsigned long int ip, unsigned char * hw) {
        struct list_struct * tmp;
        struct iplist_struct * iptmp;
        tmp = head;
        while (tmp) 
		{
                if ((hw[0]==tmp->hw[0]) && (hw[1]==tmp->hw[1]) && (hw[2]==tmp->hw[2]) && (hw[3]==tmp->hw[3]) &&
                        (hw[4]==tmp->hw[4]) && (hw[5]==tmp->hw[5])) break;
                tmp = tmp->next;
        	}
		
        if (!tmp) 
		{                     
                if ((tmp = (struct list_struct *) malloc(sizeof(struct list_struct))) == NULL) {
                        printf("\n malloc error. \n"); 
			exit (1);
                }
                
	if ((iptmp = (struct iplist_struct *) malloc(sizeof(struct iplist_struct)))== NULL) 
 		{
                printf("\n malloc error. \n"); 
		exit (1);
                }
	iptmp->ip = ip;
	iptmp->next = NULL;
	tmp->iplist = iptmp;            
	tmp->lastip = iptmp;
	tmp->next = NULL;
	memcpy(tmp->hw,hw,ETH_ALEN);
	if (tail) 
		{
		tail->next = tmp;
		tail = tmp;
		} 
	}
	else 
		{                        
		if ((iptmp = (struct iplist_struct *) malloc(sizeof(struct iplist_struct))) == NULL) 
			{
			printf("\n malloc error. \n"); 
			exit (1);
			}
		iptmp->ip = ip;
		iptmp->next = NULL;
		tmp->lastip->next = iptmp;
		tmp->lastip = iptmp;
		}
	if (!head) head = tail = tmp; 
}



void sendarp (int fd, unsigned char * h_source, unsigned char * h_dest,
                      unsigned char * arp_src, unsigned char * arp_dst, 
                      unsigned long int ip_source, unsigned long int ip_dest, unsigned char op)
	{
	struct pkt_struct pkt;

	// cabecalho Ethernet 
	memcpy(&pkt.eth_dst,h_dest,ETH_ALEN);
	memcpy(&pkt.eth_src,h_source,ETH_ALEN);
	pkt.eth_proto = htons(ETH_P_ARP);
	
	// Cabecalho ARP 
	pkt.arp_hw_type = htons(ARP_ETHER);
	pkt.arp_proto = htons(ETH_P_IP);
	pkt.arp_hw_len = ETH_ALEN;
	pkt.arp_proto_len = 4;
	pkt.arp_oper = htons(op);

	if (arp_src==0) bzero(&pkt.arp_hw_src,ETH_ALEN);
	else memcpy(&pkt.arp_hw_src,arp_src,ETH_ALEN);
	if (arp_dst==0) bzero(&pkt.arp_hw_dst,ETH_ALEN);
	else memcpy(&pkt.arp_hw_dst,arp_dst,ETH_ALEN);

	memcpy(&pkt.arp_ip_src,&ip_source,4);
	memcpy(&pkt.arp_ip_dst,&ip_dest,4);

	if ( (sendto(fd,&pkt,sizeof(pkt),0,&sock,sizeof(sock))) < 0) perror("Erro ao enviar ARP");

}


void netmap (int fd, unsigned long int start_ip,char *login,char *senha) 
	{ 
	sqlite *banco;
	char query3[]="drop table cliente;";
	char query4[]="create table cliente(ip text , hw text , nome text);";
	
	banco = sqlite_open("clientes.db",0,NULL);
	sqlite_exec(banco, query3,NULL,NULL,NULL);
	sqlite_exec(banco, query4,NULL,NULL,NULL);
	
	sqlite_close(banco);

	unsigned long int ip, ip_s;
	struct pkt_struct * pkt;
	int i;
      
	ip_s = start_ip;
	ip = ip_s & netmask;

	i = fcntl(fd,F_GETFL);
	if ((fcntl(fd,F_SETFL, i | O_NONBLOCK))<0) 
		{
		perror("FCNTL"); exit (1);
        	}

	pkt = (struct pkt_struct *) malloc(sizeof(struct pkt_struct));
	bzero(pkt,sizeof(struct pkt_struct));

	while (ip < broadcast) 
		{
		
		unsigned long int iptmp;
		//unsigned char hwa[ETH_ALEN];
		ip = ntohl(ip);
		ip = htonl(++ip);

		sendarp(fd,hwaddr,str_to_hwaddr("FF:FF:FF:FF:FF:FF"),hwaddr,0,ip_s,ip,ARP_REQUEST);
		usleep(10500);

		i = sizeof(sock);
		bzero(pkt,sizeof(struct pkt_struct));
		if ((recvfrom(fd,pkt,sizeof(struct pkt_struct),0,&sock,&i)) < 0) continue;

		memcpy(&iptmp,pkt->arp_ip_dst,4);
		if ((iptmp!=ip_s) || (ntohs(pkt->arp_oper)!=ARP_REPLY)) continue;          
     
		memcpy(&iptmp,pkt->arp_ip_src,4);
		add_to_list(iptmp,pkt->eth_src);
		} 
	show_list(login,senha);
	free (pkt);

	}

	
int main (int argc,char *argv[]) { 
		struct ifreq ifr;
		struct sockaddr_in sin;
		unsigned long int ip;
		int fd, ret;
		char *dev;

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
    return 0;
}//main



