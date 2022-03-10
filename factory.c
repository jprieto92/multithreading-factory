#include <stdio.h>
#include <stdlib.h>
#include "db_factory.h"
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "factory.h"
#include <time.h>

// You can add here more global variables


//Defined variables
//Object that will be inserted into the belt (belt)

struct object
{
  	int id;
  	char name [255];
	//int disponible;
};


struct object belt[MAX_BELT]; /* common belt for transporters and receivers */

/*STRUCT THAT DEFINE ATRIBUTES OF ANY INSERTED THREAD*/
struct insertObject
{
	int idThread;			/*NUMBER OF THREAD --0 TO N-1 INSERTERS*/
	int numberElements;		/*NUMBER OF CREATES ELEMENTS BY THREAD*/
	int numberModifiedElements;	/*NUMBER OF ELEMENTS TO MODIFIED BY THREAD*/
	int incrementStock;		/*NUMBER OF UNITS TO INCREMENT THE STOCK OF ANY ELEMENT TO MODIFIED*/
};
typedef struct insertObject INSERTERS_OBJECT;

/*STRUCT THAT DEFINE INTERNAL DATA OF ANY ELEMENT IN DATA BASE -- ELEMENTS OF CONCURRENCY*/
struct dataBaseElementConcurrency
{
	pthread_mutex_t mutexElement;	/*MUTEX OF ELEMENT IN LOCAL ACCESS*/
};
typedef struct dataBaseElementConcurrency ELEMENT_CONCURRENCY;


/*DATA BASE CONCURRENCY*/
pthread_mutex_t mutexdb; 	/*MUTEX TO ACCESS DATABASE*/
pthread_cond_t dbDisponible; 	/*CONDITION VARIABLE OF DATA BASE DISPONIBILITY*/

int dataBaseCreate;		/*0-DATA BASE DISPONIBLE --- 1-DATA BASE WITH LOCAL WRITTINGS --- 2-DATA BASE WITH GLOBAL WRITTINGS*/
int dataBaseReady;		/*0-EMPTY DATABASE --- 1-DATABASE WITH 1 OR MORE CREATES*/
/*-----------------------*/

/*BELT CONCURRENCY*/
pthread_mutex_t mutexBelt;	/*MUTEX TO ACCESS DATABASE*/
pthread_cond_t beltEmpty;	/*CONDITION VARIABLE OF EMPTY BELT*/
pthread_cond_t beltFull;	/*CONDITION VARIABLE OF FULL BELT*/

int beltPosIn;			/*INDEX OF BELT POSITION IN*/
int beltPosOut;			/*INDEX OF BELT POSITION AUX*/
int elementsInBelt;		/*NUMBER OF ELEMENTS IN BELT AT THE SAME TIME*/
int elementsReceived;		/*NUMBER OF ELEMENTS RECEIVED BY RECEIVERS*/
/*-----------------------*/


//Number of threads for transport
int number_transporters=0;
//Number of threads for insert
int number_inserters=0;
//Number of threads for receive
int number_receivers=0;

/*THREADS POINTERS*/
pthread_t * transporters;
pthread_t * inserters;
pthread_t * receivers;

 

//Total number of elements to be transported
int total_number=0;





int main(int argc, char ** argv)
{

  	if(argc == 2)
	{
			init_factory(argv[1]);
			close_factory();
	}else
	{
			perror("Invalid syntax: ./factory input_file ");
			return 0;
	}
	exit(0);
}

