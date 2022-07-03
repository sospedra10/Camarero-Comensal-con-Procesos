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
./camarero clave periodo numeroComensales 
 
 *No hace falta poner numeroComensales, si pasa eso, su valor por defecto es 5
**/

int *comensales;
int numeroComensales=5;
int shmid;
int semid;

void fin();

int main(int argc, char** argv){
	
	if (argc<3 && argc>4){
		perror("Error en el paso de argumentos(gestor)\n");
		exit(-1);
	}
	
	int clave=atoi(argv[1]);
	int periodo=atoi(argv[2]);
	if (argc==4){
		numeroComensales=atoi(argv[3]);
	}
	
	
	//Creo MC
	//Necesito 4 enteros:
	//pan, jamon, cuchillo y contador (en este orden)
	shmid=shmget(clave,4*sizeof(int),IPC_CREAT | 0640);
    if(shmid ==-1){
        perror("Error al crear la memoria compartida(gestor)\n");
        exit(-2);
    }
    printf("Creada la MC\n");
    
    //Creo semaforo
    semid=semget(clave,1,IPC_CREAT | 0640);
    if(semid==-1){
        perror("Error al crear el semaforo(gestor)\n");
        exit(-3);
    }
	printf("Creado el semaforo\n");
	
	//Inicializado a 1 (Verde)
    if ( (semctl(semid,0,SETVAL,1))==-1 ){
		perror("Error al inicializar el semaforo(gestor)\n");
		exit(-5);
	}
	
	//Inicializo la MC
	int *entero;
    entero=(int*)shmat(shmid,NULL,0);
    if (entero==(int *)-1){
        perror("Error al escribir en la posicion indicada.\n");
        exit(-4);
    }
    for (int i=0;i<4;i++){
        entero[i]=0;
    }
    printf("Inicializado el semaforo y la MC\n");
    
    //Creo comensales
    comensales=(int*)malloc(numeroComensales*sizeof(int));
    
    int pid;
    char idComensal[4];
    
    for (int i=0;i<numeroComensales;i++){
		sleep(1);//para que los comensales no tengan los mismos objetos
		pid=fork();
		if (pid==0){// ./comensal clave periodo idComensal
			sprintf(idComensal,"%d",i);
			execlp("./comensal","./comensal",argv[1], argv[2], idComensal, NULL);
			perror("Error al ejecutar coordinador(gestor)\n");
			exit(-7);
		}
		else{
			comensales[i]=pid;
		}
	}
	
	srand (time(NULL));

	signal(SIGINT,&fin);
	
	int opcion=-1;
	int ciclo=0;
	struct sembuf accion;
	
	while(1){
		sleep(periodo);
		//numero = rand () % (N-M+1) + M;// Este está entre M y N(siendo M el pequeño)
		opcion=rand()%(3);//entre 0 y 2
		
		//WAIT sobre el semaforo    
		accion.sem_num=0; 
		accion.sem_op=-1;   
		accion.sem_flg=0;
		semop(semid,&accion,1);
		
		entero[opcion]++;
		entero[3]++;//ciclo
		ciclo=entero[3];
		
		//SIGNAL sobre el semaforo
		accion.sem_num = 0;   
		accion.sem_op = 1;     
		accion.sem_flg = 0; 
		semop(semid,&accion,1);
		
		if(opcion==0)//pan
			printf("CAMARERO, ciclo:%d, pan, %d %d %d \n",ciclo, entero[0], entero[1], entero[2]);
		else if (opcion==1)//cuchillo
			printf("CAMARERO, ciclo:%d, cuchillo, %d %d %d \n",ciclo, entero[0], entero[1], entero[2]);
		else//jamon
			printf("CAMARERO, ciclo:%d, jamon, %d %d %d \n",ciclo, entero[0], entero[1], entero[2]);
		
	}
	
    
	
	
	return 0;
}

void fin(){
	for (int i=0;i<numeroComensales;i++){
		kill(comensales[i],SIGUSR1);
	}
	
	//Elimino semaforo
	if ( (semctl(semid,0,IPC_RMID))==-1 ){
		perror("Error al eliminar el semaforo.\n");
		exit(-6);
	}
	
	//Elimino MC
	if( (shmctl(shmid,IPC_RMID,NULL))==-1){
		perror("Error al eliminar la memoria compartida.\n");
		exit(-7);
	}
	
	printf("\nElimino el semaforo y la MC\n");
	printf("Finalizado el programa CAMARERO\n");
	exit(0);
}










