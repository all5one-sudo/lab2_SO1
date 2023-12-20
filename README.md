# Trabajo Practico 2: Sistemas Operativos 1

## Introduccion
En este archivo se ira documentando el desarrollo del segundo trabajo practico de la materia Sistemas Operativos I. Este repositorio corresponde al alumno Villar Federico Ignacio, de la carrera Ingeniera en Computacion e Ingenieria Electronica. Por separado se mostraran las consignas, implementaciones y resultados obtenidos.

## Herramientas utilizadas
Para el desarrollo del laboratorio, se utilizaron las siguientes herramientas:
- Ubuntu 22.04 LTS
- Visual Studio Code
- Obsidian
## Consigna
### 1. Command line prompt
_myshell_ debe contar con un prompt que contenga el camino al directorio actual.
Por ejemplo, para el home: _username@hostname:~$_
### 2. Internal commands
_myshell_ debe soportar los siguientes comandos internos:
- __cd \> directorio \>__ : cambia el directorio actual a \<directorio\>. Si \<directorio\> no está presente, reporta el directorio actual. Si el directorio no existe se debe imprimir un error apropiado. Además, este comando debe cambiar la variable de entorno PWD. 
Este comando debe soportar la opción *cd -*, que retorna al último directorio de trabajo (__OLDPWD__).
- __clr__: limpia la pantalla
- __echo \<comentario\|env var\>__ : muestra \<comentario\> en la pantalla seguido por una línea nueva. (múltiples espacios\/tabs pueden ser reducidos a un espacio).
- __quit__ : cierra myshell

### 3. Program invocation
Las entradas del usuario que no sean comandos internos deben ser interpretados como la invocación de un programa. La misma tiene que ser realizada mediante _fork_ y _execl_. Debe además soportar paths relativos y absolutos.
### 4. Batch file
_myshell_ debe ser capaz de tomar sus comandos a ejecutar desde un archivo. Por ejemplo, si la shell es invocada con un argumento _myshell batchfile_. El _batchfile_ contiene un conjunto de comandos de línea para que la shell ejecute. 
Cuando se alcance el fin de archivo (EOF), _myshell_ debe cerrarse.
Notar que si myshell se ejecuta sin argumento (.\/myshell), se tiene que mostrar el command prompt y se debe esperar a comandos del usuario vía stdin.
### 5. Background execution
Un ampersand & al final de la línea de comando indica que la shell debe retornar al prompt inmediatamente luego de lanzar al programa a ejecutarse en background.

Cuando se comienza un trabajo en background, se debe imprimir un mensaje indicando su _Trabajo_ y su _ID de proceso_.

`[<job id>] <process id>`

Ejemplo:
`$ echo 'hola' &
[1] 10506
hola`
### 6. Signal Handling
Si se ingresa alguna de las combinaciones CTRL-C, CTRL-Z o CTRL-/\, las señales resultantes (**SIGINT**, **SIGTSTP**, **SIGQUIT** respectivamente) deben ser enviadas al trabajo (job) en ejecución de primer plano en vez de a _myshell_. Si no hay un trabajo (job) en ejecución de primer plano, no debe suceder nada.
### 7. Pipe
_myshell_ provee la funcionalidad de un pipe a través del operador **|** (pipe). El mismo conecta la salida estándar del proceso (stdout) lazando por el comando de la izquierda del pipe con la entrada estándar del proceso (stdin) que se genera con el comando a la derecha del pipe.

Ejemplos:
```
$ last <username> | wc -l
$ ps aux | grep firefox
$ grep bash /etc/passwd | cut -d “:” -f 1 | sort -r
```
##### Responder:
¿Dónde se encuentran los pipes en el filesystem, qué atributos tienen?
### 8. I/O redirection 
Se debe soportar redirección de entrada/salida en _stdin_ y/o _stdout_. 
Por ejemplo:
```
program arg1 ar2 < inputfile > outputfile
```

Ejecuta la el programa _program_ con los arguments _arg1_, _arg2_. _stdin_ es reemplazado por _inputfile_ y _stdout_ por _outputfile_.
La redirección debe funcionar para el comando interno _echo_.

## Resolucion
Para el desarrollo del programa, se trabaja con un archivo llamado `myshell.c`. En el se iran agregando las caracteristicas requeridas por las consignas.
### Command line prompt
Para esta primera parte, se implemento el siguiente codigo:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>

int main() {
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    while(1) {
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
    }
    return 0;
}
```

En la ejecucion de acuerdo a lo solicitado:
- Para obtener el nombre del host: se crea un array de caracteres para almacenar el nombre del host y se utiliza la función `gethostname` para obtenerlo.
```c
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
```
- Para obtener el directorio actual: se crea un array de caracteres para almacenar el directorio actual y se utiliza la función `getcwd` para obtenerlo.
```c
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return 1;
    }
```
- Para obtener el nombre de usuario: se utiliza la función `getpwuid` y `geteuid` para obtener el nombre de usuario.
```c
    struct passwd *pw;
    pw = getpwuid(geteuid());
```
- Se tiene un bucle principal que es infinito y se encarga de imprimir el prompt y espera a que el usuario introduzca un comando (el comentario es porque en los siguientes pasos del trabajo se completara esa parte).
```c
while(1) {
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
    }