/*Function that parses the input file and initializes all structures and resources*/
int init_factory(char *file)
{
  	int i=0;

  	int error=0;
	int correctCreate=0;
  	int * number_elements;
  	int * number_modified_elements;
  	int * number_modified_stock;
  	if(file != NULL)
	{
    		FILE * filefd = fopen(file, "r");
    		error=fscanf(filefd, "%d", &number_inserters);
    		printf("Number inserters %d\n", number_inserters);
    		if(error != 1)
		{
      			perror("Error reading from file\n");
			error = fclose(filefd);
 	                if(error != 0)
        	        {
                        	perror("Error closing file\n");
                	}
			exit(-1);
    		}
    		number_transporters=1;
    		error=fscanf(filefd, "%d", &number_receivers);
    		printf("Number receivers %d\n", number_receivers);
   	 	if(error != 1)
		{
      			perror("Error reading from file\n");
			error = fclose(filefd);
                	if(error != 0)
                	{
                        	perror("Error closing file\n");
                	}
			exit(-1);
    		}

    		//Allocation of memory
    		//Number of elements that are going to be inserted by each thread
    		number_elements= (int *) malloc(sizeof(int)*number_inserters);
    		//Number of elements which stock is going to be updated
    		number_modified_elements= (int *) malloc(sizeof(int)*number_inserters);
    		//Number of eements that are going to be added to the stock
    		number_modified_stock= (int *) malloc(sizeof(int)*number_inserters);
    		//Pthread_t of each inserter
    		inserters =(pthread_t *) malloc(sizeof(pthread_t)*number_inserters);
    		//Pthread_t of each transporter
    		transporters =(pthread_t *) malloc(sizeof(pthread_t)*number_transporters);
    		//Pthread_t of each receiver
    		receivers =(pthread_t *) malloc(sizeof(pthread_t)*number_receivers);

    		//Reading elements depending on number of inserters
    		for(i=0; i< number_inserters; i++)
		{
      			error= fscanf(filefd, "%d %d %d", &number_elements[i], &number_modified_elements[i], &number_modified_stock[i]);

      			if(error != 3 || number_modified_elements[i] > number_elements[i] || number_modified_elements[i]<0 || number_modified_stock[i]<0)
			{
        			perror("Error reading from file\n");
        			free(number_elements);
				free(number_modified_elements);
				free(number_modified_stock);
				free(transporters);
        			free(inserters);
        			free(receivers);
				error = fclose(filefd);
		                if(error != 0)
                		{
                        		perror("Error closing file\n");
                		}
				exit(-1);
      			}
    		}
    		//Total number of elements that will be in the database
    		for(i=0; i< number_inserters; i++ )
		{
      			total_number+=number_elements[i]+(number_modified_elements[i]*number_modified_stock[i]);
    		}
    		//Init the database
    		error=db_factory_init();	
    		if(error!=0)
		{
			perror("Error when initializing the database\n");
			free(number_elements);
                        free(number_modified_elements);
                        free(number_modified_stock);
			error = fclose(filefd);
                	if(error != 0)
                	{
                        	perror("Error closing file\n");
                	}
			exit(-1);
    		}
    		printf("Total number of elements %d\n", total_number);

    		//You can add more code here
		if(number_receivers<=0)
		{
			perror("Error, receivers menor o igual a 0.\n");
			free(number_elements);
                        free(number_modified_elements);
                        free(number_modified_stock);
			close_factory();
			error = fclose(filefd);
	                if(error != 0)
                	{
                        	perror("Error closing file\n");
                	}
			exit(1);
		}
		if(number_inserters<0)
		{
			perror("Error, inserters menor que 0.\n");
			free(number_elements);
                        free(number_modified_elements);
                        free(number_modified_stock);
			close_factory();
			error = fclose(filefd);
	                if(error != 0)
        	        {
                        	perror("Error closing file\n");
                	}
			exit(1);
		}
		double diff;
		time_t start,finish;
		start=time(NULL);
		/*BLOQUE DE CODIGO INSERTERS*/
		/*CREATE DE ARRAY STRUCT OF ATRIBUTES OF INSERTERS THREADS*/
		INSERTERS_OBJECT insObject [number_inserters];
		for(i=0;i<number_inserters;i++)
		{
			insObject[i].idThread=i;
			insObject[i].numberElements=number_elements[i];
			insObject[i].numberModifiedElements=number_modified_elements[i];
			insObject[i].incrementStock=number_modified_stock[i];
		}
		/*INIT MUTEX, CONDITION VARIABLES AND VARIABLES OF CONTROL FOR CONCURRENCY*/
		pthread_mutex_init(&mutexdb,NULL);
		pthread_cond_init(&dbDisponible,NULL);
		pthread_mutex_init(&mutexBelt,NULL);
		pthread_cond_init(&beltEmpty,NULL);
		pthread_cond_init(&beltFull,NULL);
		elementsInBelt=0;
		dataBaseCreate=0;
		dataBaseReady=0;
		elementsReceived=0;
		beltPosIn=0;
		beltPosOut=0;
		/*CREATE AND INIT INSERTERS THREADS*/
		for(i=0;i<number_inserters;i++)
		{
			correctCreate=pthread_create(&inserters[i],NULL,inserter,&insObject[i]);
			if(correctCreate!=0)
			{
				perror("Error at create inserter.\n");
				free(number_elements);
                        	free(number_modified_elements);
                        	free(number_modified_stock);
                                close_factory();
                                error = fclose(filefd);
		                if(error != 0)
               	 		{
                        		perror("Error closing file\n");
                		}
				exit(1);	
			}
		}
		/*CREATE AND INIT TRANSPORTER THREAD*/
		pthread_create(transporters,NULL,(void * (*)(void *))transporter,NULL);
		if(correctCreate!=0)
                {
                	perror("Error at create transporter.\n");
                        free(number_elements);
                        free(number_modified_elements);
                        free(number_modified_stock);
			close_factory();
                        error = fclose(filefd);
	                if(error != 0)
        	        {
               		         perror("Error closing file\n");
               		}
			exit(1);
                }
		/*CREATE AND INIT RECEIVER THREAD*/
		for(i=0;i<number_receivers;i++)
		{
			pthread_create(&receivers[i],NULL,(void * (*)(void *))receiver,NULL);
			if(correctCreate!=0)
                        {
                                perror("Error at create receiver.\n");
                                free(number_elements);
                        	free(number_modified_elements);
                        	free(number_modified_stock);
				close_factory();
				error = fclose(filefd);
		                if(error != 0)
               			{
                        		perror("Error closing file\n");
                		}
                                exit(1);
                        }
		}
		/*JOIN INSERTERS*/
		for(i=0;i<number_inserters;i++)
                {
                        pthread_join(inserters[i],NULL);
                }
		/*JOIN TRANSPORTER*/
		pthread_join(*transporters,NULL);
		/*JOIN RECEIVER*/
		for(i=0;i<number_receivers;i++)
		{
			pthread_join(receivers[i],NULL);
		}
		printf("EXPORT FINISHED\n");
		finish=time(NULL);
		diff = difftime(finish,start);
		printf("Tiempo: %lf\n",diff);
		free(number_elements);
                free(number_modified_elements);
                free(number_modified_stock);
		error = fclose(filefd);	
		if(error != 0)
		{
      			perror("Error closing file\n");
    		}
    		return 0;
  	}else
	{
    		return -1;
  	}
}
/*Function that closes and free the resources used by the factory*/

