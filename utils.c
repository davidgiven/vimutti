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