```
Al ejecutar el programa, se tiene algo como lo siguente:
```
all5one@all5one-Ubuntu:~/Documents/soi-2023-lab2-myshell-all5one-sudo$ ./myshell
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$
```

### Internal commands
Para esta seccion se implemento el siguiente codigo:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>

int main()
{
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    while (1)
    {
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = 0;
        if (strcmp(command, "clr") == 0)
        {
            system("clear");
        }
        else if (strncmp(command, "cd", 2) == 0)
        {
            char *dir = command + 3;
            if (dir[0] == '\0')
            {
                printf("%s\n", cwd);
            }
            else if (strcmp(dir, "-") == 0)
            {
                if (oldpwd[0] != '\0')
                {
                    strcpy(dir, oldpwd);
                }
                else
                {
                    printf("myshell: cd: OLDPWD not set\n");
                    continue;
                }
            }
            if (chdir(dir) == 0)
            {
                strcpy(oldpwd, cwd);
                if (getcwd(cwd, sizeof(cwd)) == NULL)
                {
                    perror("getcwd() error");
                    return 1;
                }
                setenv("PWD", cwd, 1);
            }
            else
            {
                perror("myshell");
            }
        }
        else if (strncmp(command, "echo", 4) == 0)
        {
            char *arg = command + 5;
            if (arg[0] == '$')
            {
                char *env_var = getenv(arg + 1);
                if (env_var != NULL)
                {
                    printf("%s\n", env_var);
                }
            }
            else
            {
                printf("%s\n", arg);
            }
        }
        else if (strcmp(command, "quit") == 0)
        {
            exit(0);
        }
    }
    return 0;
}
```
#### Comando `clr`
```c
if (strcmp(command, "clr") == 0) {
    system("clear");
}
```
Aquí, `strcmp` compara la cadena ingresada por el usuario (`command`) con la cadena `"clr"`. Si son iguales, se ejecuta el comando `clear` del sistema utilizando la función `system`, que limpia la pantalla.
#### Comando `cd`
```c
else if (strncmp(command, "cd", 2) == 0) {
    char *dir = command + 3;
    if (dir[0] == '\0') {
        printf("%s\n", cwd);
    } else if (strcmp(dir, "-") == 0) {
        if (oldpwd[0] != '\0') {
            strcpy(dir, oldpwd);
        } else {
            printf("myshell: cd: OLDPWD not set\n");
            continue;
        }
    }
```
Aquí, `strncmp` compara los primeros 2 caracteres de la cadena ingresada por el usuario (`command`) con la cadena `"cd"`. Si son iguales, se extrae el argumento del comando (el directorio al que se quiere cambiar), que es la parte de la cadena después del espacio. Si el argumento es una cadena vacía (`dir[0] == '\0'`), se imprime el directorio de trabajo actual (`cwd`). Si el argumento es `"-"`, se cambia al directorio de trabajo anterior (`oldpwd`). Si `oldpwd` está vacío, se imprime un error y se pasa a la siguiente iteración del bucle con `continue`.
```c
    if (chdir(dir) == 0) {
        strcpy(oldpwd, cwd);
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
            return 1;
        }
        setenv("PWD", cwd, 1);
    }
```
Luego, se intenta cambiar al directorio especificado por el argumento usando la función `chdir`. Si el cambio de directorio es exitoso (`chdir(dir) == 0`), se actualiza `oldpwd` con el directorio de trabajo anterior y se obtiene el nuevo directorio de trabajo actual con `getcwd`. Si `getcwd` falla, se imprime un mensaje de error y se termina el programa con un estado de salida de 1. Finalmente, se actualiza la variable de entorno `PWD` con el nuevo directorio de trabajo.
#### Comando `echo`
```c
else if (strncmp(command, "echo", 4) == 0) {
    char *arg = command + 5;
    if (arg[0] == '$') {
        char *env_var = getenv(arg + 1);
        if (env_var != NULL) {
            printf("%s\n", env_var);
        }
    } else {
        printf("%s\n", arg);
    }
}
```
Aquí, `strncmp` compara los primeros 4 caracteres de la cadena ingresada por el usuario (`command`) con la cadena `"echo"`. Si son iguales, se extrae el argumento del comando, que es la parte de la cadena después del espacio.
Si el primer carácter del argumento es `$` (`arg[0] == '$'`), se asume que el usuario quiere imprimir el valor de una variable de entorno. Se usa la función `getenv` para obtener el valor de la variable de entorno, que es la parte de la cadena después del `$`. Si la variable de entorno existe (`env_var != NULL`), se imprime su valor.
Si el primer carácter del argumento no es `$`, se imprime el argumento tal cual. Esto permite al usuario imprimir cualquier cadena de texto.
#### Comando `quit`
```c
else if (strcmp(command, "quit") == 0) {
    exit(0);
}
```
Aquí, `strcmp` compara la cadena ingresada por el usuario (`command`) con la cadena `"quit"`. Si son iguales, se llama a la función `exit` con el argumento `0`.
La función `exit` termina la ejecución del programa y devuelve el control al sistema operativo. El argumento `0` indica que el programa se ha ejecutado correctamente. Cualquier otro valor indicaría que ha ocurrido un error.

#### Ejemplo de ejecucion
Con fines de probar la capacidad de `myshell` hasta ahora, se crea un directorio llamado exampleDir. Luego, se muestra a continuacion una serie de comandos llamados al programa principal, para poder corroborar el correcto funcionamiento de lo implementado hasta el momento:
```
all5one@all5one-Ubuntu:~/Documents/soi-2023-lab2-myshell-all5one-sudo$ ./myshell
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ cd exampleDir
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo/exampleDir$ cd ex
myshell: No such file or directory
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo/exampleDir$ cd -
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ echo Hello, world!
Hello, world!
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ quit
all5one@all5one-Ubuntu:~/Documents/soi-2023-lab2-myshell-all5one-sudo$ 
```

### Program invocation
Se implementa el codigo actual:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>