int close_factory()
{
  	int correctLiberation=0;
	//You can add your code here
	printf("FREE THE RESOURCES USED BY THE FACTORY...\n");
	/*DESTROY DATABASE*/
	correctLiberation=db_factory_destroy();
	if(correctLiberation!=0)
	{
		perror("ERROR AL DESTRUIR LA BASE DE DATOS.\n");
	}		
	/*FREE MALLOC OF THREADS*/
	free(transporters);
	free(inserters);
	free(receivers);
	pthread_cond_destroy(&dbDisponible);
	pthread_cond_destroy(&beltEmpty);
	pthread_cond_destroy(&beltFull);
	printf("OK. ALL RESOURCES LIBERATED...\n");
  	return 0;
}



/*Function that inserts a new element to the database*/


/*Function that updates the stock of the element of the database*/


/*Function executed by the inserter thread.
 * It is in charge of inserting and updating elements
 */
void * inserter(void * data)
{
  	int correctInsertion=0; 		/*RESULT OF CORRECT OR NOT ACTIONS WITH DATABASE*/
	int id;					/*ID OF ELEMENT*/
	int stock;				/*STOCK OF ELEMENT*/
	INSERTERS_OBJECT insObject;		/*ELEMENT OF TYPE STRUCT INSERTER OBJECTS*/
	insObject=*((INSERTERS_OBJECT *) data);	/*CASTING AND ASIGNATION*/
	char name[255];				/*ARRAY OF CHAR FOR NAME OF ELEMENT*/
	char aux[16];				/*AUXILIAR ARRAY OF CHAR FOR CREATE NAME OF ELEMENT*/
	int i=0;
	for(i=0;i<insObject.numberElements;i++)
	{
		sprintf(name,"%d",insObject.idThread);
		sprintf(aux,"%d",i);
		strcat(name,aux);
		ELEMENT_CONCURRENCY *concurrencyController=(ELEMENT_CONCURRENCY *)malloc(sizeof(ELEMENT_CONCURRENCY));	/*CREATE STRUCT OF INTERNAL DATA IN THE HEAP WITH MALLOC*/
		/*INIT CONCURRENCY ATRIBUTES OF ELEMENT*/
		pthread_mutex_init(&(concurrencyController->mutexElement),NULL);
		/*
		*SECCION CRITICA PARA CREAR EN BASE DE DATOS
		*/
		pthread_mutex_lock(&mutexdb);
		while(dataBaseCreate>0)	/*DATA BASE NOT DISPONIBLE*/
		{
			pthread_cond_wait(&dbDisponible,&mutexdb);
		}
		dataBaseCreate=2;	/*SET STATE OF DATABASE TO GLOBAL WRITTING*/
		correctInsertion=db_factory_create_element(name,1,&id);
		if(correctInsertion!=0)
		{
			perror("Error in create element.\n");
			free(concurrencyController);
			pthread_cond_broadcast(&dbDisponible);
			pthread_mutex_unlock(&mutexdb);
			close_factory();
			exit(1);
		}else
		{	
			correctInsertion=db_factory_set_internal_data(id,concurrencyController,sizeof(concurrencyController));
			if(correctInsertion!=0)
			{
				perror("Error in set internal data.\n");
				close_factory();
				exit(1);
			}
			dataBaseCreate=0;			/*SET STATE OF DATABASE TO DISPONIBLE*/
			if(dataBaseReady==0)
			{
				dataBaseReady=1;		/*SET READY TO 1 -- TRANSPORTER THREAD CAN INIT*/
			}	
			pthread_cond_broadcast(&dbDisponible);
			pthread_mutex_unlock(&mutexdb);
		}
		/*
		*FIN DE SECCION CRITICA CREACION BASE DE DATOS
		*/
	}
	ELEMENT_CONCURRENCY *concurrencyElementAux;
	int z=0;	/*AUXILIAR VARIABLE TO GET SIZE AT GET INTERNAL DATA*/
	for(i=0;i<insObject.numberModifiedElements;i++)
	{
		/*
		*SECCION CRITICA DE AUMENTO DE STOCK--PROTEGEMOS DATA BASE
		*/
		pthread_mutex_lock(&mutexdb);
		while(dataBaseCreate>1)
		{
			pthread_cond_wait(&dbDisponible,&mutexdb);
		}
		dataBaseCreate=1;	/*CAMBIAMOS A ESCRITURA LOCAL*/
		pthread_cond_broadcast(&dbDisponible);
		pthread_mutex_unlock(&mutexdb);
		/*LIBERAMOS BASE DE DATOS*/
		correctInsertion=db_factory_get_internal_data(i,(void **)&concurrencyElementAux,&z);
		if(correctInsertion!=0)
		{
			perror("Error in get element.\n");
			close_factory();
			exit(1);
		}
		pthread_mutex_lock(&(concurrencyElementAux->mutexElement));
		correctInsertion=db_factory_get_stock(i,&stock);
		if(correctInsertion!=0)
                {
                        perror("Error in get stock.\n");
                        close_factory();
                        exit(1);
                }
		stock=stock+insObject.incrementStock;
		correctInsertion=db_factory_update_stock(i,stock);
		if(correctInsertion!=0)
                {
                        perror("Error in update stock.\n");
                        close_factory();
                        exit(1);
                }
		pthread_mutex_lock(&mutexdb);
		/*BASE DE DATOS EN ESTADO DISPONIBLE*/
		dataBaseCreate=0;
		pthread_cond_broadcast(&dbDisponible);
		pthread_mutex_unlock(&mutexdb);
		pthread_mutex_unlock(&(concurrencyElementAux->mutexElement));
		/*
		*FIN DE SECCION CRITICA AUMENTO DE STOCK		
		*/
	}
  	printf("Exitting inserter thread\n");
  	return 0;
}



