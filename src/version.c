/* (c) Vasian CEPA 2002, http://madebits.com */
#include <stdio.h>

int main(int argc, char** argv){
	FILE* f;
	double ver = -0.01;
	char * format = "ver = %1.2lf;";
	char * file = "version.txt";
	if(argc == 2) {
		 file = argv[1];
	}
	printf("Updating version file [%s]\n", file);
	f = fopen(file, "r");
	if(f != NULL){
		fscanf(f, "ver = %lf", &ver);
		fclose(f);
	}
	ver += 0.01;
	printf(format, ver);
	printf("\n");
	f = fopen(file, "w");
	fprintf(f, format, ver);
	fclose(f);
}