int main()
{
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    while (1)
    {
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = 0;
        if (strcmp(command, "clr") == 0)
        {
            system("clear");
        }
        else if (strncmp(command, "cd", 2) == 0)
        {
            char *dir = command + 3;
            if (dir[0] == '\0')
            {
                printf("%s\n", cwd);
            }
            else if (strcmp(dir, "-") == 0)
            {
                if (oldpwd[0] != '\0')
                {
                    strcpy(dir, oldpwd);
                }
                else
                {
                    printf("myshell: cd: OLDPWD not set\n");
                    continue;
                }
            }
            if (chdir(dir) == 0)
            {
                strcpy(oldpwd, cwd);
                if (getcwd(cwd, sizeof(cwd)) == NULL)
                {
                    perror("getcwd() error");
                    return 1;
                }
                setenv("PWD", cwd, 1);
            }
            else
            {
                perror("myshell");
            }
        }
        else if (strncmp(command, "echo", 4) == 0)
        {
            char *arg = command + 5;
            if (arg[0] == '$')
            {
                char *env_var = getenv(arg + 1);
                if (env_var != NULL)
                {
                    printf("%s\n", env_var);
                }
            }
            else
            {
                printf("%s\n", arg);
            }
        }
        else if (strcmp(command, "quit") == 0)
        {
            exit(0);
        }
        else
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                perror("myshell");
            }
            else if (pid == 0)
            {
                if (execlp(command, command, NULL) == -1)
                {
                    perror("myshell");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }
    return 0;
}
```
La nueva parte es la siguiente:
```c
else {
    pid_t pid = fork();
    if (pid < 0) {
        perror("myshell");
    } else if (pid == 0) {
        if (execlp(command, command, NULL) == -1) {
            perror("myshell");
            exit(EXIT_FAILURE);
        }
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}
```
Aquí, si el comando ingresado por el usuario no coincide con ninguno de los comandos internos, se crea un nuevo proceso utilizando `fork`.
Si `fork` devuelve un valor negativo, significa que hubo un error al crear el nuevo proceso. En este caso, se imprime un mensaje de error utilizando `perror`.
Si `fork` devuelve `0`, estamos en el proceso hijo. Aquí, se intenta reemplazar el proceso hijo con el programa especificado por el comando utilizando `execlp`. Si `execlp` devuelve `-1`, significa que hubo un error al intentar ejecutar el programa. En este caso, se imprime un mensaje de error y se termina el proceso hijo con un estado de salida de fallo.
Si `fork` devuelve un valor positivo, estamos en el proceso padre y el valor devuelto es el PID del proceso hijo. Aquí, se espera a que el proceso hijo termine utilizando `waitpid`.
#### Ejemplo de ejecucion
Para probar entradas de usuario que no estan implementadas en este shell, se llama a los comandos `ls` y `pwd`. Se tiene lo siguiente:
```
all5one@all5one-Ubuntu:~/Documents/soi-2023-lab2-myshell-all5one-sudo$ ./myshell3
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ ls
exampleDir  lab.md  myshell  myshell1  myshell2  myshell3  myshell.c  README.md
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ pwd
/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ 
```

### Batch file
Se implementa el siguiente codigo:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd);

int main()
{
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    while (1)
    {
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = 0;
        if (strncmp(command, "myshell ", 8) == 0)
        {
            char *filename = command + 8;
            filename[strcspn(filename, "\n")] = 0;

            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("myshell");
                continue;
            }

            char batchCommand[256];
            while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
            {
                batchCommand[strcspn(batchCommand, "\n")] = 0;
                execute_command(batchCommand, cwd, sizeof(cwd), oldpwd);
            }

            fclose(file);
        }
        else
        {
            execute_command(command, cwd, sizeof(cwd), oldpwd);
        }
    }
    return 0;
}

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd)
{
    char *args[256];
    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL)
    {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (strcmp(args[0], "clr") == 0)
    {
        system("clear");
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        char *dir = args[1];
        if (dir == NULL)
        {
            printf("%s\n", cwd);
        }
        else if (strcmp(dir, "-") == 0)
        {
            if (oldpwd[0] != '\0')
            {
                strcpy(dir, oldpwd);
            }
            else
            {
                printf("myshell: cd: OLDPWD not set\n");
                return;
            }
        }
        if (chdir(dir) == 0)
        {
            strcpy(oldpwd, cwd);
            if (getcwd(cwd, cwd_size) == NULL)
            {
                perror("getcwd() error");
                exit(1);
            }
            setenv("PWD", cwd, 1);
        }
        else
        {
            perror("myshell");
        }
    }
    else if (strcmp(args[0], "echo") == 0)
    {
        char *arg = args[1];
        if (arg[0] == '$')
        {
            char *env_var = getenv(arg + 1);
            if (env_var != NULL)
            {
                printf("%s\n", env_var);
            }
        }
        else
        {
            printf("%s\n", arg);
        }
    }
    else if (strcmp(args[0], "quit") == 0)
    {
        exit(0);
    }
    else if (strcmp(args[0], "myshell") == 0)
    {
        char *filename = args[1];
        FILE *file = fopen(filename, "r");
        if (file == NULL)
        {
            perror("myshell");
            return;
        }

        char batchCommand[256];
        while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
        {
            batchCommand[strcspn(batchCommand, "\n")] = 0;
            execute_command(batchCommand, cwd, cwd_size, oldpwd);
        }

        fclose(file);
    }
    else
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("myshell");
        }
        else if (pid == 0)
        {
            if (execvp(args[0], args) == -1)
            {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}
```

Con respecto a las versiones anteriores, ahora se llevo el manejo de las entradas de usuario a una funcion `execute_command()` que se encarga de procesarlas de la misma forma que antes. En el bucle `while(1)` de la funcion `main()` se hace la distincion entre un comando que se pasa directamente a la shell, en cuyo caso se ejecuta `execute_command(command, cwd, sizeof(cwd), oldpwd);`. Caso contrario, al tratarse de una llamada a `myshell <batchfile>`, entonces se parsea el archivo linea por linea, para de esa forma llamar individualmente a la ejecucion de cada comando, esto se hace en las lineas siguientes:
```c
if (strncmp(command, "myshell ", 8) == 0)
        {
            char *filename = command + 8;
            filename[strcspn(filename, "\n")] = 0;

            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("myshell");
                continue;
            }

            char batchCommand[256];
            while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
            {
                batchCommand[strcspn(batchCommand, "\n")] = 0;
                execute_command(batchCommand, cwd, sizeof(cwd), oldpwd);
            }

            fclose(file);
        }
```

#### Ejemplo de ejecucion
Para probar la ejecucion de este punto, se hace uso de un pequeño batch, llamado `batchFileExample.sh`, que tiene las siguientes lineas:
```sh
echo Hello
echo World!
ls
cd exampleDir
echo Bye
cd -
echo Hi!
```
Entonces, como testing se obtiene lo siguiente:
```
all5one@all5one-Ubuntu:~/Documents/soi-2023-lab2-myshell-all5one-sudo$ ./myshell4
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ myshell batchFileExample.sh
Hello
World!
batchFileExample.sh  exampleDir  lab.md  myshell  myshell1  myshell2  myshell3  myshell4  myshell.c  README.md  testFile.txt
Bye
Hi!
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$
```

### Backgroud execution
Se implementa lo sigueinte:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd);

typedef struct
{
    pid_t pid;
    int job_id;
    char command[256];
} Job;

Job jobs[256];
int job_id = 1;

int main()
{
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    while (1)
    {
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = 0;
        if (strncmp(command, "myshell ", 8) == 0)
        {
            char *filename = command + 8;
            filename[strcspn(filename, "\n")] = 0;

            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("myshell");
                continue;
            }

            char batchCommand[256];
            while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
            {
                batchCommand[strcspn(batchCommand, "\n")] = 0;
                execute_command(batchCommand, cwd, sizeof(cwd), oldpwd);
            }

            fclose(file);
        }
        else
        {
            execute_command(command, cwd, sizeof(cwd), oldpwd);
        }
    }
    return 0;
}

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd)
{
    char *args[256];
    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL)
    {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    bool run_in_background = false;
    pid_t pidG;
    if (i > 0 && strcmp(args[i - 1], "&") == 0)
    {
        run_in_background = true;
        args[i - 1] = NULL;
        pidG = getpid();
    }

    if (strcmp(args[0], "clr") == 0)
    {
        system("clear");
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        if (run_in_background)
        {
            jobs[job_id - 1].pid = pidG;
            jobs[job_id - 1].job_id = job_id;
            strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
            printf("[%d] %d ", job_id, pidG);
            job_id++;
        }
        char *dir = args[1];
        if (dir == NULL)
        {
            printf("%s\n", cwd);
        }
        else if (strcmp(dir, "-") == 0)
        {
            if (oldpwd[0] != '\0')
            {
                strcpy(dir, oldpwd);
            }
            else
            {
                printf("myshell: cd: OLDPWD not set\n");
                return;
            }
        }
        if (chdir(dir) == 0)
        {
            strcpy(oldpwd, cwd);
            if (getcwd(cwd, cwd_size) == NULL)
            {
                perror("getcwd() error");
                exit(1);
            }
            setenv("PWD", cwd, 1);
        }
        else
        {
            perror("myshell");
        }
    }
    else if (strcmp(args[0], "echo") == 0)
    {
        if (run_in_background)
        {
            jobs[job_id - 1].pid = pidG;
            jobs[job_id - 1].job_id = job_id;
            strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
            printf("[%d] %d ", job_id, pidG);
            job_id++;
        }
        char *arg = args[1];
        if (arg[0] == '$')
        {
            char *env_var = getenv(arg + 1);
            if (env_var != NULL)
            {
                printf("%s\n", env_var);
            }
        }
        else
        {
            printf("%s\n", arg);
        }
    }
    else if (strcmp(args[0], "quit") == 0)
    {
        exit(0);
    }
    else if (strcmp(args[0], "myshell") == 0)
    {
        char *filename = args[1];
        FILE *file = fopen(filename, "r");
        if (file == NULL)
        {
            perror("myshell");
            return;
        }

        char batchCommand[256];
        while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
        {
            batchCommand[strcspn(batchCommand, "\n")] = 0;
            execute_command(batchCommand, cwd, cwd_size, oldpwd);
        }

        fclose(file);
    }
    else
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork failed");
            exit(1);
        }
        else if (pid == 0)
        {
            if (execvp(args[0], args) == -1)
            {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (run_in_background)
            {
                if (run_in_background)
                {
                    jobs[job_id - 1].pid = pidG;
                    jobs[job_id - 1].job_id = job_id;
                    strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                    printf("[%d] %d ", job_id, pidG);
                    job_id++;
                }
            }
            else
            {
                waitpid(pid, NULL, 0);
            }
        }
    }
}
```
Ahora se hace uso de `stdbool` para definir si se solicito o no una ejecucion en segundo plano. Para ello, dentro del manejador de cada una de las llamadas a usuario, se implementa el sigueinte bloque de cogigo:
```c
if (run_in_background)
        {
            jobs[job_id - 1].pid = pidG;
            jobs[job_id - 1].job_id = job_id;
            strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
            printf("[%d] %d ", job_id, pidG);
            job_id++;
        }
