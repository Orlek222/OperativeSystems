/*------------------------------------------------------------------------------
Proyecto Shell de UNIX. Sistemas Operativos
Grados I. Inform�tica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Algunas secciones est�n inspiradas en ejercicios publicados en el libro
"Fundamentos de Sistemas Operativos", Silberschatz et al.

Para compilar este programa: gcc ProyectoShell.c ApoyoTareas.c -o MiShell
Para ejecutar este programa: ./MiShell
Para salir del programa en ejecuci�n, pulsar Control+D
------------------------------------------------------------------------------*/

#include "ApoyoTareas.h" // Cabecera del m�dulo de apoyo ApoyoTareas.c
 
#define MAX_LINE 256 // 256 caracteres por l�nea para cada comando es suficiente
#include <string.h>  // Para comparar cadenas de cars. (a partir de la tarea 2)


job *tasks;

void manejador(int senal){
    block_SIGCHLD();
    job *item;
    int status;
    int info;
    int pid_wait = 0;
    enum status status_res;
  
    for (int i = 1; i <= list_size(tasks); i++)
    {
        item = get_item_bypos(tasks, i);
        pid_wait = waitpid(item->pgid, &status, WUNTRACED | WNOHANG);
        if (pid_wait == item->pgid)
        {
            status_res = analyze_status(status, &info);
            if(status_res == FINALIZADO){
                printf("\nComando %s ejecutado en segundo plano con PID %d ha concluido\n", item->command, item->pgid);
                delete_job(tasks, item);
            }
            else if(status_res == SUSPENDIDO){
                printf("\nComando %s ejecutado en segundo plano con PID %d ha suspendido su ejecución\n", item->command, item->pgid);
                item->ground = DETENIDO;
            }
        }
    }
    unblock_SIGCHLD();
}



// --------------------------------------------
//                     MAIN          
// --------------------------------------------

int main(void)
{
    char inputBuffer[MAX_LINE]; // B�fer que alberga el comando introducido
    int background;         // Vale 1 si el comando introducido finaliza con '&'
    char *args[MAX_LINE/2]; // La l�nea de comandos (de 256 cars.) tiene 128 argumentos como m�x
                            // Variables de utilidad:
    int pid_fork, pid_wait; // pid para el proceso creado y esperado
    int status;             // Estado que devuelve la funci�n wait
    enum status status_res; // Estado procesado por analyze_status()
    int info;		      // Informaci�n procesada por analyze_status()

    job *item;
    int primerplano;

    ignore_terminal_signals();
    signal(SIGCHLD, manejador);
    tasks = new_list("Tasks");

    while (1) // El programa termina cuando se pulsa Control+D dentro de get_command()
    {  		
    printf("COMANDO->");
    fflush(stdout);
    get_command(inputBuffer, MAX_LINE, args, &background); // Obtener el pr�ximo comando
    if (args[0]==NULL) continue; // Si se introduce un comando vac�o, no hacemos nada
    /* Los pasos a seguir a partir de aqu�, son:
    (1) Genera un proceso hijo con fork()
    (2) El proceso hijo invocar� a execvp()
    (3) El proceso padre esperar� si background es 0; de lo contrario, "continue" 
    (4) El Shell muestra el mensaje de estado del comando procesado 
    (5) El bucle regresa a la funci�n get_command()
    */

        if(strcmp(args[0], "cd")==0){
            int directorio_existe = chdir(args[1]); //Si devuelve -1 es que no existe el directorio
            if(directorio_existe < 0) printf ("\nError, el directorio %s no existe.\n", args[1]);
            continue;
        }
    
        if(strcmp(args[0], "logout")==0){
            exit(0);
        }

        if(strcmp(args[0], "jobs")==0){
            block_SIGCHLD();
            print_job_list(tasks);
            unblock_SIGCHLD();
            continue;
        }

        if(strcmp(args[0], "bg")==0){
            block_SIGCHLD();
            int pos = 1;
            if(args[1] != NULL){
                pos = atoi(args[1]);
            }
            item = get_item_bypos(tasks, pos);
            if(item != NULL && (item->ground == DETENIDO)){
                item->ground = SEGUNDOPLANO;
                killpg(item->pgid, SIGCONT);
            }
            unblock_SIGCHLD();
            continue;
        }

        if(strcmp(args[0], "fg")==0){
            block_SIGCHLD();
            int pos = 1;
            primerplano = 1;
            if(args[1] != NULL){
                pos = atoi(args[1]);
            }
            item = get_item_bypos(tasks, pos);
            if(item!=NULL){
                strcpy(args[0],item->command);
                set_terminal(item->pgid);
                if(item->ground==DETENIDO){
                    killpg(item->pgid, SIGCONT);
                }
                pid_fork = item->pgid;
                delete_job(tasks, item);
            }
            unblock_SIGCHLD();
        }
        if(!primerplano) pid_fork = fork();

        if(pid_fork > 0){ //padre
            if (background == 0){
                waitpid(pid_fork, &status, WUNTRACED);
                set_terminal(getpid());
                
                status_res = analyze_status(status, &info);

                if(status_res == SUSPENDIDO){
                    block_SIGCHLD();
                    item = new_job(pid_fork, args[0], DETENIDO);
                    add_job(tasks, item);
                    printf("\nComando %s ejecutado en primer plano con pid %d. Estado %s. Info: %d\n", args[0], pid_fork,status_strings[status_res], info);
                    unblock_SIGCHLD();
                }
                else if (status_res == FINALIZADO){
                    //Comando pwd ejecutado en primer plano con pid 5615. Estado finalizado. Info: 0.
                    if(info!=255) printf("\nComando %s ejecutado en primer plano con pid %d. Estado %s. Info: %d\n", args[0], pid_fork,status_strings[status_res], info);
                }

                primerplano = 0;
            }
            else{ // Segundo Plano
                block_SIGCHLD();
                item = new_job(pid_fork, args[0], SEGUNDOPLANO);
                add_job(tasks, item);
                //Comando sleep ejecutado en segundo plano con pid 5622.
                printf("\nComando %s ejecutado en segundo plano con pid %d.\n", args[0], pid_fork);
                unblock_SIGCHLD();
            }
        }
        else{
            new_process_group(getpid());

            if (background == 0)
            {
                set_terminal(getpid());
            }
            restore_terminal_signals();
            execvp(args[0], args);
            printf("\nError. Comando %s no encontrado\n", args[0]);
            exit(-1);
        }
    }// end while
   
}


