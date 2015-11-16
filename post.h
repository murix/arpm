#define URL "http://www.wi.not.br:81/index.php?op=atualiza_action"
//#define URL "http://localhost/ftp/index.php?op=atualiza_action"
#include <time.h>

send_post(char *login , char *senha, char *lista_ip , char *lista_hw,char *nomes)
	{
	struct HttpPost *post = NULL;
	struct HttpPost *last = NULL;

	//FILE *htmlfile;
	CURL *curl_handle;
	// dia mes dia horas:min:seg ano\n\0 

//    printf("\n%s\n",lista_ip);
//    printf("\n%s\n",lista_hw);
//    printf("\n%s\n",nomes);

		
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
	
	// Setando informacoes do form
	//htmlfile = fopen("mq.htm", "w");
	
	curl_easy_setopt(curl_handle, CURLOPT_HTTPPOST,(struct curl_httppost **) post);
	//curl_easy_setopt(curl_handle, CURLOPT_FILE, htmlfile);
	
	curl_easy_perform(curl_handle);
	
	/* Apagando a estrutura */
	curl_easy_cleanup(curl_handle);
	
	/* Liberando a memoria alocada com a estrutura de curl */
	curl_formfree((struct curl_httppost *)post);
	}//send_post