```
Que permite, junto a las siguientes variables manejar la informacion de los procesos en segundo plano. 
```c
typedef struct
{
    pid_t pid;
    int job_id;
    char command[256];
} Job;

Job jobs[256];
int job_id = 1;
```
Para obtener el llamado `pidG`, se hace uso del siguiente bloque de codigo al inicio de la funcion `execute_command()`:
```c
pid_t pidG;
    if (i > 0 && strcmp(args[i - 1], "&") == 0)
    {
        run_in_background = true;
        args[i - 1] = NULL;
        pidG = getpid();
    }
```

#### Ejemplo de ejecucion
```
all5one@all5one-Ubuntu:~/Documents/soi-2023-lab2-myshell-all5one-sudo$ ./myshell5
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ echo Hi! &
[1] 167524 Hi!
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ echo bye! &
[2] 167524 bye!
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ 
```

### Signal handling
Se implementa la siguiente solucion:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd);
void handle_sigint(int sig);
void handle_sigtstp(int sig);
void handle_sigquit(int sig);
void handle_sigchld(int sig);

sigjmp_buf jmpbuf;
pid_t foreground_job_pid = 0;
pid_t flag_job_pid = 0;
int job_id = 1;

typedef struct
{
    pid_t pid;
    int job_id;
    char command[256];
} Job;

Job jobs[256];

int main()
{
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    signal(SIGQUIT, handle_sigquit);
    signal(SIGCHLD, handle_sigchld);
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    //sigsetjmp(jmpbuf, 1);
    while (1)
    {
        sigsetjmp(jmpbuf, 1);
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = 0;
        if (strncmp(command, "myshell ", 8) == 0)
        {
            char *filename = command + 8;
            filename[strcspn(filename, "\n")] = 0;

            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("myshell");
                continue;
            }

            char batchCommand[256];
            while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
            {
                batchCommand[strcspn(batchCommand, "\n")] = 0;
                execute_command(batchCommand, cwd, sizeof(cwd), oldpwd);
            }

            fclose(file);
        }
        else
        {
            execute_command(command, cwd, sizeof(cwd), oldpwd);
        }
    }
    return 0;
}

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd)
{
    char *args[256];
    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL)
    {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    bool run_in_background = false;
    pid_t pidG;
    if (i > 0 && strcmp(args[i - 1], "&") == 0)
    {
        run_in_background = true;
        args[i - 1] = NULL;
        pidG = getpid();
    }

    if (strcmp(args[0], "clr") == 0)
    {
        system("clear");
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        if (run_in_background)
        {
            jobs[job_id - 1].pid = pidG;
            jobs[job_id - 1].job_id = job_id;
            strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
            printf("[%d] %d ", job_id, pidG);
            job_id++;
        }
        char *dir = args[1];
        if (dir == NULL)
        {
            printf("%s\n", cwd);
        }
        else if (strcmp(dir, "-") == 0)
        {
            if (oldpwd[0] != '\0')
            {
                strcpy(dir, oldpwd);
            }
            else
            {
                printf("myshell: cd: OLDPWD not set\n");
                return;
            }
        }
        if (chdir(dir) == 0)
        {
            strcpy(oldpwd, cwd);
            if (getcwd(cwd, cwd_size) == NULL)
            {
                perror("getcwd() error");
                exit(1);
            }
            setenv("PWD", cwd, 1);
        }
        else
        {
            perror("myshell");
        }
    }
    else if (strcmp(args[0], "echo") == 0)
    {
        if (run_in_background)
        {
            jobs[job_id - 1].pid = pidG;
            jobs[job_id - 1].job_id = job_id;
            strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
            printf("[%d] %d ", job_id, pidG);
            job_id++;
        }
        char *arg = args[1];
        if (arg[0] == '$')
        {
            char *env_var = getenv(arg + 1);
            if (env_var != NULL)
            {
                printf("%s\n", env_var);
            }
        }
        else
        {
            printf("%s\n", arg);
        }
    }
    else if (strcmp(args[0], "quit") == 0)
    {
        exit(0);
    }
    else if (strcmp(args[0], "myshell") == 0)
    {
        char *filename = args[1];
        FILE *file = fopen(filename, "r");
        if (file == NULL)
        {
            perror("myshell");
            return;
        }

        char batchCommand[256];
        while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
        {
            batchCommand[strcspn(batchCommand, "\n")] = 0;
            execute_command(batchCommand, cwd, cwd_size, oldpwd);
        }

        fclose(file);
    }
    else
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork failed");
            exit(1);
        }
        else if (pid == 0)
        {
            if (execvp(args[0], args) == -1)
            {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (run_in_background)
            {
                jobs[job_id - 1].pid = pidG;
                jobs[job_id - 1].job_id = job_id;
                strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                printf("[%d] %d ", job_id, pidG);
                job_id++;
            }
            else
            {
                foreground_job_pid = pid;
                waitpid(pid, NULL, 0);
                foreground_job_pid = 0;
            }
        }
    }
}

void handle_sigint(int sig)
{
    printf("\033[33mSIGINT received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        printf("foreground_job_pid: %d\n", foreground_job_pid);
        kill(foreground_job_pid, SIGINT);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigtstp(int sig)
{
    printf("\033[33mSIGSTP received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        kill(foreground_job_pid, SIGTSTP);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigquit(int sig)
{
    printf("\033[33mSIGQUIT received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        kill(foreground_job_pid, SIGQUIT);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigchld(int sig)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
}
```

