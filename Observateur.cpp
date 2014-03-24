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
  write(sock, "3",1);
}



int main(int args, char *arg[]){
  /*Partie Initialisation de la Connexion à Master*/
  int sock=initSocketClient(arg[1], atoi(arg[2]));
  if (sock==-1) return -1;
  connection(sock); 
  bool quit=false;
  string s="a";


  /*Boucle d'intéraction*/
  while(quit==false){
    cin >> s;
    if(!strcmp(s.c_str(),"STOP")){
      quit=true;
    }
    else if(!strcmp(s.c_str(),"C")){//Connexion Controleur-Spy
      string test="GET";
      cout<<"NUM SALLE ?"<<endl;
      cin >> s;
      write(sock,test.c_str(),test.size());
      
    }
    else if(!strcmp(s.c_str(),"P")){//Prendre les screens des Spy connecté
     
    }
    else if(!strcmp(s.c_str(),"TEST")){//Fonction de test BRODCAST r à tout les spy

    }
    else if(!strcmp(s.c_str(),"A")){//BUG NIVEAU BASH
   
    }
    else if(!strcmp(s.c_str(),"M")){//Envoie un mesage

    }
    else if(!strcmp(s.c_str(),"O")){//Execute une commande
   
    }
    else {
      cout<<"Commande inconnu : "<<s<<endl;
    }

  }
  

  close(sock);
  return 0;
}
  
