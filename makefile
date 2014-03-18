CC=g++ -pthread -std=c++11 

master: Master.cpp
	$(CC) -o Master Master.cpp

spy: spy.cpp
	$(CC) -o spy spy.cpp

controleur: Controleur.cpp
	$(CC) -o Controleur Controleur.cpp


clean :
	rm -rf *.o main



