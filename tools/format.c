#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv)
{
	uint8_t b;
	int x;
	int i = 0;

	while ((x = getc(stdin)) != EOF) {
		b = (uint8_t)x;
		i++;
		printf("0x%02X,", b);
		if (i % 16 == 0)
			printf("\\\n");
	}

	return 0;
}
