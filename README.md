# Distirbuidos-final
Este ejercicio implementa una aplicacion distrbuída para compartir ficheros entre clientes en un sistema p2p, coordinado a través de un servidor central.

Para información más detallada consulte la [memoria](https://github.com/CLopMan/University_Projects/blob/main/distribuidos_final/doc/memoria.pdf) del proyecto.

## Autores
- [100472182](https://github.com/Adri-Extremix)
- [CLopMan](https://github.com/CLopMan)

Curso: 2023-24

# Compilación

Este entregable tiene una estructura de directorios con el fin de organizar mejor el código según la funcionalidad que este implemente. Para el correcto funcionamiento del Makefile es importante respetar dicha estructura. 

Puede compilar el proyecto a través del comando `make` desde la raíz del proyecto. 

# Ejecución 
Para la ejecución del proyecto, siga las instrucciones incluídas en la sección 6 de la memoria. Como breve resumen, se adjunta una plantilla para la ejecución: 

```
# suponiendo que este en la raiz del proyecto
    
# servidores
env RPC_IP=localhost ./servidor -p 4500 &
./servidor_rpc & 
python src/servicio_web/timestamp.py &

# cliente
python src/cliente/client.py -s localhost -p 4500
```
En caso de querer ver la salida más claramente, se recomienda ejecutar cada uno de los comandos en una terminal distinta. 

# Tests
Como se especifica en la memoria, la mayoría de pruebas realizadas sobre la práctica han sido manuales. No obstante, se adjuntan en el directorio `tests` un archivo *test_usersLists.c* que incluye una serie de pruebas sobre la estructura de datos; un archivo *setup_clean.py* capaz de generar las entradas y ficheros de prueba utilizados en los test; y un archivo en `bash` *test_concurrencia.sh* para la ejecución de una prueba concurrente. 
