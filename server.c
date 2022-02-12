//server


//Mareci Ioana Amalia 2A2


#include <sys/types.h>

#include <sys/time.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <errno.h>

#include <unistd.h>

#include <stdio.h>

#include <string.h>

#include <stdlib.h>

#include <signal.h>

#include <pthread.h>

#include <string.h>

#include <time.h>

/*portul folosit */


#define PORT 2908


/*codul de eroare returnat de anumite apeluri */

extern int errno;

typedef struct thData


{
	int idThread;	//id-ul thread-ului tinut in evidenta de acest program

	int cl;	//descriptorul intors de accept

}thData;

struct Player

{

	char nume[1000];

	int punctaj;

	int intrebarifolosite[100];

}jucatori[1000];

int nr_jucatori=0;

int nr_terminati=0;

int terminat[1000];

int varianta_corecta[100];

void XMLParser(FILE *XML_questions);

int GetXmlTagValue(char *RespBuffer, char Tag[100], char *TagValue);

static void *treat(void*); /*functia executata de fiecare thread ce realizeaza comunicarea cu clientii */

void raspunde(void*);

void intrebare(int j, char s[1000]);

int punctaj_maxim(char castigator[100]);

int jucatorii_au_terminat();

char intrebari_variante[100][100] = { 0 };

int nr_castigatori;

int main()

{

	FILE * XML_questions;

	if (!(XML_questions = fopen("/home/amalia/proiect/q&a.xml", "r")))

	{

		perror("[server]Error opening file with questions\n");

		return errno;


	}

	XMLParser(XML_questions);	// We parse the file with the questions

	fclose(XML_questions);	// We don't need this file anymore, so we can close it

	struct sockaddr_in server;	// structura folosita de server

	struct sockaddr_in from;

	int nr;	//mesajul primit de trimis la client 

	int sd;	//descriptorul de socket 

	int pid;

	pthread_t th[100];	//Identificatorii thread-urilor care se vor crea

	/*crearea unui socket */

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)

	{
		perror("[server]Eroare la socket().\n");

		return errno;

	}
    int on=1;
	/*utilizarea optiunii SO_REUSEADDR */

	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	/*pregatirea structurilor de date */

	bzero(&server, sizeof(server));

	bzero(&from, sizeof(from));

	/*umplem structura folosita de server */

	/*stabilirea familiei de socket-uri */

	server.sin_family = AF_INET;

	/*acceptam orice adresa */

	server.sin_addr.s_addr = htonl(INADDR_ANY);

	/*utilizam un port utilizator */

	server.sin_port = htons(PORT);

	/*atasam socketul */

	if (bind(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)


	{

		perror("[server]Eroare la bind().\n");

		return errno;

	}

	/*punem serverul sa asculte daca vin clienti sa se conecteze */

	if (listen(sd, 2) == -1)

	{

		perror("[server]Eroare la listen().\n");

		return errno;

	}

	/*servim in mod concurent clientii...folosind thread-uri */

	while (1)

	{

		int client;


		thData * td;	//parametru functia executata de thread     


		int length = sizeof(from);


		printf("[server]Asteptam la portul %d...\n", PORT);


		fflush(stdout);

		/*acceptam un client (stare blocanta pina la realizarea conexiunii) */

		if ((client = accept(sd, (struct sockaddr *) &from, &length)) < 0)

		{

			perror("[server]Eroare la accept().\n");

			continue;

		}


		/*s-a realizat conexiunea, se astepta mesajul */


		// int idThread;	//id-ul threadului

		// int cl;	//descriptorul intors de accept


		td = (struct thData *) malloc(sizeof(struct thData));

		td->idThread = nr_jucatori++;

		td->cl = client;


		// struct id thread


		pthread_create(&th[nr_jucatori], NULL, &treat, td);


	}	//while    

};

static void *treat(void *arg)