/*Function executed by the thread transporter*/
void * transporter(void)
{
  	//To be changed for concurrency
  	char name[255];
  	int id=0;
  	int number_elements=0;		/*NUMBER OF ELEMENTS TRANSPORTED TO BELT*/
	int state=0;			/*STATE OF ELEMENT CONSULT*/
	int stock=-1;			/*STOCK OF ELEMENT CONSULT*/
	int correctConsult=0;		/*CORRECT OR NOT CORRECT ACTION IN ELEMENT*/
	int elementsInBeltAux=0;	/*AUXILIAR VARIABLE TO PRINT ELEMENTS IN BELT*/
	int position=0;			/*AUXILIAR VARIABLE THAT PRINT POSITION THE ELEMENT IN BELT*/
	int z=0;
	ELEMENT_CONCURRENCY *concurrencyElementAux;	/*POINTER TO INTERNAL DATA OF ELEMENT CONSULT*/
	while(number_elements<total_number)
	{
		id=0;					
		pthread_mutex_lock(&mutexdb);
		while(dataBaseReady==0)				/*IF DATABASE WITHOUT ANY CREATE...WAIT*/
		{
			pthread_cond_wait(&dbDisponible,&mutexdb);
		}
		while(dataBaseCreate>1)				/*IF DATA BASE IN STATE GLOBAL WRITTING...WAIT*/
                {
                        pthread_cond_wait(&dbDisponible,&mutexdb);
                }
                dataBaseCreate=1;				/*STATE OF DATA BASE IN LOCAL READ-WRITE*/
                pthread_cond_broadcast(&dbDisponible);
	        pthread_mutex_unlock(&mutexdb);
		correctConsult=db_factory_get_ready_state(id,&state);
		if(correctConsult!=0)
                {
                        perror("Error in get ready state.\n");
                        close_factory();
                        exit(1);
                }
		while(state!=0 && (number_elements<total_number))
		{
			/*
			*SECCION CRITICA POR CADA ELEMENTO DE LA BASE DE DATOS
			*/
			correctConsult=db_factory_get_internal_data(id,(void **)&concurrencyElementAux,&z);
			if(correctConsult!=0)
	                {
        	                perror("Error in get internal data.\n");
                	        close_factory();
                       		exit(1);
                	}
			pthread_mutex_lock(&(concurrencyElementAux->mutexElement));
			correctConsult=db_factory_get_stock(id,&stock);
			if(correctConsult!=0)
	                {
        	                perror("Error in get stock.\n");
                	        close_factory();
                        	exit(1);
                	}
			while(stock>0)
			{
                		correctConsult=db_factory_get_element_name(id,name);
				if(correctConsult!=0)
		                {
                		        perror("Error in get element name.\n");
                        		close_factory();
                        		exit(1);
                		}
				stock=stock-1;
				correctConsult=db_factory_update_stock(id,stock);
				if(correctConsult!=0)
		                {
                	        	perror("Error in update stock.\n");
                       	 		close_factory();
                        		exit(1);
                		}
				pthread_mutex_unlock(&(concurrencyElementAux->mutexElement));
				pthread_mutex_lock(&mutexdb);
                		dataBaseCreate=0;	/*LIBERAMOS LA BASE DE DATOS*/
               			pthread_cond_broadcast(&dbDisponible);
                		pthread_mutex_unlock(&mutexdb);
				/*
				*FIN SECCION CRITICA POR CADA ELEMENTO DE LA BASE DE DATOS
				*/
				/*
				*SECCION CRITICA DE LA CINTA
				*/
				pthread_mutex_lock(&mutexBelt);	
				while(elementsInBelt==MAX_BELT)
				{
					pthread_cond_wait(&beltFull,&mutexBelt);
				}
				belt[beltPosIn].id=id;
				strcpy(belt[beltPosIn].name,name);
				position=beltPosIn;
				elementsInBelt++;
				elementsInBeltAux=elementsInBelt;
				beltPosIn=(beltPosIn+1)%MAX_BELT;		
				if(elementsInBelt==1)
				{
					pthread_cond_broadcast(&beltEmpty);
				}
				pthread_mutex_unlock(&mutexBelt);
				/*
				*FIN SECCION CRITICA DE LA CINTA
				*/
				number_elements++;
				printf("Introducing element %d, %s in position [%d] with %d number of elements\n",id, name, position, elementsInBeltAux);
				pthread_mutex_lock(&mutexdb);
               			while(dataBaseCreate>1)
                		{
                        		pthread_cond_wait(&dbDisponible,&mutexdb);
               	 		}
                		dataBaseCreate=1;
                		pthread_cond_broadcast(&dbDisponible);
                		pthread_mutex_unlock(&mutexdb);
				correctConsult=db_factory_get_internal_data(id,(void **)&concurrencyElementAux,&z);
                        	pthread_mutex_lock(&(concurrencyElementAux->mutexElement));
				correctConsult=db_factory_get_stock(id,&stock);
			}
			pthread_mutex_unlock(&(concurrencyElementAux->mutexElement));	/*LIBERAMOS ELEMENTO*/
			pthread_mutex_lock(&mutexdb);
                        dataBaseCreate=0;						/*LIBERAMOS BASE DE DATOS*/
                        pthread_cond_broadcast(&dbDisponible);
                        pthread_mutex_unlock(&mutexdb);
			id++;								/*NEXT ELEMENT IN DATA BASE*/
			pthread_mutex_lock(&mutexdb);
	                while(dataBaseCreate>1)
        	        {
                	        pthread_cond_wait(&dbDisponible,&mutexdb);
            	    	}
               		dataBaseCreate=1;
                	pthread_cond_broadcast(&dbDisponible);
                	pthread_mutex_unlock(&mutexdb);
			correctConsult=db_factory_get_ready_state(id,&state);
			if(correctConsult!=0)
	                {
        	                perror("Error in get ready state.\n");
                	        close_factory();
                        	exit(1);
                	}
			/*
			*FIN DE SECCION CRITICA POR CADA ELEMENTO DE LA BASE DE DATOS
			*/		
		}
		pthread_mutex_lock(&mutexdb);
                dataBaseCreate=0;	/*LIBERAMOS BASE DE DATOS*/
                pthread_cond_broadcast(&dbDisponible);
                pthread_mutex_unlock(&mutexdb);
	}
  	//To be printed when inserted in the belt (belt)
  	printf("Exitting thread transporter\n");
  	return 0;
}


