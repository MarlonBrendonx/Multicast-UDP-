#include <sys/types.h>   
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>   
#include <stdio.h>       
#include <stdlib.h>      
#include <string.h>      
#include <unistd.h>      
#include <pthread.h>

#define MAX_LEN  1024    
#define PORT_MIN 1024    
#define PORT_MAX 65535  
 
#define PROGRAM_NAME "Multicasting com threads"
#define AUTHOR \
  proper_name ("Marlon Brendo")

/*
-------------------------Header-----------------------------------------------------|

 AUTOR             : Marlon Brendo <marlonbrendo2013@gmail.com>
 DATA-DE-CRIAÇÃO   : 01/04/2021
 PROGRAMA          : Multicasting UDP em localhost utilzando threads
 DESCRIÇÃO 	   : Envia e recebe mensagem em localhost utilizando multicasting UDP
		     com threads

#-----------------------------------------------------------------------------------|

*/

//-------------------------Cores-----------------------------------------------------|


#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define BLUE    "\x1b[34m"
#define END     "\x1b[0m"

//-----------------------------------------------------------------------------------|	



typedef struct {
	
	int sock;
	struct sockaddr_in address;
	struct ip_mreq mc_req;
	

}st_receive;

void *Receive(void *args){
 	  
	  char recv_str[1024];     /* Buffer para receber a string */
  	  int recv_len;
	  unsigned int from_len; 
	  struct sockaddr_in from_addr; 

	  st_receive *actual_args = args;

	  while(1){          

		    /* Limpando buffer e structs */
		    memset(recv_str, 0, sizeof(recv_str));
		    from_len = sizeof(from_addr);
		    memset(&from_addr, 0, from_len);

		    /* Aguardando para receber um pacote */
		    if ((

			recv_len = recvfrom(

					actual_args->sock,
					recv_str, MAX_LEN, 0, 
			 		(struct sockaddr*)&from_addr, &from_len

					)) < 0) {

		      		perror(RED "[!]" END " recvfrom() falhou");

		      exit(0);

		    }

		    /* Recenbendo a menssagem e enviando
		       para output  */

		    printf(GREEN "[*]" END " Recebendo %d bytes de %s:\n", recv_len,
		    inet_ntoa(from_addr.sin_addr));

		    printf(GREEN "~" END " %s", recv_str);

	  }

	  /* Envia uma mensagem de DROP via setsockopt */
	  if ((

		setsockopt(

			 actual_args->sock,
			 IPPROTO_IP,
			 IP_DROP_MEMBERSHIP, 
	       		 (void*) &actual_args->mc_req,
			 sizeof(actual_args->mc_req)

		)) < 0) {

		    perror(RED "[!]" END " setsockopt() falhou");
	    exit(1);
	  }

	  free(args);
	  close(actual_args->sock);  

}




int main(int argc, char *argv[]) {

	  int isock;                   
	  unsigned short mc_port;    
	  int flag=1;	 
     
	  char *ch_mc_addr; 
          char ch_send_message[1024];

	  struct ip_mreq mc_req;
	  struct sockaddr_in address; 
	  
	  pthread_t thread_id;
	

	  ch_mc_addr  = argv[1];       /* arg 1: Endereço do IP Multicast */
	  mc_port     = atoi(argv[2]); /* arg 2: Número da porta Multicast */

	  if(argc != 3){

			printf(RED "[!]" END  " Parametro invalido:\n Use: <MULTICAST IP> <MULTICAST PORT>\n");
			exit(1);

	  }

	  if( mc_port < PORT_MIN ||  mc_port > PORT_MAX  ){

			printf( RED "[!]" END " Porta invalida: Use uma porta entre 1024 e 65535\n");
			exit(1);

	  }
	 
          /* Criando um socket UDP*/
          isock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	  
	  /* Seta a reulização da porta para permitir vários bind por host*/
	  if ((
		 setsockopt(
			 isock,
			 SOL_SOCKET,
			 SO_REUSEADDR,
			 &flag,
	       		 sizeof(flag)

	     )) < 0 ) {

		    perror(RED "[x]" END " setsockopt() falhou");

		    exit(1);
	  }


	  if ( isock < 0 ) {

		    perror(RED "[x]" END "socket() falhou");
		    exit(1);

	  }

	

	  /* Preenche com zero o endereço */
	  bzero( (&address),sizeof(address) );

	  address.sin_family      = AF_INET;		      // Atribuindo endereços da internet
	  address.sin_addr.s_addr = inet_addr(ch_mc_addr);    // Aceitando somente o endereço passado  
	  address.sin_port        = htons(mc_port);	      // Porta para comunicação,mantendo a ordem dos bytes


	  /* bind to multicast address to socket */
	  if ((
		bind(
		     isock,
		     (struct sockaddr *) &address, 
	       	     sizeof(address)
	      )) < 0) {
	    
			perror(RED "[x]" END " bind() falhou");
	    exit(1);
	  }

	  /* Construindo uma estrutura para registrar dinamicamente 
	     os host individuais em um grupo multicast  */

	  mc_req.imr_multiaddr.s_addr = inet_addr(ch_mc_addr);
	  mc_req.imr_interface.s_addr = htonl(INADDR_ANY);



	  /* Usando o setsockop com a opção IP_ADD_MEMBERSHIP para entrar em um grupo multicast
	     em uma rede local */

	  if ((

		setsockopt(
			isock,
			IPPROTO_IP,
			IP_ADD_MEMBERSHIP, 
	       		(void*) &mc_req, sizeof(mc_req)
		)) < 0 ) {

	    		perror(RED "[x]" END " setsockopt() failed");

	    exit(1);
 	  }
		
	  st_receive *args=malloc(sizeof *args);
		
    	  args->sock    = isock;
	  args->address = address;
	  args->mc_req.imr_multiaddr.s_addr = inet_addr(ch_mc_addr);   
	  args->mc_req.imr_interface.s_addr = htonl(INADDR_ANY);
		

	 
	  pthread_create(&thread_id, NULL,Receive,args);
	  
    	   
    	 
	  printf("Envie uma mensagem,Ctrl-c para sair\n");
	  printf(":>");
	  
	  bzero( (&ch_send_message),sizeof(ch_send_message) );


	  while ( fgets(ch_send_message, MAX_LEN, stdin) ){

		
		
	    	if( sendto(
		
			   isock,
			   ch_send_message,
			   strlen(ch_send_message),
			   0,
			   (struct sockaddr *) &address,
			   sizeof(address)

		    ) != strlen(ch_send_message) 
		){

			   perror("Numero de bytes incorretos:");
			   exit(1);

		}
		

	    	bzero( (&ch_send_message),sizeof(ch_send_message) );
		printf("\n");
	  	printf(":>");

	  }

	  close(isock);  

	  exit(0);
}

