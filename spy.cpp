//#define _GNU_SOURCE    
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
using namespace std;

int initSocketClient(char *host, short port){
  int sock, val;
  struct sockaddr_in serv; //ma structure d'adresse pour le serveur
  //initialisation socket
  sock=socket(AF_INET, SOCK_STREAM,0);
  if (sock==-1) {
    printf("Erreur d'utilisation de la fonction socket\n");
    return -1; // fin de fonction main
  }

  //initialisation structure d'adresse
  struct hostent *info;
  info=gethostbyname(host);
  serv.sin_family=AF_INET;
  serv.sin_port=htons(port);
  serv.sin_addr.s_addr=*((uint32_t *)info->h_addr);

  //tentative de connexion
  val=connect(sock, (struct sockaddr *) &serv, sizeof(struct sockaddr_in));
  if (val==-1) {
    printf("Erreur d'utilisation de la fonction connect\n");
    return -1; // fin de fonction main
  }

  //le socket est retourné en cas de succès
  return sock;
}

void connection(int sock){
  write(sock, "1",1);
}


void getReponse(int sock){
  bool quit=false;
  char reponse[1000];
  int i;
  while(!quit){
    i=read(sock, reponse, 1);
    if(i>0){
      if(!strcmp(reponse,"C")){//Demande de connection C PORT IP
	cout<<"demande de connection"<<endl;
	//Connexion Spy controleur.
      }
      else if(!strcmp(reponse,"S")){//STOP
	cout<<"Spy Stop"<<endl;
	quit=true;
      }
      else{
	cout<<"Erreur: "<<reponse[0]<<endl;
      }
    }
  }
}


int main(int args, char *arg[]){
  //initialisation du serveur
  int sock=initSocketClient(arg[1], atoi(arg[2]));
  if (sock==-1) return -1; //en cas d'echec de l'initialisation
  connection(sock);  
  getReponse(sock);  
  close(sock);
  return 0;
 }
  
