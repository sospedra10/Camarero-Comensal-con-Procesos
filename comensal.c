#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <string.h> 
#include <sys/msg.h>
#include <sys/shm.h> 
#include <sys/wait.h>

#include <time.h>

/**
./comensal clave periodo idComensal
**/

int contador=0;
int idComensal;
int numeroBocadillos=0;

void fin(int x);

int main(int argc, char** argv){
	printf("comensal\n");
	if (argc!=4){
		perror("Error en los argumentos(comensal)\n");
		exit(-1);
	}

	int clave=atoi(argv[1]);
	int periodo=atoi(argv[2]);
	idComensal=atoi(argv[3]);
	
	//MC
	int shmid=shmget(clave,sizeof(int), 0640);
    if(shmid ==-1){
        perror("Error al crear la memoria compartida(comensal).\n");
        exit(-2);
    }
     
    //Semaforo
    int semid=semget(clave,1, 0640);
    if(semid==-1){
        perror("Error al crear el semaforo(comensal).\n");
        exit(-3);
    }
    
	signal(SIGUSR1,&fin);
	
	int pan=0, cuchillo=1, jamon=2;
	int opcion1;
	int opcion2;
	int opcion3=-1;
	int bocadilloPreparado=-1;//-1 sin preparar, 1 preparado
	struct sembuf accion;
	
	int *valorMC=(int*)shmat(shmid,NULL,0);
	
	srand (time(NULL));
	
	opcion1=rand()%(3);
	opcion2=rand()%(3);
	while(opcion1==opcion2){
		opcion2=rand()%(3);
	}
	sleep(periodo+1);
	printf("Proceso %d tiene %d y %d\n",idComensal,opcion1, opcion2);
	
	while(1){
		if (bocadilloPreparado==1){//bocadillo peparado y necesito nuevos objetos
			opcion1=rand()%(3);
			opcion2=rand()%(3);
			while(opcion1==opcion2){
				opcion2=rand()%(3);
			}
			bocadilloPreparado=-1;
			printf("Proceso %d tiene %d y %d\n",idComensal,opcion1, opcion2);
			sleep(periodo+1);
		}
		
		
		//WAIT sobre el semaforo    
		accion.sem_num=0; 
		accion.sem_op=-1;   
		accion.sem_flg=0;
		semop(semid,&accion,1);
		
		if ( (opcion1==pan && opcion2==cuchillo) || (opcion1==cuchillo && opcion2==pan) ){//tengo pan cuchillo, necesito JAMON->2
			if (valorMC[2]!=0){
				opcion3=jamon;
				//printf("Jamones\n");
				valorMC[2]--;
				valorMC[3]++;
			}
		}
		else if ( (opcion1==cuchillo && opcion2==jamon) || (opcion1==jamon && opcion2==cuchillo) ){//tengo cuchillo jamon, necesito PAN->0
			if (valorMC[0]!=0){
				//printf("Panes\n");
				opcion3=pan;
				valorMC[0]--;
				valorMC[3]++;
			}
		}
		else{//tengo pan jamon, necesito CUCHILLO->1
			if (valorMC[1]!=0){
				//printf("Cuchillos\n");
				opcion3=cuchillo;
				valorMC[1]--;
				valorMC[3]++;
			}
		}
		
		//SIGNAL sobre el semaforo
		accion.sem_num = 0;   
		accion.sem_op = 1;     
		accion.sem_flg = 0; 
		semop(semid,&accion,1);
		
		if (opcion3!=-1){
			sleep(2);
			numeroBocadillos++;
			if (opcion3==jamon){//Obtenido JAMON
				printf("Proceso %d: %d bocadillos preparados-->pan cuchillo JAMON\n",idComensal,numeroBocadillos);
			}
			else if (opcion3==pan){//Obtenido PAN
				printf("Proceso %d: %d bocadillos preparados-->PAN cuchillo jamon\n",idComensal,numeroBocadillos);
			}
			else{//Obtenido CUCHILLO
				printf("Proceso %d: %d bocadillos preparados-->pan CUCHILLO jamon\n",idComensal,numeroBocadillos);
			}
			bocadilloPreparado=1;
			opcion3=-1;
		}
	
	}
	return 0;
}

void fin(int x){
	printf("Proceso:%d:%d bocadillos preparados-->FINALIZADO\n",idComensal,numeroBocadillos);
	exit(0);
}








