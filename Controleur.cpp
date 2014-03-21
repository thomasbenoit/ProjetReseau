
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
#include <map>


using namespace std;

map<int,int> listespy;//<numpc,socket>


/*
 * Coté Client
 */


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
  write(sock, "2",1);
}


void getReponse(int sock){
  char reponse[100];
  int i;
  i=read(sock, reponse, 99);
  reponse[i]=0;
  printf("Reponse : %s\n", reponse);
}




/*
 * Coté Serveur
 */


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
      FD_SET(it->second, &fd);
    }
    fdMax=0;

    for (auto it=listespy.begin();it!=listespy.end();it++) 
      if (it->second>fdMax) fdMax=it->second;
    fdMax++;

    //attente qu'un client envoie un message ou se déconnecte
    
    n=select(fdMax, &fd, NULL, NULL, &tv);

    //Changement
    if (n>0){
      for (auto it=listespy.begin();it!=listespy.end();it++) {
	if (FD_ISSET(it->second, &fd)) {
	  n=lireMessage(it->second, buffer, 10);
	  if (n==0) {//Deco Client
	    close(it->second);
	    listespy.erase(it);
	    cout<<"Client Sup !"<<endl;	        
	  }
	}
      }
    }
  }
}



int initSocketServeur(short port){
  int sock, val;
  struct sockaddr_in stad; //ma structure d'adresse pour le serveur
  sock=socket(AF_INET, SOCK_STREAM,0);
  if (sock==-1) {
    printf("Erreur d'utilisation de la fonction socket serveur\n");
    return -1; // fin de fonction main
  }

  stad.sin_family=AF_INET;
  stad.sin_port=htons(port);
  stad.sin_addr.s_addr=INADDR_ANY;
  val=bind(sock, (struct sockaddr *) &stad, sizeof(struct sockaddr_in));
  if (val==-1) {
    printf("Erreur d'utilisation de la fonction bind serveur\n");
    return -1; // fin de fonction main
  }
  val=listen(sock, 5);
  if (val==-1) {
    printf("Erreur d'utilisation de la fonction listen serveur\n");
    return -1; // fin de fonction main
  }
  return sock;
}


void* serv(void* arg){
  while (1) {
    cout<<"boucle"<<endl;
    int sock=*(int *)arg;
    struct sockaddr_in stclient; //ma structure d'adresse pour le client
    socklen_t taille=sizeof(struct sockaddr_in);
    int client; //descripteur pour le client  

    client=accept(sock, (struct sockaddr *) &stclient, &taille);
    cout<<"Un client vient de se connecter sock: "<<client<<endl;

    struct hostent *h;
    h=gethostbyaddr((void *)&stclient.sin_addr.s_addr, 4, AF_INET);

    string tmp=string(h->h_name);
    tmp=tmp.substr(7,2);
    cout<<tmp<<endl;
    //Traite le cas du local host
    if(strcmp(tmp.c_str(),"lh")){
      listespy.insert(pair<int,int>(atoi(tmp.c_str()),client));
    }
    else{
      listespy.insert(pair<int,int>(0,client));
    }
  }
}


int main(int args, char *arg[]){
  /*Partie Initialisation de la Connexion à Master*/
  int sock=initSocketClient(arg[1], atoi(arg[2]));
  if (sock==-1) return -1;
  connection(sock); 

  /*Partie Initialisation du Serveur pour Acceuillir les potentiels Spy*/
  int sockserv;
  pthread_t T1,T2;
  pthread_mutex_t verr;
  string tmp;
  sockserv=initSocketServeur(atoi(arg[3]));
  if (sockserv==-1) return -1; //en cas d'echec de l'initialisation

  cout<<"Pas de Crash"<<endl;

  pthread_create(&T2, NULL, serv, (void*)&sockserv);
  pthread_create(&T1, NULL, gereSpy, NULL);
 

  bool quit=false;
  string s="a";
  while(quit==false){
    cin >> s;
    if(!strcmp(s.c_str(),"STOP")){
      quit=true;
    }
    else if(!strcmp(s.c_str(),"GET")){
      string test="GET";
      test+=(string)(arg[3]);
      write(sock,test.c_str(),7);
      
    }
    else if(!strcmp(s.c_str(),"SCR")){
      cout<<"demande de screen (Pas encore implementé)"<<endl;
    }
    else if(!strcmp(s.c_str(),"SEN")){
      for(auto it=listespy.begin();it!=listespy.end();it++){
	cout<<"Socket: "<<it->second<<endl;
	write(it->second,"r",1);
      }
      cout<<"Fin Transmision"<<endl;
    }
  }
  

  close(sock);
  return 0;
}
  
