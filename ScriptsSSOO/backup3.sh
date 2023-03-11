#!/bin/bash
for i in $*
do
    if [ ! -f "$1" ] # Chequea si el parámetro de entrada representa un fichero que existe
        then # En caso de no existir, da un mensaje comunicándolo
            echo $i no existe
        else # En caso de existir el fichero ...
            A=$(ls $i* | wc -w) # Lista los ficheros que comienzan por el nombre dado como
    # parámetro y cuenta cuantos hay y lo almacena en A
            if [ $A -ge 9 ] # Si hay 9 o más, da un mensaje indicando que se ha
    # alcanzado el máximo número de versiones
                then
                    echo “Se ha superado el número máximo de versiones”
                else # Si no se ha alcanzado el maximo ...
                    Num=`expr $A + 1` # Se suma 1 al número de ficheros que comienzan
    # con ese nombre (A) para que sea la terminación
    # de la nueva copia
                    fecha=`date +%y%m%d`
                    #Version=$i 
                    if [ ! -d "$fecha" ]
                    then
                        mkdir $fecha #creo el directorio con la fecha por nombre
                    fi
                    cp $i $fecha # Copia el fichero original ($i) con el nombre nuevo al directorio con la fecha por nombre
            fi
    fi
done