{

	struct thData tdL;

	tdL = *((struct thData *) arg);


	pthread_detach(pthread_self());

	// se primeste numele clientului cu care se identifica in joc

	if (read(tdL.cl, jucatori[tdL.idThread].nume, sizeof(jucatori[tdL.idThread].nume)) <= 0)

	{


		terminat[tdL.idThread] = 1;

		printf("[Thread %d]\n", tdL.idThread);

		perror("Eroare la citirea numelui de la client.\n");	

		nr_terminati++;

	}


	else


	{

		char buf[100];

		memset(buf, 0, sizeof(buf));


		if (read(tdL.cl, buf, sizeof(buf)) <= 0)//daca jucatorul alege 1.joaca 2.iesire



		{

			terminat[tdL.idThread] = 1;

			printf("[Thread %d]\n", tdL.idThread);

			perror("Eroare la comenzii numelui de la client.\n");
			
			nr_terminati++;

		}

		else

		{

			int alege = atoi(buf);

			//jucatorul alege sa joace


			if (alege == 1)

			{

				jucatori[tdL.idThread].punctaj = 0;

				printf("[thread]- %d - Incepem jocul pentru %s.\n", tdL.idThread, jucatori[tdL.idThread].nume);

				fflush(stdout);

				raspunde((struct thData *) arg);

			}


			else	//jucatorul alege exit


			{

				terminat[tdL.idThread] = 1;

				printf("Jucatorul %s a parasit quizzul \n", jucatori[tdL.idThread].nume);
				
				nr_terminati++;
			}


		}	//else comanda


	}	//else nume


	int OK = 1;

	printf("numar jucatori%d numar terminati %d\n",nr_jucatori,nr_terminati);

		if (nr_terminati != nr_jucatori)

			OK = 0;
	
	//printf("id %d si punctaj %d",tdL.idThread,jucatori[tdL.idThread].punctaj)	;
	if(OK == 1)
	{

		for (int i = 0; i < nr_jucatori; i++)

		jucatori[i].punctaj = 0;

	}	
	/*am terminat cu acest client, inchidem conexiunea */


	close((intptr_t) arg);

	return (NULL);

}


void raspunde(void *arg)

{


	fd_set actfds; /*multimea descriptorilor activi */


	char rasp_juc[1000] = { 0 };

	char clasament_final[1000] = { 0 };


	int  numar_rasp = 0;
	
	int iesit=0;


	struct thData tdL;

	tdL = *((struct thData *) arg);



	//


	int x = 0;


	srand(time(0));



	for (int i = 1; i <= 7; i++)

	{
		FD_ZERO(&actfds); /*initial, multimea este vida */


		FD_SET(tdL.cl, &actfds);


		x = rand() % 8;

		while (jucatori[tdL.idThread].intrebarifolosite[x] == 1 || x == 0)

			x = rand() % 8;


		jucatori[tdL.idThread].intrebarifolosite[x] = 1;

		//printf("%d\n",x);		

		char sir[1000];


		memset(sir, 0, sizeof(sir));


		intrebare(x, sir);	// pun in s cate o intrebare


		if (write(tdL.cl, sir, sizeof(sir)) <= 0)


		{
			printf("[Thread %d]\n", tdL.idThread);


			perror("Eroare la scrierea intrebarii.\n");


			break;


		}


		memset(rasp_juc, 0, sizeof(rasp_juc));


		int rezultat = read(tdL.cl, rasp_juc, sizeof(rasp_juc));


		numar_rasp = atoi(rasp_juc);

		if (rezultat < 0)

		{

			terminat[tdL.idThread] = 1;

			jucatori[tdL.idThread].punctaj = -1;

			printf("[Thread %d]\n", tdL.idThread);

			perror("Eroare la read() de la client.\n");

			nr_terminati++;

			break;

		}


		if (rezultat == 0) //CTRL+C


		{

			iesit=1;			
			
			terminat[tdL.idThread] = 1;

			jucatori[tdL.idThread].punctaj = -1;


			printf("Clientul %s a parasit jocul \n", jucatori[tdL.idThread].nume);


			fflush(stdout);

			nr_terminati++;

			break;


		}


		if (rezultat > 0)


		{


			printf("[Thread %d]Mesajul a fost receptionat...%d\n", tdL.idThread, numar_rasp);



			fflush(stdout);

			//verific raspuns

			if (numar_rasp == varianta_corecta[x])

			{

				printf("corect\n");


				fflush(stdout);


				char s1[1000];



		      	        memset(s1, 0, sizeof(s1));


				strcpy(s1,"corect");

				if(write(tdL.cl,s1,sizeof(s1) )<0)



				{

					 printf("[Thread %d]\n", tdL.idThread);


			       	 perror("Eroare la scrierea verificarii.\n");

			       	 break;

				}


				jucatori[tdL.idThread].punctaj++;


			}

			else if (numar_rasp == 112)    //112 semnal cum ca timpul de raspuns al clientului a depasit 10 secunde


			{


				printf("Timpul a expirat pentru intrebarea %i. Nu se va acorda punctaj.\n", x);


				fflush(stdout);


				break;


			}


			else

			{

				printf("raspuns gresit\n");


				fflush(stdout);


				char s2[1000];


		                memset(s2, 0, sizeof(s2));


				strcpy(s2,"gresit");


				if(write(tdL.cl, s2,sizeof(s2) )<0)

				{

					printf("[Thread %d]\n", tdL.idThread);


			        	perror("Eroare la scrierea verificarii.\n");



			      		break;

				}



			}


			//trimitem punctajul curent clientului
			memset(rasp_juc, 0, sizeof(rasp_juc));



			sprintf(rasp_juc, "%d", jucatori[tdL.idThread].punctaj);

			  if (write(tdL.cl, rasp_juc, sizeof(rasp_juc)) <= 0)

			{


				printf("[Thread %d]\n", tdL.idThread);


				perror("Eroare la scrierea punctajului.\n");


			}


		}//de la varianta data de client


	}//de la for


	
	printf("Clientul %s a terminat jocul.\n", jucatori[tdL.idThread].nume);


	fflush(stdout);


	terminat[tdL.idThread] = 1;



	/// asteptam pana termina toti si trimitem jucatorii cu punctajul maxim 



	while (jucatorii_au_terminat() == 0)

		;
	
	char castigatorii[100] = { 0 };


	int punctaj = 0;

	char punctajj[100];


	nr_castigatori = 0;


	
	memset(castigatorii, 0, sizeof(castigatorii));


	memset(clasament_final, 0, sizeof(clasament_final));
	
	
	memset(punctajj, 0, sizeof(punctajj));




	punctaj = punctaj_maxim(castigatorii);

	
	if (nr_castigatori == 1)

		strcpy(clasament_final, "Castigatorul este : ");


	else


		strcpy(clasament_final, "Castigatorii sunt : ");


	strcat(clasament_final, castigatorii);



	strcat(clasament_final, "\nAvand punctajul  : ");



	sprintf(punctajj, "%d", punctaj);


	strcat(clasament_final, punctajj);



	if (write(tdL.cl, clasament_final, sizeof(clasament_final)) <= 0)


	{

		printf("[Thread %d]\n", tdL.idThread);

		perror("Eroare la scrierea punctajului.\n");


	}
	
	if(iesit == 0)
	
		nr_terminati++;
	


}




