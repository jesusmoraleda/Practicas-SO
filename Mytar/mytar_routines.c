//Jesus Martin Moraleda 06277633J
//Jorge Arevalo Echevarria 51000180A
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	int i = 0;
	char* ptr = malloc(1); // reservamos 1 espacio de memoria por ser un char
	int rt = 0; // utilizamos rt para comprobar las funciones fread y fwrite

	// recorre el archivo de origen para copiarlo en el destino char a char
	for(i=0; i < nBytes && feof(origin) == 0; i++){
		rt = fread(ptr,1,1,origin);

		if(feof(origin)==0){
			if(rt != 1)
				return (-1);

			rt = fwrite(ptr,1,1,destination);

			if(rt != 1)
				return (-1);
		}

		else break;
	}

	free(ptr);

	return i;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file)
{

	int rt=0;
	bool found = false;
	int nameSize = 0;
	char* name = malloc(1);

	// mientras no se haya acabado la cadena(nombre del archivo),
	// aumenta el nameSize
	while(!found){
		rt = fread(name,1,1,file);

		if(rt !=1)
			return (NULL);

		if(*name=='\0')
			found = true;

		nameSize++;
	}

	//fseek mueve el puntero a la posicion del principio del nombre del file
	//posicionActual = seek_cur + (-nameSize)
	rt = fseek(file, -nameSize,SEEK_CUR);

	if(rt !=0) // si falla fseek, devuelve null
		return (NULL);

	free(name);

	name = malloc(nameSize);

	// carga el archivo
	rt = fread(name,nameSize,1,file);

	if(rt !=1)
		return (NULL);

	return name;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{

	stHeaderEntry* array = NULL;
	int i =0;
	char* pName = NULL;

	fread(nFiles, sizeof(int), 1, tarFile);

	// reserva memoria para el nombre y el tamaño de todos los archivos que haya
	array = malloc(sizeof(stHeaderEntry)*(*nFiles));

	//Se lee la información y la almacena
	for(i = 0; i < *nFiles; i++){
		pName = loadstr(tarFile);
		array[i].name = pName; // almacena el nombre

		fread(&array[i].size, sizeof(int),1,tarFile); // almacena el tamaño
	}

	return array;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	int i, nameSize = 0, rt = 0, size;
	char* ptr = malloc(1);
	FILE *out, *input;
	stHeaderEntry* arrayCabecera;

	arrayCabecera = malloc(sizeof(stHeaderEntry)* nFiles);

	for(i=0;i<nFiles;i++){
		nameSize += strlen(fileNames[i])+1; // tamaño del nombre del archivo
		arrayCabecera[i].name = fileNames[i]; // va metiendo el nombre del archivo
	}

	nameSize += (nFiles+1)*4;// lo x4 porque es un entero

	out = fopen(tarName,"w"); // el w para crear el fichero
	if(out == NULL)
		return (EXIT_FAILURE);

	rt = fwrite(ptr,nameSize,1,out);

	// en este bucle copia el contenido del archivo de los ficheros
	for(i = 0; i < nFiles; i++){
		input = fopen(fileNames[i],"r"); // r: abre el fichero en lectura
		if(input == NULL)
			return (EXIT_FAILURE);

		size = copynFile(input,out,INT_MAX);

		if(size != -1)
			arrayCabecera[i].size = size;
		else
			return (EXIT_FAILURE);

		if(fclose(input)!=0)
			return (EXIT_FAILURE);
	}

	rt = fseek(out,0,SEEK_SET);
	if(rt != 0)
		return (EXIT_FAILURE);

	rt = fwrite(&nFiles,4,1,out);

	// en este bucle copia el nombre y el tamaño del archivo de los ficheros
	for(i = 0; i < nFiles; i++){
		rt = fwrite(arrayCabecera[i].name,strlen(arrayCabecera[i].name)+1,1,out);
		if(rt == -1)
			return (EXIT_FAILURE);
		rt = fwrite(&arrayCabecera[i].size,4,1,out);
		if(rt == -1)
			return (EXIT_FAILURE);
	}

	free(arrayCabecera);
	free(ptr);

	if(fclose(out) != 0)
		return (EXIT_FAILURE);

	fprintf(stdout, "mtar file created successfully\n");

	return (EXIT_SUCCESS);
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	int i, n;
	int* nr_files = malloc(sizeof(int));
	FILE* out, *tar = fopen(tarName,"r");
	stHeaderEntry* arrayCabecera=NULL;

	if(tar==NULL)
		return (EXIT_FAILURE);

	arrayCabecera = readHeader(tar,nr_files);
	
	for(i = 0; i < *nr_files; i++){
		out = fopen(arrayCabecera[i].name, "w");

		if(out==NULL)
			return (EXIT_FAILURE);

		//Escribimos el archivo
		n = copynFile(tar,out,arrayCabecera[i].size);

		if(n!= arrayCabecera[i].size)
			return (EXIT_FAILURE);

		if(fclose(out)!=0)
			return (EXIT_FAILURE);
	}

	free(nr_files);
	free(arrayCabecera);

	if(fclose(tar)!=0)
		return (EXIT_FAILURE);

	fprintf(stdout, "mtar file uncompressed successfully\n");


	return (EXIT_SUCCESS);
}
/** Extract info files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE.
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the
 * tarball's data section. By using information from the
 * header --number of files and (file name, file size) pairs--, extract files
 * stored in the data section of the tarball.
 *
 */
int
infoTar(char tarName[])
{
	int i;
	stHeaderEntry* arrayCabecera=NULL;
	int *nFiles;
	nFiles= malloc(sizeof(int));

	FILE *file = fopen(tarName, "r");
	arrayCabecera = readHeader(file,nFiles);

	for (i = 0; i < (*nFiles); i++){
			printf ("[%d] File name: %s, size: %d bytes \n", i, arrayCabecera[i].name, arrayCabecera[i].size);
	}

		return (EXIT_SUCCESS);
}

/** Append a tarball archive
 *
 * nfiles: number of mtar files
 * filenames: array with the path names of the mtar files
 * tarname: name of the tarball archive
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE.
 * (macros defined in stdlib.h).
 *
 */
int
appendTar(int nFiles, char *fileNames[], char tarName[])
{
	int i, nameSize = 0, rt, size;
	char* ptr = malloc(1);
	stHeaderEntry* arrayCabecera=NULL;
	int *n_files = malloc(sizeof(nFiles));

	FILE *out, *input;
	out = fopen(tarName,"r+"); // el r+ para modificar
	arrayCabecera = readHeader(out, n_files);

	for(i=0;i< nFiles;i++){
			nameSize += strlen(fileNames[i])+1; // tamaño del nombre del archivo
			arrayCabecera[i].name = fileNames[i]; // va metiendo el nombre del archivo
	}

	nameSize += (nFiles+1)*4;// lo x4 porque es un entero

		if(out == NULL)
		return (EXIT_FAILURE);

	rt = fwrite(ptr,nameSize,1,out);

	// en este bucle copia el contenido del archivo de los ficheros
	for(i = 0; i < nFiles; i++){
		input = fopen(fileNames[i],"r"); // r: abre el fichero en lectura
		if(input == NULL)
			return (EXIT_FAILURE);

		size = copynFile(input,out,INT_MAX);

		if(size != -1)
			arrayCabecera[i].size = size;
		else
			return (EXIT_FAILURE);

		if(fclose(input)!=0)
			return (EXIT_FAILURE);
	}

	rt = fseek(out,0,SEEK_SET);
	if(rt != 0)
		return (EXIT_FAILURE);

	rt = fwrite(&nFiles,4,1,out);

	// en este bucle copia el nombre y el tamaño del archivo de los ficheros
	for(i = 0; i < nFiles; i++){
		rt = fwrite(arrayCabecera[i].name,strlen(arrayCabecera[i].name)+1,1,out);
		if(rt == -1)
			return (EXIT_FAILURE);
		rt = fwrite(&arrayCabecera[i].size,4,1,out);
		if(rt == -1)
			return (EXIT_FAILURE);
	}

	free(arrayCabecera);
	free(ptr);

	if(fclose(out) != 0)
		return (EXIT_FAILURE);

	fprintf(stdout, "mtar file modified successfully\n");

	return (EXIT_SUCCESS);
}
