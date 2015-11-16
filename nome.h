/*
Desenvolvido por Marcos Alvares (socram85@linuxmail.org)
			CORISCO(2004)
*/


char *pegar_nome(char *ip)
    {
    struct hostent *he;
    int i=0;
    
    he = gethostbyname(ip);
    if(he == 0)
	{
	return "@";
	}
    he=gethostbyaddr(he->h_addr_list[0],he->h_length,he->h_addrtype);
    if(he == 0)
	{
	return "@";
	}

    else return he->h_name;
    }