En este caso se implementan los handlers para cada una de las signals mencionadas, ademas de que se agrega un ultimo para `SIGCHLD`, de forma de evitar la creacion de procesos zombies. Los handlers son:
```c
void handle_sigint(int sig)
{
    printf("\033[33mSIGINT received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        printf("foreground_job_pid: %d\n", foreground_job_pid);
        kill(foreground_job_pid, SIGINT);
    }
    siglongjmp(jmpbuf, 1);
}
void handle_sigtstp(int sig)
{
    printf("\033[33mSIGSTP received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        kill(foreground_job_pid, SIGTSTP);
    }
    siglongjmp(jmpbuf, 1);
}
void handle_sigquit(int sig)
{
    printf("\033[33mSIGQUIT received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        kill(foreground_job_pid, SIGQUIT);
    }
    siglongjmp(jmpbuf, 1);
}
void handle_sigchld(int sig)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
}
```
Con esos handlers, se retorna a la funcion principal del programa, pudiendo obtener de nuevo la posibilidad de enviar entradas de usuario. Esto se hace a traves de un salto con las instrucciones `siglongjmp(jmpbuf, 1);` y `sigsetjmp(jmpbuf, 1);`. Como ejemplo de ejecucion, se tiene lo siguiente:
```html
all5one@all5one-Ubuntu:~/Documents/soi-2023-lab2-myshell-all5one-sudo$ ./myshell6
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ ^CSIGINT received
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ ^ZSIGSTP received
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ ^\SIGQUIT received
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ 
```
### Pipe
La solucion implementada se muestra:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd);
void execute_command_with_pipe(char *command1, char *command2);
void handle_sigint(int sig);
void handle_sigtstp(int sig);
void handle_sigquit(int sig);
void handle_sigchld(int sig);

sigjmp_buf jmpbuf;
pid_t foreground_job_pid = 0;
pid_t flag_job_pid = 0;
int job_id = 1;

typedef struct
{
    pid_t pid;
    int job_id;
    char command[256];
} Job;

Job jobs[256];

int main()
{
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    signal(SIGQUIT, handle_sigquit);
    signal(SIGCHLD, handle_sigchld);
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    while (1)
    {
        sigsetjmp(jmpbuf, 1);
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = 0;
        if (strncmp(command, "myshell ", 8) == 0)
        {
            char *filename = command + 8;
            filename[strcspn(filename, "\n")] = 0;

            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("myshell");
                continue;
            }

            char batchCommand[256];
            while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
            {
                batchCommand[strcspn(batchCommand, "\n")] = 0;
                execute_command(batchCommand, cwd, sizeof(cwd), oldpwd);
            }

            fclose(file);
        }
        else
        {
            execute_command(command, cwd, sizeof(cwd), oldpwd);
        }
    }
    return 0;
}

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd)
{
    char command_copy[256];
    strncpy(command_copy, command, sizeof(command_copy));

    char *command1 = strtok(command, "|");
    char *command2 = strtok(NULL, "|");
    if (command2 != NULL)
    {
        execute_command_with_pipe(command1, command2);
    }
    else
    {
        char *args[256];
        char *token = strtok(command, " ");
        int i = 0;
        while (token != NULL)
        {
            args[i] = token;
            i++;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        bool run_in_background = false;
        pid_t pidG;
        if (i > 0 && strcmp(args[i - 1], "&") == 0)
        {
            run_in_background = true;
            args[i - 1] = NULL;
            pidG = getpid();
        }

        if (strcmp(args[0], "clr") == 0)
        {
            system("clear");
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            if (run_in_background)
            {
                jobs[job_id - 1].pid = pidG;
                jobs[job_id - 1].job_id = job_id;
                strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                printf("[%d] %d ", job_id, pidG);
                job_id++;
            }
            char *dir = args[1];
            if (dir == NULL)
            {
                printf("%s\n", cwd);
            }
            else if (strcmp(dir, "-") == 0)
            {
                if (oldpwd[0] != '\0')
                {
                    strcpy(dir, oldpwd);
                }
                else
                {
                    printf("myshell: cd: OLDPWD not set\n");
                    return;
                }
            }
            if (chdir(dir) == 0)
            {
                strcpy(oldpwd, cwd);
                if (getcwd(cwd, cwd_size) == NULL)
                {
                    perror("getcwd() error");
                    exit(1);
                }
                setenv("PWD", cwd, 1);
            }
            else
            {
                perror("myshell");
            }
        }
        else if (strcmp(args[0], "echo") == 0)
        {
            if (run_in_background)
            {
                jobs[job_id - 1].pid = pidG;
                jobs[job_id - 1].job_id = job_id;
                strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                printf("[%d] %d ", job_id, pidG);
                job_id++;
            }
            char *arg = args[1];
            if (arg[0] == '$')
            {
                char *env_var = getenv(arg + 1);
                if (env_var != NULL)
                {
                    printf("%s\n", env_var);
                }
            }
            else
            {
                printf("%s\n", arg);
            }
        }
        else if (strcmp(args[0], "quit") == 0)
        {
            exit(0);
        }
        else if (strcmp(args[0], "myshell") == 0)
        {
            char *filename = args[1];
            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("myshell");
                return;
            }

            char batchCommand[256];
            while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
            {
                batchCommand[strcspn(batchCommand, "\n")] = 0;
                execute_command(batchCommand, cwd, cwd_size, oldpwd);
            }

            fclose(file);
        }
        else
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                perror("fork failed");
                exit(1);
            }
            else if (pid == 0)
            {
                if (execvp(args[0], args) == -1)
                {
                    perror("myshell");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                if (run_in_background)
                {
                    jobs[job_id - 1].pid = pidG;
                    jobs[job_id - 1].job_id = job_id;
                    strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                    printf("[%d] %d ", job_id, pidG);
                    job_id++;
                }
                else
                {
                    foreground_job_pid = pid;
                    waitpid(pid, NULL, 0);
                    foreground_job_pid = 0;
                }
            }
        }
    }
}

