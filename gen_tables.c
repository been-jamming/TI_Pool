#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define NUM_ANGLES 256
#define PI 3.1415926535897932

int main(int argc, char **argv){
	int i;
	double ang;

	printf("uint16_t sin_table[%d] = {\n", NUM_ANGLES);
	for(i = 0; i < NUM_ANGLES; i++){
		ang = PI*i/(2*NUM_ANGLES);
		printf("\t0x%x,\n", (uint16_t) (int) (256*sin(ang)));
	}
	printf("};\n");

	return 0;
}

