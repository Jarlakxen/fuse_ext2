#!/bin/bash


## Funciones Generales

function create_and_fill_random_file {
   dd if=/dev/urandom of=$1 bs=$2 count=1
}

function create_and_fill_zero_file {
   dd if=/dev/zero of=$1 bs=$2 count=1
}

## Main

PATH_MOUNT=$1

echo '[+] Making Directory: /Carpeta1'
echo '[+] Making Directory: /Carpeta1/Subdir11'
echo '[+] Making Directory: /Carpeta1/Subdir11/Subdir111'
mkdir -p $PATH_MOUNT/Carpeta1/Subdir11/Subdir111
echo '[+] Making Directory: /Carpeta1/Subdir11/Subdir112'
mkdir -p $PATH_MOUNT/Carpeta1/Subdir11/Subdir112
echo '[+] Making Directory: /Carpeta1/Subdir11/Subdir113'
mkdir -p $PATH_MOUNT/Carpeta1/Subdir11/Subdir113

echo '[+] Making Directory: /Carpeta2'
echo '[+] Making Directory: /Carpeta2/Subdir21'
mkdir -p $PATH_MOUNT/Carpeta2/Subdir21
echo '[+] Making Directory: /Carpeta2/Subdir22'
mkdir -p $PATH_MOUNT/Carpeta2/Subdir22
echo '[+] Making Directory: /Carpeta2/Subdir23'
mkdir -p $PATH_MOUNT/Carpeta2/Subdir23

echo '[+] Making File: /empty.txt'
touch $PATH_MOUNT/empty.txt

echo '[+] Making File: /file1.txt'
echo 'file1.txt tiene LFN' > $PATH_MOUNT/file1.txt
echo '[+] Making File: /FILE2.TXT'
echo 'FILE2.TXT no tiene LFN' > $PATH_MOUNT/FILE2.TXT

echo '[+] Making File: /file1_1c.txt'
create_and_fill_random_file "$PATH_MOUNT/file1_1c.txt" 128

echo '[+] Making File: /file2_1c.txt'
create_and_fill_random_file "$PATH_MOUNT/file2_1c.txt" 4096

echo '[+] Making File: /file3_2c.txt'
create_and_fill_random_file "$PATH_MOUNT/file3_2c.txt" 8192

echo '[+] Making File: /file4_5c.txt'
create_and_fill_random_file "$PATH_MOUNT/file4_5c.txt" 16680

echo '[+] Making File: /f5_25601c.txt'
create_and_fill_zero_file "$PATH_MOUNT/f5_25601c.txt" 104857608