int punctaj_maxim(char str[100])

{

	int punctajmax = -10;


	char castigatori[100];



	  for(int i =0; i<nr_jucatori ; i++)	

		if (jucatori[i].punctaj > punctajmax)


			{
				punctajmax = jucatori[i].punctaj;


			}



	memset(castigatori, 0, sizeof(castigatori));



           for(int i =0; i<nr_jucatori ; i++)	



		if (jucatori[i].punctaj == punctajmax)

			{


				strcat(castigatori, jucatori[i].nume);


				strcat(castigatori, "  ");

				nr_castigatori++;

			}


	strcpy(str, castigatori);

	return punctajmax;



}

int jucatorii_au_terminat()

{


	for (int i = 0; i < nr_jucatori; i++)

		if (terminat[i] != 1)

			return 0;



	return 1;


}



////


void intrebare(int i, char sir[1000])

{

	char intrebare1[100];


	char varianta1[100];


	char varianta2[100];


	char varianta3[100];


	char varianta4[100];

	if( i == 6)


		i = 26;

	if(i == 7)


		i = 31;	


	if (i == 2)

		i = 6;

	if (i == 3)

		i= 11;

	if (i == 4)

		i = 16;


	if (i == 5)

		i = 21;


	//strncpy(intrebare1,intrebari_variante[j],sizeof(intrebare1));


	int v = 0;



	while (intrebari_variante[i][v] != '\0')

	{

		intrebare1[v] = intrebari_variante[i][v];


		v++;

	}



	intrebare1[v] = '\0';

	v = 0;

	while (intrebari_variante[i + 1][v] != '\0')


	{


		varianta1[v] = intrebari_variante[i + 1][v];

		v++;

	}


	varianta1[v] = '\0';

	v = 0;


	while (intrebari_variante[i + 2][v] != '\0')


	{

		varianta2[v] = intrebari_variante[i + 2][v];
		v++;

	}


	varianta2[v] = '\0';

	v = 0;


	while (intrebari_variante[i + 3][v] != '\0')

	{

		varianta3[v] = intrebari_variante[i + 3][v];


		v++;



	}

	varianta3[v] = '\0';


	v = 0;


	while (intrebari_variante[i + 4][v] != '\0')


	{

		varianta4[v] = intrebari_variante[i + 4][v];


		v++;


	}



	varianta4[v] = '\0';

	strcat(sir, "Intrebare: ");


	strcat(sir, intrebare1);

	strcat(sir, "\n");


	strcat(sir, "Variante :");


	strcat(sir, "\n");


	strcat(sir, "1.");


	strcat(sir, varianta1);


	strcat(sir, "\n");


	strcat(sir, "2.");


	strcat(sir, varianta2);


	strcat(sir, "\n");


	strcat(sir, "3.");


	strcat(sir, varianta3);


	strcat(sir, "\n");


	strcat(sir, "4.");

	strcat(sir, varianta4);

	strcat(sir, "\n");

}


