/*

Crea un programa signal3.c tomando como base el programa signal1.c del apartado 7, y modifícalo para que el proceso hijo creado intercepte la señal SIGINT
y en vez de realizar la acción por defecto (lo aprendido en el punto 8 te servirá para hacer esto), deberá:

Imprimir un mensaje informando de las malas intenciones de su padre
Esperar durante 2 segundos (sleep)
Enviar al proceso padre la señal de terminación SIGINT (contraataque)
Continuar con la ejecución normal del proceso hijo

*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>


void manejador(int senal){
    int pid = getpid();
    int ppid = getppid();
    printf("Proceso padre (pid = %d) ejecutando señal Kill para matar a este proceso hijo (pid = %d) \n", ppid, pid);
    sleep(2);
    signal(SIGKILL, ppid);
}


void main(int argc, char *argv[]) {
    pid_t pidfork;
    int status;

    pidfork = fork(); // creamos proceso hijo
    if (pidfork == 0) { /* proceso hijo */
    int i=0;
    printf("Hijo: pid %d: ejecutando...\n",getpid());
    signal(SIGINT, manejador);
    while(1) {sleep(1); printf("Hijo: %d seg\n",++i); } // bucle infinito
    }

    else {/* proceso padre */
    sleep(2);
    printf("\nPadre: pid %d: mandando señal SIGINT\n",getpid());
    kill(pidfork,SIGINT);
    while (pidfork != wait(&status));
    if (WIFEXITED(status)) { // el proceso ha terminado con un exit()
      printf("El proceso terminó con estado %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) { // el proceso ha terminado por la recepción de una señal
      printf("El proceso terminó al recibir la señal %d\n", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) { // el proceso se ha parado por la recepción de una señal
      printf("El proceso se ha parado al recibir la señal %d\n", WSTOPSIG(status));
    }
  }
  exit(0);
}