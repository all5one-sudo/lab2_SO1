# MyShell
### SOI - FCEFyN - UNC - 2023


## Objetivo
Los objetivos del presente laboratorio son:
- Utilizar mecanismos de creación de Procesos.
- Utilizar mecanismos de concurrencia e IPC.
- Diseñar un intérprete de línea de comandos al estilo Bourne shell.
- Ampliar las características de un intérprete de línea de comandos al estilo Bourne shell.
- Comprender el uso de la API de señales del SO

## Duración
Con el material dado en clase, este laboratorio está diseñado para resolverse entre 16 y 20 horas.


## Introducción
Este trabajo práctico consta en la elaboración de un programa en lenguaje C sobre _GNU/Linux_. El trabajo se divide en soluciones incrementales.


## Actividades
#### 1. Command line prompt
_myshell_ debe contar con un prompt que contenga el camino al directorio actual.

Por ejemplo, para el home: _username@hostname:~$_


#### 2. Internal commands
_myshell_ debe soportar los siguientes comandos internos:

- __cd \> directorio \>__ : cambia el directorio actual a \<directorio\>. Si \<directorio\> no está presente, reporta el directorio actual. Si el directorio no existe se debe imprimir un error apropiado. Además, este comando debe cambiar la variable de entorno PWD. 
Este comando debe soportar la opción *cd -*, que retorna al último directorio de trabajo (__OLDPWD__).

- __clr__: limpia la pantalla

- __echo \<comentario\|env var\>__ : muestra \<comentario\> en la pantalla seguido por una línea nueva. (múltiples espacios\/tabs pueden ser reducidos a un espacio).

- __quit__ : cierra myshell

#### 3. Program invocation
Las entradas del usuario que no sean comandos internos deben ser interpretados como la invocación de un programa. La misma tiene que ser realizada mediante _fork_ y _execl_. Debe además soportar paths relativos y absolutos.


#### 4. Batch file
_myshell_ debe ser capaz de tomar sus comandos a ejecutar desde un archivo. Por ejemplo, si la shell es invocada con un argumento _myshell batchfile_. El _batchfile_ contiene un conjunto de comandos de línea para que la shell ejecute. 

Cuando se alcance el fin de archivo (EOF), _myshell_ debe cerrarse.

Notar que si myshell se ejecuta sin argumento (.\/myshell), se tiene que mostrar el command prompt y se debe esperar a comandos del usuario vía stdin.

#### 5. Background execution
Un ampersand & al final de la línea de comando indica que la shell debe retornar al prompt inmediatamente luego de lanzar al programa a ejecutarse en background.

Cuando se comienza un trabajo en background, se debe imprimir un mensaje indicando su _Trabajo_ y su _ID de proceso_.

`[<job id>] <process id>`

Ejemplo:
`$ echo 'hola' & [1] 10506 hola`

#### 6. Signal Handling
Si se ingresa alguna de las combinaciones CTRL-C, CTRL-Z o CTRL-/\, las señales resultantes (**SIGINT**, **SIGTSTP**, **SIGQUIT** respectivamente) deben ser enviadas al trabajo (job) en ejecución de primer plano en vez de a _myshell_. Si no hay un trabajo (job) en ejecución de primer plano, no debe suceder nada.

#### 7. Pipe
_myshell_ provee la funcionalidad de un pipe a través del operador **|** (pipe). El mismo conecta la salida estándar del proceso (stdout) lazando por el comando de la izquierda del pipe con la entrada estándar del proceso (stdin) que se genera con el comando a la derecha del pipe.

Ejemplos:
```
$ last <username> | wc -l
$ ps aux | grep firefox
$ grep bash /etc/passwd | cut -d “:” -f 1 | sort -r
```
##### Responder:
¿Dónde se encuentran los pipes en el filesystem, qué atributos tienen?


#### 8. I/O redirection 
Se debe soportar redirección de entrada/salida en _stdin_ y/o _stdout_. 

Por ejemplo:
```
program arg1 ar2 < inputfile > outputfile
```

Ejecuta la el programa _program_ con los arguments _arg1_, _arg2_. _stdin_ es reemplazado por _inputfile_ y _stdout_ por _outputfile_.
La redirección debe funcionar para el comando interno _echo_.


## Criterios de Corrección
- Se debe compilar el código con los flags de compilación: -Wall -Pedantic -Werror 
- Dividir el código en módulos de manera juiciosa.
- Estilo de código.
- El código no debe contener errores, ni warnings.
- El código no debe contener errores de cppcheck.
- _myshell_ no debe crear procesos zombies.

## Qué se debe Entregar
- Informe del desarrollo del proyecto (markdown).
- Código (funcionando bajo las especificaciones dadas y bajo cualquier caso de test de parámetros de entrada).
- Makefile
- Todo en el GitHub


### Links
- [1] https://www.toptal.com/git/git-workflows-for-pros-a-good-git-guide
- [2] https://www.dataschool.io/how-to-contribute-on-github/