void XMLParser(FILE *XML_questions)


{


	char Response[10000];



	char line[500];



	while (fgets(line, sizeof(line), XML_questions))

	{

		strcat(Response, line);

	}



	//printf("%s\n",Response);


	int n = strlen(Response);

	Response[n] = '\0';


	int k = 1;

	
	//formam matricea de caractere
		
	for (int i = 1; i <= 7; i++)



	{

		char x;

		if (i == 1)

			x = '1';


		else if (i == 2)


			x = '2';


		else if (i == 3)

			x = '3';

		else if (i == 4)

			x = '4';


		else if (i == 5)

			x = '5';

		else if (i == 6)


			x = '6';


      		else if(i == 7)


			x = '7';

		char str[2];


		str[0] = x;



		char Tagq[] = "question";


		strcat(Tagq, str);


		GetXmlTagValue(Response, Tagq, intrebari_variante[k]);

		char A[] = "optionA";


		strcat(A, str);


		GetXmlTagValue(Response, A, intrebari_variante[k + 1]);

		char B[] = "optionB";

		strcat(B, str);

		GetXmlTagValue(Response, B, intrebari_variante[k + 2]);


		char C[] = "optionC";


		strcat(C, str);


		GetXmlTagValue(Response, C, intrebari_variante[k + 3]);

		char D[] = "optionD";


		strcat(D, str);

		GetXmlTagValue(Response, D, intrebari_variante[k + 4]);

		char ans[100] = { 0 };

		char R[] = "answer";


		strcat(R, str);


		GetXmlTagValue(Response, R, ans);


		varianta_corecta[i] = ans[0] - '0';


		//Print Tag Value



		/*printf("%s\n",intrebari_variante[k]);


		  printf("%s\n",intrebari_variante[k+1]);

		  printf("%s\n",intrebari_variante[k+2]);

		  printf("%s\n",intrebari_variante[k+3]);


		  printf("%s\n",intrebari_variante[k+4]);

		  printf("%d\n",varianta_corecta[i]); */




		k += 5;



	}

}



int GetXmlTagValue(char *RespBuffer, char Tag[100], char *TagValue)

{

	int len = 0, pos = 0;



	char FirstTag[100] = { 0 };	//First Tag


	char SecondTag[100] = { 0 };	//Second Tag

	int PosFirstTag = 0, PosSecondTag = 0;


	//check enter buffer


	len = strlen(RespBuffer);


	// printf("%s",Tag);

	if (len <= 0)

	{

		return -1;


	}

	//Create first Tag

	memset(FirstTag, 0, sizeof(FirstTag));


	strcpy(FirstTag, "<");

	strcat(FirstTag, Tag);


	strcat(FirstTag, ">");


	//  printf("%s\n",FirstTag);


	//Create second tag


	memset(SecondTag, 0, sizeof(SecondTag));


	strcpy(SecondTag, "</");


	strcat(SecondTag, Tag);


	strcat(SecondTag, ">");

	//Get first tag position


	for (pos = 0; pos < len; pos++)

	{

		if (!memcmp(FirstTag, RespBuffer + pos, strlen(FirstTag)))

		{

			PosFirstTag = pos;


			break;


		}

	}


	//Get second tag position


	for (pos = 0; pos < len; pos++)


	{

		if (!memcmp(SecondTag, RespBuffer + pos, strlen(SecondTag)))


		{

			PosSecondTag = pos;

			break;


		}



	}


	if ((PosFirstTag != 0) && (PosSecondTag != 0))

	{


		if (PosSecondTag - PosFirstTag - strlen(FirstTag))


		{

			//Get tag value



			memcpy(TagValue, RespBuffer + PosFirstTag + strlen(FirstTag), PosSecondTag - PosFirstTag - strlen(FirstTag));



			if (strlen(TagValue))




			{

		//printf("%s\n",TagValue);


				return 1;



			}



		}


	}


	return -1;



}
