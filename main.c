/* ************************************************************************ */
/*                          Microshell UNIX                                 */
/* ************************************************************************ */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>

#define MAX_BUFOR 1024                  // maksymalna dlugosc linii
#define MAX_ARGS 32                     // maksymalna liczba argumentow
#define SEPARATORY "\n\t "              // separatory tokenow
#define MAX_SCIEZKA 128                 // max dlugosc sciezki
#define MAX_IMG_LEN 2048                // max rozmiar pliku z grafika ascii
#define HOST 64

void print_image(FILE *fptr)            // funkcja wyswietlajaca graf ascii
{
    char read_string[MAX_IMG_LEN];

    while(fgets(read_string, sizeof(read_string), fptr) != NULL){
        printf("%s", read_string);
    }       
}

int main(){

    char komenda[MAX_BUFOR];
    char * argumenty[MAX_ARGS];         // wskazniki do stringow arg (w argumenty[])
    char ** arg;                        // wskaznik do kolejnych argumentow
    char * user = getenv("USER");       // zmienna zawierajaca nazwe uzytkownika
    char hostname[HOST];                // tablica przechowujaca nazwe hosta
    char cwd[MAX_SCIEZKA];              // znak zachety(tablica ze sciezka)
    char * aktualna_sciezka = cwd;      // wskaznik do sciezki path
    char last_cwd[MAX_SCIEZKA];         // tablica przechowujaca ostatnia sciezke (do polecenia 'cd -')
    char * ostatnia_sciezka = last_cwd;
    char * $path = getenv("PATH");                                  // do wypisania zmiennej srodowiskowej PATH
    char * katalog_domowy = getenv("HOME");                         // sprawdza zmienna ze sciezka do katalogu domowego
    char * filename1 = "/home/s444372/Desktop/projekt/image1.txt";  // grafika podczas uruchomiania programu
    char * filename2 = "/home/s444372/Desktop/projekt/image2.txt";  // help
    char cwd_temp[MAX_SCIEZKA];         // przechowuje adres o ibcina ostatni folder do komendy 'cd ..'
    char * folder_nadrzedny = cwd_temp; // do komendy 'cd ..' przechowuje sciezke do adresu nadrzednego
    FILE * wskazany_plik = NULL;
    //DIR *dir;

    
    if((wskazany_plik = fopen(filename1,"r")) == NULL){          // otwiera plik .txt z grafika ascii
        fprintf(stderr," error opening ascii-img: %s\n",filename1);
    }else{
    print_image(wskazany_plik);
    fclose(wskazany_plik);                                       // wypisuje i zamyka
    }

    while(!feof(stdin)){                                         // czyta dopoki 'exit' lub eof

/* *********************** wyswietlanie znaku zachety prompt ************************** */

        if(getcwd(cwd, sizeof(cwd)) != NULL){                   // prompt
            gethostname(hostname, HOST);
            fprintf(stdout, "[%s@%s:", user, hostname);         // wyswietla nazwy hosta i uzytkownika
            fputs(aktualna_sciezka, stdout);                             // wypisuje znak zachety w postaci [pwd] $
            fprintf(stdout, "]$ ");
        }

/* **************************** tokenizacja wejscia *********************************** */
/* **************************** input tokenisation ************************************ */

        if(fgets(komenda, MAX_BUFOR, stdin)){                   // czyta komendy z stdin

            arg = argumenty;
            *arg++ = strtok(komenda, SEPARATORY);               // tokenizuje input
            while((*arg++ = strtok(NULL, SEPARATORY)));         // ostatni na wejsciu jest NULL

            if(argumenty[0]){                                   // jesli cos zostalo wpisane do st wejscia

                if(!strcmp(argumenty[0], "exit")){              // komenda 'exit' (break z while)
                    break;
                }

/* **************************** wlasna implementacja 'echo $PATH' ******************************* */

                if(!strcmp(argumenty[0], "echo")){
                    if(!strcmp(argumenty[1], "$PATH")){
                        if($path == NULL){
                            fprintf(stderr, "*** Couldn't print $PATH.\n Value of errno = %d. %s\n", errno, strerror(errno));
                            continue;
                        }
                        else{
                            fprintf(stdout, "$PATH = %s\n", $path);
                            continue;
                        }
                    }
                }
/* ************************************ polecenie pwd ********************************** */

                if(!strcmp(argumenty[0], "pwd")){
                    if(getcwd(cwd, sizeof(cwd)) != NULL){                   
                        fputs(aktualna_sciezka, stdout);                             // wyswietla aktualna sciezke
                        fprintf(stdout, "\n");
                    }
                    else{
                        fprintf(stderr, "*** Unnable to check pwd\n*** Value of errno = %d. %s\n", errno, strerror (errno));
                    }
                    continue;
                }

/* *********************** wlasna implementacja polecenia cd ************************** */
/* *********************** cd command own implementation (it is not exec) ************* */

                if(!strcmp(argumenty[0], "cd")){
                    if(argumenty[1]){;}                         // na poczatku komenda 'cd' bez argumentow, w tej linii sprawdza, czy jest argument, a jesli nie ma przechodzi do "else" i wykonuje 'cd' bez argumentow                                                 
                    else{                                       // else(czyli jesli nie ma), dziala jak komenda cd bez argumentow
                        if(chdir(katalog_domowy) != 0){
                            fprintf(stderr, "*** Error in chdir('%s')\n*** Value of errno = %d: %s\n", katalog_domowy, errno, strerror (errno));
                        }
                        else{
                            strncpy(ostatnia_sciezka, aktualna_sciezka, MAX_SCIEZKA);
                        }
                        continue;
                    }

                    if(!strcmp(argumenty[1], "~")){             // działanie analogiczne do 'cd ~'
                        if(chdir(katalog_domowy) != 0){
                        fprintf(stderr, "*** Error in chdir('%s')\n*** Value of errno = %d: %s\n", katalog_domowy, errno, strerror (errno));
                        }
                        else{
                            strncpy(ostatnia_sciezka, aktualna_sciezka, MAX_SCIEZKA);
                        }
                        continue;
                    }
                    if(!strcmp(argumenty[1], "-")){             // analogicznie do 'cd -'
                        if(chdir(ostatnia_sciezka) !=0 ){
                            fprintf(stderr, "*** Error in chdir('%s')\n*** Value of errno = %d: %s\n", ostatnia_sciezka, errno, strerror (errno));
                        }
                        else{
                            strncpy(ostatnia_sciezka, aktualna_sciezka, MAX_SCIEZKA);   //zapisuje aktualna sciezke na wypadek pozniejszego wywolania 'cd -'
                        }
                        continue;
                    }
                    if(!strcmp(argumenty[1], "..")){            // analogicznie do 'cd ..'
                        int stop = 0;
                        int n = strlen(aktualna_sciezka);

                        for(int i = 0; i<n; i++){
                            if(aktualna_sciezka[i]=='/'){       // szuka ostatniego slasha "/"
                                stop = i;
                            }
                        }
                        int delete = n - stop;                      // tyle charow trzeba usunac ze stringa, aby dal sciezke 'path' rodzica
                        strncpy(folder_nadrzedny, aktualna_sciezka, MAX_SCIEZKA);   //kopiuje sciezke do zapasowej tablicy (w razie bledu)
                        folder_nadrzedny[n-delete] = '\0';          //usuwa "/ostatni folder w adresie"
                        
                        if(chdir(folder_nadrzedny)!=0){
                            fprintf(stderr, "*** Error in chdir('%s')\n*** Value of errno = %d: %s\n", folder_nadrzedny, errno, strerror (errno));
                        }
                        else{
                            strncpy(ostatnia_sciezka, aktualna_sciezka, MAX_SCIEZKA);   // dla 'cd -'
                            strncpy(aktualna_sciezka, folder_nadrzedny, MAX_SCIEZKA);   // jesli funkcja wykonala sie pomyslnie wpisuje path folderu rodzica do aktualnej path
                        }
                        continue;
                    }
                    if(argumenty[1]){                           // sprawdza, czy jest jakis argument z cd, np "cd /folder", jesli tak, przenosi do folderu, o ile istnieje
                        if(chdir(argumenty[1]) != 0){
                            fprintf(stderr, "*** Error in chdir('%s')\n*** Value of errno = %d: %s\n", argumenty[1], errno, strerror (errno));
                        }
                        else{
                            strncpy(ostatnia_sciezka, aktualna_sciezka, MAX_SCIEZKA);
                        }
                        continue;
                    }                                        
                }

/* ************************************ help **************************************** */

                if(!strcmp(argumenty[0], "help")){
                    pid_t pid = fork();
                    if(pid == -1){              // w razie bledu forka
                        perror ("*** Error occurred while forking");
                        fprintf(stdout, "*** Value of errno: %d\n", errno);
                    }
                    else if(pid > 0){           // rodzic czekajacy na zakonczenie dziecka
                        int status;
                        waitpid(pid, &status, 0);
                    }
                    else{                       // tutaj jest dziecko
                        if((wskazany_plik = fopen(filename2,"r")) == NULL){         //otwiera kolejny plik .txt z grafika ascii
                        fprintf(stderr,"*** Error while opening following ascii-img: %s\n*** Value of errno: %d. %s.",filename2, errno, strerror(errno));
                        fprintf(stdout, "Microshell built-in commands and functions, works same as bash functionalities:\n\n--'exit' - exits from MicroShell\n-- 'help' - help page\n--'cd' - change directory, with parameters: ' .. ', ' - ', ' ~ ', 'new directory'");
                        fprintf(stdout, "examples: 'cd ..', 'cd directory'\n--'pwd' - shows working directory\n--'echo $PATH' - shows environmental variable\nYou may also run all Unix executables with parameters, for example: 'ls -l'.\n");
                        exit(1);
                        }else{
                        print_image(wskazany_plik);
                        fclose(wskazany_plik);
                        exit(0);
                        }
                    }
                    fprintf(stdout, "\n"); continue;
                }               

/* *********************** wywolanie funkcji systemowych execiem ************************** */
/* *********************** system functiona calling with exec ***************************** */

/*jesli nie wywolano zadnej z powyzszych komend podaje komende z parametrami funkcjom systemowym*/

                pid_t pid = fork();
                if(pid == -1){              // w razie bledu forka /-/ if error
                    perror("*** Error occurred while forking");
                    fprintf(stdout, "*** Value of errno: %d\n", errno);
                    exit(1);
                }
                else if(pid > 0){           // rodzic czekajacy na zakonczenie dziecka /-/ parent
                    int status;
                    waitpid(pid, &status, 0);
                }
                else{                       // tutaj jest dziecko /-/ child
                    arg = argumenty;
                    if(execvp(argumenty[0], arg++)<0){                  // wywolanie funkcji systemowej z argumentami arg++
                        perror("*** The following error occured in system call execvp()");
                        fprintf(stdout, "*** Value of errno: %d\n", errno); // w razie bledu
                        exit(errno);
                    }
                }
                //   while(*arg)fprintf(stdout, "%s ", *arg++); (moze sie przydac do historii komend)
            }
        }
    };
    return 0;
}
