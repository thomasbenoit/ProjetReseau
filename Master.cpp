//#define _GNU_SOURCE    

/*g++ -pthread -std=c++11 -o Master Master.cpp*/


#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <iostream>
#include <map>
#include <string.h>

using namespace std;

struct pc{
  int sock;
  string nomhote;
};


multimap<int,pc> listespy;//<numsalle,pc>
map<int,pc> listecontrolleur;//<numsalle,pc>



int initSocketServeur(short port){
  int sock, val;
  struct sockaddr_in stad; //ma structure d'adresse pour le serveur
  sock=socket(AF_INET, SOCK_STREAM,0);
  if (sock==-1) {
    printf("Erreur d'utilisation de la fonction socket\n");
    return -1; // fin de fonction main
  }

  stad.sin_family=AF_INET;
  stad.sin_port=htons(port);
  stad.sin_addr.s_addr=INADDR_ANY;
  val=bind(sock, (struct sockaddr *) &stad, sizeof(struct sockaddr_in));
  if (val==-1) {
    printf("Erreur d'utilisation de la fonction bind\n");
    return -1; // fin de fonction main
  }
  val=listen(sock, 5);
  if (val==-1) {
    printf("Erreur d'utilisation de la fonction listen \n");
    return -1; // fin de fonction main
  }
  return sock;
}


int lireMessage(int client, char *buff, int max){
    char c='a'; //c initialisé a n'importe quoi sauf \n
    int i=0, n;
    //lire la quantité demandée
    while ((c!='\n')&&(i<max)) {
      n=read(client, &c, 1);
      if (n==0) return 0;
      if ((c!='\n')&&(c!='\r')) buff[i++]=c;
    }
    buff[i]=0;
    return strlen(buff);
}


void *gereSpy(void *arg){ 
  //Gere les deconnexion
  //Gere les demande de co sur le controleur

  int i, n, fdMax;
  char buffer[10];
  fd_set fd;
  struct timeval tv;
  tv.tv_sec=1;
  tv.tv_usec = 0;
  while(1){
    FD_ZERO(&fd);
    for (auto it=listespy.begin();it!=listespy.end();it++){
      FD_SET(it->second.sock, &fd);
    }
    fdMax=0;

    for (auto it=listespy.begin();it!=listespy.end();it++) 
      if (it->second.sock>fdMax) fdMax=it->second.sock;
    fdMax++;

    //attente qu'un client envoie un message ou se déconnecte
    
    n=select(fdMax, &fd, NULL, NULL, &tv);

    //Changement
    if (n>0){
      for (auto it=listespy.begin();it!=listespy.end();it++) {
	if (FD_ISSET(it->second.sock, &fd)) {
	  n=lireMessage(it->second.sock, buffer, 10);
	  if (n==0) {//Deco Client
	    close(it->second.sock);
	    listespy.erase(it);
	    cout<<"Client Sup !"<<endl;
	        
	  }
	}
      }
    }

  }
}



void connectSpy (int salle,int s,string nomh){
  struct pc tmp;
  tmp.nomhote=nomh;
  tmp.sock=s;
  listespy.insert(pair<int,pc>(s,tmp));
}


int lireType(int client){
  char *c=new char('a'); //c initialisé a n'importe quoi sauf \n
  int i=0, n;
  n=read(client, c, 1);
  if (n==0) return -1;
  else return atoi(c);
}


int main(int args, char *arg[]){
  //initialisation du serveur
  int sock,type;
  pthread_t T1;
  pthread_mutex_t verr;
  sock=initSocketServeur(atoi(arg[1]));
  if (sock==-1) return -1; //en cas d'echec de l'initialisation

  //variables pour accepter une connexion
  struct sockaddr_in stclient; //ma structure d'adresse pour le client
  socklen_t taille=sizeof(struct sockaddr_in);
  int client; //descripteur pour le client  

  //initialisatio du verrou
  pthread_mutex_init(&verr, NULL);
  pthread_create(&T1, NULL, gereSpy, NULL);
 
  //attente de clients puis gestion des requêtes
  while (1) {
    type=-1;
    client=accept(sock, (struct sockaddr *) &stclient, &taille);
    printf("Un client vient de se connecter : \n");//, h->h_name);
    type=lireType(client);
    switch(type){
    case 1 : connectSpy(12,client,"info27-13"); cout<<"Spy"<<endl; break;
    case 2 : cout<<"futur Controleur"<<endl; break;
    case 3 : cout<<"futur Observateur"<<endl; break;
    case -1: cout<<"Erreur"<<endl; break;
    }
  }
  return 0;
}
  