void execute_command_with_pipe(char *command1, char *command2)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }

    pid1 = fork();
    if (pid1 == -1)
    {
        perror("fork");
        return;
    }
    else if (pid1 == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execute_command(command1, cwd, sizeof(cwd), oldpwd);
        exit(EXIT_SUCCESS);
    }
    waitpid(pid1, NULL, 0);
    pid2 = fork();
    if (pid2 == -1)
    {
        perror("fork");
        return;
    }
    if (pid2 == 0)
    {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execute_command(command2, cwd, sizeof(cwd), oldpwd);
        exit(EXIT_SUCCESS);
    }
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void handle_sigint(int sig)
{
    printf("\033[33mSIGINT received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        printf("foreground_job_pid: %d\n", foreground_job_pid);
        kill(foreground_job_pid, SIGINT);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigtstp(int sig)
{
    printf("\033[33mSIGSTP received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        kill(foreground_job_pid, SIGTSTP);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigquit(int sig)
{
    printf("\033[33mSIGQUIT received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        kill(foreground_job_pid, SIGQUIT);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigchld(int sig)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
}
```

Se implemento una nueva funcion, `execute_command_with_pipe()`, que se utiliza para ejecutar dos comandos en un shell, donde la salida del primer comando se utiliza como entrada para el segundo comando. Esto se logra utilizando una tubería (pipe), que es una forma de redirección que se utiliza en Linux y otros sistemas operativos similares a Unix.

Aquí está un desglose paso a paso de lo que hace esta función:
1. Crea un pipe utilizando la función `pipe()`. Esto crea dos descriptores de archivo: uno para el extremo de lectura del pipe y otro para el extremo de escritura.
2. Crea un nuevo proceso utilizando la función `fork()`. Esto duplica el proceso actual, creando un proceso hijo que es una copia exacta del proceso padre.
3. En el proceso hijo, cierra el extremo de lectura de la tubería y redirige la salida estándar (STDOUT) al extremo de escritura de la tubería utilizando la función `dup2()`. Luego, ejecuta el primer comando utilizando una función como `execvp()`.
4. Crea otro proceso hijo para ejecutar el segundo comando.
5. En el segundo proceso hijo, cierra el extremo de escritura de la tubería y redirige la entrada estándar (STDIN) al extremo de lectura de la tubería. Luego, ejecuta el segundo comando.
6. Finalmente, el proceso padre espera a que ambos procesos hijo terminen utilizando la función `waitpid()`.

```c
void execute_command_with_pipe(char *command1, char *command2)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }

    pid1 = fork();
    if (pid1 == -1)
    {
        perror("fork");
        return;
    }
    else if (pid1 == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execute_command(command1, cwd, sizeof(cwd), oldpwd);
        exit(EXIT_SUCCESS);
    }
    waitpid(pid1, NULL, 0);
    pid2 = fork();
    if (pid2 == -1)
    {
        perror("fork");
        return;
    }
    if (pid2 == 0)
    {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execute_command(command2, cwd, sizeof(cwd), oldpwd);
        exit(EXIT_SUCCESS);
    }
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}
```

#### Ejemplos de ejecucion
```
all5one@all5one-Ubuntu:~/Documents/soi-2023-lab2-myshell-all5one-sudo$ ./myshell7
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ ps aux | grep firefox
all5one    10405  8.8  2.4 12575520 596708 ?     Sl   11:04  26:03 /snap/firefox/2356/usr/lib/firefox/firefox
all5one    10547  0.0  0.1 222968 37464 ?        Sl   11:04   0:00 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -parentBuildID 20230214170610 -prefsLen 28516 -prefMapSize 229091 -appDir /snap/firefox/2356/usr/lib/firefox/browser {bb0e2de7-7e7c-42af-ba15-767a3a5766fc} 10405 true socket
all5one    10565  0.0  0.4 2447144 111600 ?      Sl   11:04   0:02 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -childID 1 -isForBrowser -prefsLen 28657 -prefMapSize 229091 -jsInitLen 246560 -parentBuildID 20230214170610 -appDir /snap/firefox/2356/usr/lib/firefox/browser {5596b57d-c6ee-4843-a3e3-f65ab4973a78} 10405 true tab
all5one    10734  0.0  0.4 2459720 103660 ?      Sl   11:04   0:01 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -childID 2 -isForBrowser -prefsLen 34111 -prefMapSize 229091 -jsInitLen 246560 -parentBuildID 20230214170610 -appDir /snap/firefox/2356/usr/lib/firefox/browser {7ed2144f-e3d5-4762-a715-24f4ce52d0f4} 10405 true tab
all5one    11078 17.8  2.1 3068628 531468 ?      Sl   11:04  52:44 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -childID 6 -isForBrowser -prefsLen 34098 -prefMapSize 229091 -jsInitLen 246560 -parentBuildID 20230214170610 -appDir /snap/firefox/2356/usr/lib/firefox/browser {0c63397a-659f-44d1-b48b-e533656504ff} 10405 true tab
all5one    11141  0.0  0.7 2591004 188688 ?      Sl   11:04   0:14 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -childID 7 -isForBrowser -prefsLen 34120 -prefMapSize 229091 -jsInitLen 246560 -parentBuildID 20230214170610 -appDir /snap/firefox/2356/usr/lib/firefox/browser {b4a3b636-9057-481f-998e-7b14e7606dc6} 10405 true tab
all5one    11156  6.2  0.3 406080 79688 ?        Sl   11:05  18:21 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -parentBuildID 20230214170610 -prefsLen 34121 -prefMapSize 229091 -appDir /snap/firefox/2356/usr/lib/firefox/browser {621a678f-06f0-43be-8687-efc01d2f49d9} 10405 true rdd
all5one    11158  1.9  0.1 228408 37340 ?        Sl   11:05   5:45 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -parentBuildID 20230214170610 -sandboxingKind 0 -prefsLen 34121 -prefMapSize 229091 -appDir /snap/firefox/2356/usr/lib/firefox/browser {165ce8ab-2a8a-49c7-b717-dd95a1645b03} 10405 true utility
all5one   147455  0.1  0.7 2561764 184396 ?      Sl   12:35   0:15 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -childID 46 -isForBrowser -prefsLen 34122 -prefMapSize 229091 -jsInitLen 246560 -parentBuildID 20230214170610 -appDir /snap/firefox/2356/usr/lib/firefox/browser {6ef7edc3-e5c9-41b3-9b8e-19b2e4e766f9} 10405 true tab
all5one   169056  0.2  0.6 2523344 168316 ?      Sl   14:21   0:15 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -childID 59 -isForBrowser -prefsLen 34179 -prefMapSize 229091 -jsInitLen 246560 -parentBuildID 20230214170610 -appDir /snap/firefox/2356/usr/lib/firefox/browser {f6b6100f-9014-4aa3-8a7a-837785472d51} 10405 true tab
all5one   171923  0.0  0.3 2422216 77092 ?       Sl   15:00   0:01 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -childID 63 -isForBrowser -prefsLen 34179 -prefMapSize 229091 -jsInitLen 246560 -parentBuildID 20230214170610 -appDir /snap/firefox/2356/usr/lib/firefox/browser {08bc8a4c-5118-4736-9e0b-39738fc37533} 10405 true tab
all5one   171988  0.0  0.2 2403932 61996 ?       Sl   15:05   0:00 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -childID 64 -isForBrowser -prefsLen 34179 -prefMapSize 229091 -jsInitLen 246560 -parentBuildID 20230214170610 -appDir /snap/firefox/2356/usr/lib/firefox/browser {3477d1a2-b080-4a55-bc88-3f6f679a9e77} 10405 true tab
all5one   172047  0.0  0.2 2403936 62172 ?       Sl   15:11   0:00 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -childID 65 -isForBrowser -prefsLen 34179 -prefMapSize 229091 -jsInitLen 246560 -parentBuildID 20230214170610 -appDir /snap/firefox/2356/usr/lib/firefox/browser {052b6811-9b9b-4afc-a4f3-268dff3941d8} 10405 true tab
all5one   172204  0.0  0.2 2403928 62504 ?       Sl   15:16   0:00 /snap/firefox/2356/usr/lib/firefox/firefox -contentproc -childID 66 -isForBrowser -prefsLen 34178 -prefMapSize 229091 -jsInitLen 246560 -parentBuildID 20230214170610 -appDir /snap/firefox/2356/usr/lib/firefox/browser {4bd5ca5a-c058-4d6a-a22a-133aaa2ae895} 10405 true tab
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ ls | wc -l
13
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ echo Hi! | wc -c
4
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ 
```

#### Pipes en filesystem
Los pipes no se encuentran en el sistema de archivos tradicional. Son una característica proporcionada por el kernel de Linux y existen en memoria. No tienen una ubicación en el sistema de archivos y no tienen atributos de archivo tradicionales como los que se pueden encontrar en un archivo regular, como permisos de lectura/escritura, propietario, grupo, etc. En lugar de eso, un pipe tiene un buffer en memoria que puede contener datos que se están pasando de un proceso a otro. Este buffer tiene un tamaño máximo, que puede variar dependiendo del sistema operativo y la configuración del sistema. Además, cada extremo del pipe tiene un indicador de archivo (file descriptor) asociado con él. Estos indicadores de archivo se pueden usar para leer o escribir en el pipe, dependiendo de cómo se creó el pipe.

### I/O redirection
Se implementa lo siguiente:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd);
void execute_command_with_pipe(char *command1, char *command2);
void handle_sigint(int sig);
void handle_sigtstp(int sig);
void handle_sigquit(int sig);
void handle_sigchld(int sig);

sigjmp_buf jmpbuf;
pid_t foreground_job_pid = 0;
pid_t flag_job_pid = 0;
int job_id = 1;

typedef struct
{
    pid_t pid;
    int job_id;
    char command[256];
} Job;

Job jobs[256];

int main()
{
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    signal(SIGQUIT, handle_sigquit);
    signal(SIGCHLD, handle_sigchld);
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return 1;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    while (1)
    {
        sigsetjmp(jmpbuf, 1);
        printf("%s@%s:%s$ ", pw->pw_name, hostname, cwd);
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            break;
        }
        command[strcspn(command, "\n")] = 0;
        if (strncmp(command, "myshell ", 8) == 0)
        {
            char *filename = command + 8;
            filename[strcspn(filename, "\n")] = 0;

            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("myshell");
                continue;
            }

            char batchCommand[256];
            while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
            {
                batchCommand[strcspn(batchCommand, "\n")] = 0;
                execute_command(batchCommand, cwd, sizeof(cwd), oldpwd);
            }

            fclose(file);
        }
        else
        {
            execute_command(command, cwd, sizeof(cwd), oldpwd);
        }
    }
    return 0;
}

void execute_command(char *command, char *cwd, size_t cwd_size, char *oldpwd)
{
    char command_copy[256];
    strncpy(command_copy, command, sizeof(command_copy));

    char *command1 = strtok(command, "|");
    char *command2 = strtok(NULL, "|");
    if (command2 != NULL)
    {
        execute_command_with_pipe(command1, command2);
    }
    else
    {
        char *args[256];
        char *token = strtok(command, " ");
        int i = 0;
        while (token != NULL)
        {
            args[i] = token;
            i++;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        bool run_in_background = false;
        pid_t pidG;
        if (i > 0 && strcmp(args[i - 1], "&") == 0)
        {
            run_in_background = true;
            args[i - 1] = NULL;
            pidG = getpid();
        }

        if (strcmp(args[0], "clr") == 0)
        {
            system("clear");
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            if (run_in_background)
            {
                jobs[job_id - 1].pid = pidG;
                jobs[job_id - 1].job_id = job_id;
                strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                printf("[%d] %d ", job_id, pidG);
                job_id++;
            }
            char *dir = args[1];
            if (dir == NULL)
            {
                printf("%s\n", cwd);
            }
            else if (strcmp(dir, "-") == 0)
            {
                if (oldpwd[0] != '\0')
                {
                    strcpy(dir, oldpwd);
                }
                else
                {
                    printf("myshell: cd: OLDPWD not set\n");
                    return;
                }
            }
            if (chdir(dir) == 0)
            {
                strcpy(oldpwd, cwd);
                if (getcwd(cwd, cwd_size) == NULL)
                {
                    perror("getcwd() error");
                    exit(1);
                }
                setenv("PWD", cwd, 1);
            }
            else
            {
                perror("myshell");
            }
        }
        else if (strcmp(args[0], "echo") == 0)
        {
            char *input_file = NULL;
            char *output_file = NULL;
            if (run_in_background)
            {
                jobs[job_id - 1].pid = pidG;
                jobs[job_id - 1].job_id = job_id;
                strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                printf("[%d] %d ", job_id, pidG);
                job_id++;
            }
            char *arg = args[1];
            int i = 1;
            while (args[i] != NULL)
            {
                if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL)
                {
                    input_file = args[i + 1];
                    break;
                }
                else if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL)
                {
                    output_file = args[i + 1];
                    break;
                }
                i++;
            }
            if (arg[0] == '$')
            {
                char *env_var = getenv(arg + 1);
                if (env_var != NULL)
                {
                    printf("%s\n", env_var);
                }
            }
            else
            {
                if (arg[0] != '<')
                {
                    printf("%s\n", arg);
                }
            }
            if (input_file != NULL)
            {
                char line[256];
                FILE *file = fopen(input_file, "r");
                if (file == NULL)
                {
                    perror("fopen");
                    return;
                }
                while (fgets(line, sizeof(line), file) != NULL)
                {
                    printf("%s\n", line);
                }
                fclose(file);
            }
            if (output_file != NULL)
            {
                char line[256];
                FILE *file = fopen(output_file, "w");
                if (file == NULL)
                {
                    perror("fopen");
                    return;
                }
                fprintf(file, "%s", arg);
                fclose(file);
            }
        }
        else if (strcmp(args[0], "quit") == 0)
        {
            exit(0);
        }
        else if (strcmp(args[0], "myshell") == 0)
        {
            char *filename = args[1];
            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("myshell");
                return;
            }

            char batchCommand[256];
            while (fgets(batchCommand, sizeof(batchCommand), file) != NULL)
            {
                batchCommand[strcspn(batchCommand, "\n")] = 0;
                execute_command(batchCommand, cwd, cwd_size, oldpwd);
            }

            fclose(file);
        }
        else
        {
            char *input_file = NULL;
            char *output_file = NULL;
            int i = 1;
            while (args[i] != NULL)
            {
                if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL)
                {
                    input_file = args[i + 1];

                    break;
                }
                else if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL)
                {
                    output_file = args[i + 1];
                    break;
                }
                i++;
            }
            pid_t pid = fork();
            if (pid < 0)
            {
                perror("fork failed");
                exit(1);
            }
            else if (pid == 0)
            {
                if (input_file != NULL)
                {
                    int in = open(input_file, O_RDONLY);
                    if (in == -1)
                    {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(in, STDIN_FILENO) == -1)
                    {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(in);
                    char *new_args[256];
                    int j = 0;
                    for (int k = 0; args[k] != NULL; k++)
                    {
                        if (strcmp(args[k], "<") != 0 && (k == 0 || strcmp(args[k - 1], "<") != 0))
                        {
                            new_args[j] = args[k];
                            j++;
                        }
                    }
                    new_args[j] = NULL;
                    execvp(args[0], new_args);
                    printf("\n");
                    perror("execvp");
                    close(in);
                    exit(EXIT_FAILURE);
                }
                if (output_file != NULL)
                {
                    int in = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (in == -1)
                    {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(in, STDIN_FILENO) == -1)
                    {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(in);
                    char *new_args[256];
                    int j = 0;
                    for (int k = 0; args[k] != NULL; k++)
                    {
                        if (strcmp(args[k], ">") != 0 && (k == 0 || strcmp(args[k - 1], ">") != 0))
                        {
                            new_args[j] = args[k];
                            j++;
                        }
                    }
                    new_args[j] = NULL;
                    execvp(args[0], new_args);
                    printf("\n");
                    perror("execvp");
                    close(in);
                    exit(EXIT_FAILURE);
                }
                else
                {
                    if (execvp(args[0], args) == -1)
                    {
                        perror("myshell");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            else
            {
                if (run_in_background)
                {
                    jobs[job_id - 1].pid = pidG;
                    jobs[job_id - 1].job_id = job_id;
                    strncpy(jobs[job_id - 1].command, command, sizeof(jobs[job_id - 1].command));
                    printf("[%d] %d ", job_id, pidG);
                    job_id++;
                }
                else
                {
                    foreground_job_pid = pid;
                    waitpid(pid, NULL, 0);
                    foreground_job_pid = 0;
                }
            }
        }
    }
}

void execute_command_with_pipe(char *command1, char *command2)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
        return;
    }
    struct passwd *pw;
    pw = getpwuid(geteuid());
    char command[1024];
    char oldpwd[1024] = "";
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }

    pid1 = fork();
    if (pid1 == -1)
    {
        perror("fork");
        return;
    }
    else if (pid1 == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execute_command(command1, cwd, sizeof(cwd), oldpwd);
        exit(EXIT_SUCCESS);
    }
    waitpid(pid1, NULL, 0);
    pid2 = fork();
    if (pid2 == -1)
    {
        perror("fork");
        return;
    }
    if (pid2 == 0)
    {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execute_command(command2, cwd, sizeof(cwd), oldpwd);
        exit(EXIT_SUCCESS);
    }
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void handle_sigint(int sig)
{
    printf("\033[33mSIGINT received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        printf("foreground_job_pid: %d\n", foreground_job_pid);
        kill(foreground_job_pid, SIGINT);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigtstp(int sig)
{
    printf("\033[33mSIGSTP received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        kill(foreground_job_pid, SIGTSTP);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigquit(int sig)
{
    printf("\033[33mSIGQUIT received\033[0m\n");
    if (foreground_job_pid != 0)
    {
        kill(foreground_job_pid, SIGQUIT);
    }
    siglongjmp(jmpbuf, 1);
}

void handle_sigchld(int sig)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
}
```

Para poder agregar las funcionalidades solicitadas al programa, se realiza un parseo del argumento recibido en caso de contar con los separadores `<` o `>`, permitiendo que el comando interno `echo` trabaje con los nuevos argumentos, asi como tambien lo hacen las funciones que son externas y son llamadas con `execvp()`. 
#### Ejemplo de ejecucion
Para corroborar el correcto funcionamiento del programa, se usan los comandos `cat` y `echo`. Obteniendose efectivamente lo esperado.
```
all5one@all5one-Ubuntu:~/Documents/soi-2023-lab2-myshell-all5one-sudo$ ./myshell8
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ echo Bye! > outputTest.txt
Bye!
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ cat < redirectionTest.txt
Hi!
all5one@all5one-Ubuntu:/home/all5one/Documents/soi-2023-lab2-myshell-all5one-sudo$ 
```

### Makefile
Para compilar el programa con los flags solicitados, se creo el siguiente Makefile:
```makefile
CC = gcc
CFLAGS = -Wall -pedantic -Werror

myshell: myshell.c
	$(CC) $(CFLAGS) -o myshell myshell.c

clean:
	rm -f myshell
```
