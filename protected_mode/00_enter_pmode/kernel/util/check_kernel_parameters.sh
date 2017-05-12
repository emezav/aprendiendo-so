#!/bin/bash
#Author: Erwin Meza <emezav@gmail.com>

if [ ! -f $1 ]; then
 echo "You must specify a file"
 exit 1
fi

kernel_size=$(wc -c $1 | cut -f1 -d' ')
#echo "Kernel size: $kernel_size"

#Tamaño máximo del kernel: 0x9F000 - 0x1000 = 0x97200 = 619008 = 604 KB.
max_kernel_size=619008

#Verificar si el kernel no sobrepasa el tamaño disponible en la memoria
if [ $kernel_size -gt $max_kernel_size ]; then
	echo "Kernel size is bigger than available space in real mode memory!"
	exit 1
fi
