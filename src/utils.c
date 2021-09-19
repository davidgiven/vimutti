#include "globals.h"
#include <sys/mman.h>
#include <errno.h>

void fatal(const char* message, ...)
{
	va_list ap;
	va_start(ap, message);

	fprintf(stderr, "Error: ");
	vfprintf(stderr, message, ap);
	fprintf(stderr, "\n");

	va_end(ap);
	exit(1);
}

uint16_t read16(uint8_t* ptr)
{
	return 
		(ptr[0] << 8)
		| ptr[1];
}

uint32_t read32(uint8_t* ptr)
{
	return 
		(ptr[0] << 24)
		| (ptr[1] << 16)
		| (ptr[2] << 8)
		| ptr[3];
}

void load_file(const char* filename, uint8_t** data, uint32_t* length)
{
	FILE* fp = fopen(filename, "rb");
	if (!fp)
		fatal("cannot open file '%s': %s", filename, strerror(errno));

	fseek(fp, 0, SEEK_END);
	*length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	*data = malloc(*length);
	if (fread(*data, *length, 1, fp) != 1)
		fatal("could not read file: %s", strerror(errno));

	fclose(fp);
}

uint32_t crc16(uint16_t poly, uint16_t crc, uint8_t* data, uint32_t length)
{
	while (length--)
	{
		crc ^= (*data++) << 8;
		for (int i=0; i<8; i++)
			crc = (crc & 0x8000) ? ((crc<<1)^poly) : (crc<<1);
	}

	return crc;
}

