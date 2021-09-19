#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <fnmatch.h>
#include "globals.h"

struct dirent
{
	uint32_t crc; /* CRC16, polynomial 0x1021, initial 0x0000 */
	uint32_t offset;
	uint32_t length;
	char filename[17];
};

static uint8_t* image;
static uint32_t image_len;
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

static void syntax(void)
{
	fatal("syntax error: vimutti <image filename> [<wildcard>]");
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

static void extract_file(const char* pattern)
{
	if (!image)
		fatal("no image specified");

	for (int i=0; i<directory_len; i++)
	{
		struct dirent* de = &directory[i];
		if (fnmatch(pattern, de->filename, 0))
			continue;

		printf("Extracting %s of %d bytes\n", de->filename, de->length);

		FILE* fp = fopen(de->filename, "wb");
		if (!fp)
			fatal("cannot open output file: %s", strerror(errno));

		uint8_t* p = image + de->offset;
		uint16_t crc = crc16(0x1021, 0x0000, p, de->length);
		for (int i=0; i<de->length; i++)
		{
			uint8_t b = *p++;
			putc(b, fp);
		}
		fclose(fp);

		if (crc != de->crc)
			fatal("CRC check failure during extraction --- image corrupt");
	}
}

void cmd_unpackimg(int argc, char* const* argv)
{
	if (!argv[0])
		syntax();

	for (;;)
	{
		int opt = getopt(argc, argv, "f:x:l");
		if (opt == -1)
			break;

		switch (opt)
		{
			case 'f':
				load_file(optarg, &image, &image_len);
				load_directory();
				break;

			case 'l':
				show_directory();
				break;

			case 'x':
				extract_file(optarg);
				break;

			case 'h':
			default:
				syntax();
		}
	}
}

