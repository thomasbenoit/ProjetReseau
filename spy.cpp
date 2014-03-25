#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fstream>
#include <map>
#include <iostream>
#include <pthread.h>
#include <signal.h>

using namespace std;

map<string,int>prog;//variable globale


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

void connection(int sock){//renvoi 1 lors de la connection
  write(sock, "1",1);
}

void printEcran(int sock){//fonction impression d'écran toute les 60 secondes
  int time=60;
  while(1){
    system("import -window root /tmp/image_ecran.jpg");//impression écran
    //Envoyer l'image ?
    sleep(time);
  }
} 

void messageEtudiant(int sock, string message){//zenity avec le message recu
  string test="zenity --warning --text="+message+" ";
  const char * c= test.c_str();
  system(c);
}


void controle(int sock, string message){//permet de lancer la commande sur l'ordinateur
  system(message.c_str());
}



void * alerte(void* arg){//permet de savoir si un programme de la liste est en fonction
  int sock=*(int *)arg;
  string tmp;
  

    const char * p1= "ps aux | grep -e ";
    const char * p2=" | grep -v color | wc -l >> /tmp/res_cmd.txt";

    while(1){
      system("rm /tmp/res_cmd.txt 2>/dev/null;touch /tmp/res_cmd.txt");
      for(map<string,int>::iterator it(prog.begin());it!=prog.end();it++){
	string c=p1+it->first+p2;
	const char * s=c.c_str();
	int rep=system(s);
      }
 
      sleep(10);
      ifstream fichier("/tmp/res_cmd.txt", ios::in);
      if(fichier){
	for(map<string,int>::iterator it (prog.begin());it!=prog.end();it++){
	  fichier>>prog[it->first];
	}
	fichier.close();
      }
      for(map<string,int>::iterator it (prog.begin());it!=prog.end();it++){
	if(it->second>1){
	  tmp=it->first+" est lancé";
	  write(sock,tmp.c_str(),tmp.size());
	  cout<<tmp<<endl;
	}else{
	  tmp=it->first+" n'est pas lancé";
	  write(sock,tmp.c_str(),tmp.size());
	  cout<<tmp<<endl;
	}
      }
  }
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

int lireProg(int client, char *buff, int max){
    char c='a'; //c initialisé a n'importe quoi sauf \n
    int i=0, n;
    //lire la quantité demandée
    while ((c!='.')&&(i<max)) {
      n=read(client, &c, 1);
      if (n==0) return 0;
      if (c!='.') buff[i++]=c;
    }
    buff[i]=0;
    return strlen(buff);
}


int getReponseMaster(int sock){
  char reponse[1000];
  int i;
  char port[4];
  char host[33];
  while(1){
    i=read(sock, reponse, 1);
    if(i>0){
      if(!strcmp(reponse,"C")){//Demande de connection numero salle PORT IP
	cout<<"demande de connection"<<endl;
	read(sock,reponse,1);
	read(sock,port,4);
	for(int cpt=0;cpt<4;cpt++){
	  cout<<port[cpt]<<endl;
	}
	read(sock,reponse,1);
	read(sock,host,33);
	for(int cpt=0;cpt<33;cpt++){
	  cout<<host[cpt]<<endl;
	}
	cout<<"connection"<<endl;
	return initSocketClient(host,atoi(port));
	cout<<"Fin"<<endl;
      }
      else{
	cout<<"Erreur: "<<reponse[0]<<endl;
      }
    }
  }

}
void getReponse(int sock){//fonction test pour savoir si la commande prec a fonctionné
  pthread_t T1;
  bool quit=false,continuer=true;
  char rep[1];
  char reponse[1000];
  int i;
  
  pthread_create(&T1, NULL, alerte, (void*)&sock);
  while(!quit){
    cout<<"Read "<<endl;
    i=read(sock, rep, 1);
    cout<<(string)reponse<<endl;
    if(i>0){
      if(!strcmp(rep,"S")){//STOP
	cout<<"Spy Stop"<<endl;
	quit=true;
	close(sock);
      }
      else if(!strcmp(rep,"A")){//Lis un nom de programme
	continuer=true;
	/* Protocol :
	 * A
	 * firefox
	 * ...
	 * quit
	 */

	while(continuer){
	  i=lireProg(sock,reponse,100);
	  if(i>0){
	    cout<<"Rep: "<<(string)reponse<<endl;
	    if(!strcmp(reponse, "quit")){
	      continuer=false;
	    }
	    else{
	      prog[(string)(reponse)]=0;
	    }
	  }
	}
      }
      else if(!strcmp(rep,"M")){
	i=lireProg(sock,reponse,100);
	if(i>0){
	  messageEtudiant(sock,reponse);
	}
      }
      else if(!strcmp(rep, "O")){
	i=lireProg(sock, reponse, 100);
	if(i>0){
	  controle(sock,(string)reponse);
	}
      }
      else if(!strcmp(rep,"P")){
	printEcran(sock);
      }
      else{
	cout<<"Erreur: "<<reponse[0]<<endl;
      }
    }

  }
  pthread_kill(T1,SIGUSR1);
}




int main(int args, char *arg[]){

  //initialisation du serveur
  int sock=initSocketClient(arg[1], atoi(arg[2]));
  int sockcontroleur;
  if (sock==-1) return -1; //en cas d'echec de l'initialisation
  connection(sock); 
  while(1){
    sockcontroleur=getReponseMaster(sock);
    getReponse(sockcontroleur);
  }
  close(sock);
  return 0;
}
  