/*Function execute by the receiver thread*/
void * receiver()
{
  	//To be changed for concurrency
	char name[255];
  	int id=0;
  	int number_elements=0;
  	int position=0;
  	//To be printed when an element is received
	/*
	*SECCION CRITICA DE LA CINTA
	*/
	pthread_mutex_lock(&mutexBelt);
	while(elementsInBelt==0 && (elementsReceived<total_number))
	{
		pthread_cond_wait(&beltEmpty,&mutexBelt);
	}
	while(elementsReceived<total_number)				/*ONLY ENTRY IF NOT FINISH RECEIVER ROLE*/
	{
		strcpy(name,belt[beltPosOut].name);
		id=belt[beltPosOut].id;
		position=beltPosOut;
		beltPosOut=(beltPosOut+1)%MAX_BELT;
		elementsInBelt--;
		number_elements=elementsInBelt;
		elementsReceived++;
		if(elementsInBelt==(MAX_BELT-1))
		{
			pthread_cond_broadcast(&beltFull);
		}
		pthread_mutex_unlock(&mutexBelt);
		/*
		*FIN DE SECCION CRITICA DE CINTA
		*/
		printf("Element %d, %s has been received from position [%d] with %d number of elements\n",id, name, position, number_elements );
		if(elementsReceived==total_number)			/*IF ALL ELEMENTS RECEIVED, SIGNAL FOR OUT ALL THREADS*/
		{
			pthread_cond_broadcast(&beltEmpty);		
		}else
		{
			pthread_mutex_lock(&mutexBelt);
        		while(elementsInBelt==0 && (elementsReceived<total_number))
        		{
                		pthread_cond_wait(&beltEmpty,&mutexBelt);
       	 		}
		}
	}
	pthread_cond_broadcast(&beltEmpty);
	pthread_mutex_unlock(&mutexBelt);
  	printf("Exitting thread receiver\n");
  	return 0;
}



