#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include "fnmatch.h"

struct dirent
{
	uint32_t crc; /* CRC16, polynomial 0x1021, initial 0x0000 */
	uint32_t offset;
	uint32_t length;
	char filename[17];
};

static uint8_t* image;
static long image_len;
static struct dirent* directory;
static int directory_len;

static uint32_t file_key_m;
static int file_key_n;
static uint8_t file_key_p;
static uint8_t file_key_q;

static uint32_t read32(uint8_t* ptr)
{
	return 
		(ptr[0] << 24)
		| (ptr[1] << 16)
		| (ptr[2] << 8)
		| ptr[3];
}

static void error(const char* s, ...)
{
	va_list ap;
	va_start(ap, s);

	fprintf(stderr, "Error: ");
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");

	va_end(ap);
	exit(1);
}

static void syntax(void)
{
	error("syntax error: vimutti <image filename> [<wildcard>]");
}

static uint8_t* load_image(const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (!fp)
		error("cannot open image file: %s", strerror(errno));

	fseek(fp, 0, SEEK_END);
	image_len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	image = malloc(image_len);
	if (fread(image, image_len, 1, fp) != 1)
		error("cannot read image file: %s", strerror(errno));
	fclose(fp);

	return image;
}

static void make_directory_key(uint8_t key[32])
{
	uint8_t xor;
	uint32_t mask = 0;

	for (int i=0; i<32; i++)
	{
		if (mask == 0)
		{
			mask = 0xac0c8880;
			xor = 0xef;
		}

		key[i] = xor;

		bool carry = xor & 0x80;
		xor <<= 1;

		if ((mask & 1) ^ carry)
			xor ^= 0x21;
		mask >>= 1;
	}
}

static void decrypt(uint8_t* key, int keylen, uint8_t* data, int datalen)
{
	for (int i=0; i<datalen; i++)
		data[i] ^= key[i % keylen];
}

static void load_directory(void)
{
	uint8_t key[32];
	make_directory_key(key);

	uint8_t buffer[32];
	memcpy(buffer, image, 32);
	decrypt(key, sizeof(key), buffer, 32);

	directory_len = read32(buffer+12);
	directory = calloc(directory_len, sizeof(struct dirent));

	for (int i=0; i<directory_len; i++)
	{
		uint8_t buffer[32];
		memcpy(buffer, image + (i+1)*32, 32);
		decrypt(key, sizeof(key), buffer, 32);

		struct dirent* de = &directory[i];
		de->crc = read32(buffer+0);
		de->offset = read32(buffer+4);
		de->length = read32(buffer+8);
		memcpy(de->filename, buffer+16, 16);

		char* p = de->filename;
		for (int i=0; i<16; i++)
		{
			if (*p == 0xff)
				break;
			p++;
		}
		*p = 0;
	}
}

static void show_directory(void)
{
	for (int i=0; i<directory_len; i++)
	{
		struct dirent* de = &directory[i];
		printf("crc=%08x len=%08x data=%08x until %08x: %s\n",
			de->crc, de->length, de->offset, de->offset+de->length, de->filename);
	}
}

static void make_file_key(uint8_t* key)
{
	uint8_t xor;
	uint16_t mask = 0;

	for (int i=0; i<file_key_n; i++)
	{
		if (mask == 0)
		{
			mask = file_key_m;
			xor = file_key_p;
		}

		key[i] = xor;

		bool carry = xor & 0x80;
		xor <<= 1;

		if ((mask & 1) ^ carry)
			xor ^= file_key_q;
		mask >>= 1;
	}
}


static void extract_file(const char* pattern)
{
	uint8_t key[file_key_n];
	make_file_key(key);

	for (int i=0; i<directory_len; i++)
	{
		struct dirent* de = &directory[i];
		if (fnmatch(pattern, de->filename, 0))
			continue;

		printf("Extracting %s of %d bytes\n", de->filename, de->length);

		FILE* fp = fopen(de->filename, "wb");
		if (!fp)
			error("cannot open output file: %s", strerror(errno));

		uint8_t* p = image + de->offset;
		for (int i=0; i<de->length; i++)
		{
			uint8_t b = *p++;
			if (file_key_n)
				b ^= key[i % file_key_n];
			putc(b, fp);
		}
		fclose(fp);
	}
}

int main(int argc, char* const argv[])
{
	for (;;)
	{
		int opt = getopt(argc, argv, "f:p:q:n:m:x:lh");
		if (opt == -1)
			break;

		switch (opt)
		{
			case 'f':
				load_image(optarg);
				load_directory();
				break;

			case 'p':
				file_key_p = strtoul(optarg, NULL, 0);
				break;

			case 'q':
				file_key_q = strtoul(optarg, NULL, 0);
				break;

			case 'n':
				file_key_n = strtoul(optarg, NULL, 0);
				break;

			case 'm':
				file_key_m = strtoul(optarg, NULL, 0);
				break;

			case 'l':
				show_directory();
				break;

			case 'x':
				for (int i=2; i<argc; i++)
					extract_file(argv[i]);
				break;

			case 'h':
			default:
				syntax();
		}
	}
}

