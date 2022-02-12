//client
//Mareci Ioana Amalia 2A2


#include <sys/types.h>


#include <sys/socket.h>


#include <netinet/in.h>


#include <errno.h>


#include <unistd.h>


#include <stdio.h>


#include <stdlib.h>


#include <netdb.h>



#include <string.h>


#include <signal.h>


#include <time.h>


#define RED     "\x1b[31m"


#define GREEN   "\x1b[32m"


#define RESET   "\x1b[0m"


#define CYAN    "\033[0;36m"


/*codul de eroare returnat de anumite apeluri */


extern int errno;


/*portul de conectare la server*/

int port;

int main(int argc, char *argv[])

{

	int sd;	// descriptorul de socket


	struct sockaddr_in server;	// structura folosita pentru conectare 


	// mesajul trimis

	int nr = 0;


	char buf[1000];


	const int timp_pentru_raspuns = 10;


	time_t timp_inainte, timp_dupa;


	/*exista toate argumentele in linia de comanda? */

	if (argc != 3)

	{

		printf("Sintaxa: %s<adresa_server><port>\n", argv[0]);

		return -1;


	}


	/*stabilim portul */


	port = atoi(argv[2]);


	/*cream socketul */


	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)


	{



		perror("Eroare la socket().\n");

		return errno;

	}


	/*umplem structura folosita pentru realizarea conexiunii cu serverul */


	/*familia socket-ului */

	server.sin_family = AF_INET;


	/*adresa IP a serverului */


	server.sin_addr.s_addr = inet_addr(argv[1]);


	/*portul de conectare */

	server.sin_port = htons(port);


	/*ne conectam la server */


	if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)


	{

		perror("[client]Eroare la connect().\n");

		return errno;

	}


	// identificarea cu nume

	char nume[1001];

	memset(nume, 0, 1000);

	printf("Bine ati venit,introduceti un nume cu care sa fiti identificat: ");


	fflush(stdout);


	if (read(0, nume, sizeof(nume)) < 0)


	{

		perror("[client]Eroare la citirea numelui.\n");

		return errno;

	}

	int leng = strlen(nume) - 1;

	nume[leng] = '\0';


	strncpy(nume, nume, leng);

	/*trimiterea numelui la server */


	if (write(sd, nume, leng + 1) <= 0)

	{

		perror("[client]Eroare la write() spre server.\n");

		return errno;

	}


	// citesc comanda

	memset(buf, 0, sizeof(buf));

	printf("Jucati : 1 \nIesiti : 2\n");

	fflush(stdout);

	if (read(0, buf, sizeof(buf)) < 0)


	{

		perror("[client]Eroare la citirea comenzii.\n");


		return errno;

	}

	leng = strlen(buf) - 1;


	buf[leng] = '\0';


	strncpy(buf, buf, leng);


	if (write(sd, buf, leng + 1) <= 0)

	{

		perror("[client]Eroare la write() spre server.\n");

		return errno;


	}

	int com = atoi(buf);

	if (com == 1)

	{



		printf("Aveti 10 secunde sa raspundeti la fiecare intrebare.\n\n");

		printf("Jocul va incepe in 5 secunde!\n\n");

		printf("Succes!\n\n");

		//sleep(5)

		fflush(stdout);

		int sum=5;

		for (int i = 0; i < 5; i++) { 
		
    		printf("%d\n",sum);
    		
    		sum=sum-1;
    		
		sleep(1);  
		} 


		int ok = 0;

		//citirea intrebarilor date de server 

		for (int i = 1; i <= 7 ; i++)

		{
			ok = 0;

			memset(buf, 0, sizeof(buf));

			if (read(sd, buf, sizeof(buf)) < 0)

			{
				perror("[client]Eroare la read() de la server.\n");


				return errno;
			}


			printf("\n%s", buf);	//intrebarea


			fflush(stdout);


			memset(buf, 0, sizeof(buf));


			printf("Introduceti numarul raspunsului ales: ");


			fflush(stdout);


			time(&timp_inainte);



			//printf("%s\n", ctime(&timp_inainte));



			if (read(0, buf, sizeof(buf)) < 0)

			{

				perror("[client]Eroare la read() de la terminal.\n");

				return errno;



			}

			time(&timp_dupa);


			//printf("%s\n", ctime(&timp_dupa)));


			//printf("%d\n",difftime(timp_inainte, timp_dupa))


			//printf("%f\n",difftime(timp_dupa,timp_inainte);



			if (difftime(timp_dupa, timp_inainte) > timp_pentru_raspuns)


			{



				ok = 1;	// Not a valid answer

				printf(CYAN"Ati raspuns prea tarziu ! Raspunsul nu va fi luat in calcul !\n"RESET);


			}


			if (ok == 1)

			{

				strcpy(buf, "112");


				buf[3] = '\0';


				leng = 2;


			}


			leng = strlen(buf) - 1;


			buf[leng] = '\0';


			strncpy(buf, buf, leng);


			/*trimiterea mesajului la server */


			if (write(sd, buf, sizeof(buf)) <= 0)

			{

				perror("[client]Eroare la write() spre server.\n");

				return errno;

			}


			memset(buf, 0, sizeof(buf));

			//citirea verificarii raspunsului

			if (read(sd, buf, sizeof(buf)) < 0)

			{

				perror("[client]Eroare la citirea punctajului.\n");

				return errno;

			}

			if(strcmp(buf,"corect")==0)

				printf(GREEN " %s \n" RESET, buf);

			else

				printf(RED " %s \n" RESET, buf);


			fflush(stdout);

			//citirea punctajului 

			memset(buf, 0, sizeof(buf));

			if (read(sd, buf, sizeof(buf)) < 0)

			{

				perror("[client]Eroare la citirea punctajului.\n");

				return errno;


			}

			printf("Punctajul curent = %s \n\n", buf);


			fflush(stdout);


		}//de la for

		printf("Jocul s-a terminat \n");


		fflush(stdout);

		//citeste castigatorii

		memset(buf, 0, sizeof(buf));


		if (read(sd, buf, sizeof(buf)) < 0)



		{

			perror("[client]Eroare la citirea punctajului.\n");


			return errno;


		}


		printf("Felicitari!!\n");



		printf("%s \n", buf);


		fflush(stdout);


	}//if-ul cu com==1

	/*inchidem conexiunea, am terminat */

	close(sd);


}
