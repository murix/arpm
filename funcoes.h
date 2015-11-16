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
			sprintf(temp_ip,"%s\n",inet_ntoa(iptmp->ip) );
			sprintf(temp_hw,"%s\n", hwaddr_to_str(tmp->hw) );
			
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
		unsigned char hwa[ETH_ALEN];
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
