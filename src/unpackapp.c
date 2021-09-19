#include "globals.h"
#include <errno.h>

struct chunk
{
	uint16_t index;
	uint16_t length;
	uint16_t address;
	uint16_t offset;
	uint16_t dcrc;
	uint16_t tcrc;
};

static uint8_t* image;
static uint32_t image_len;
static struct chunk* directory;
static int directory_len;

static void syntax(void)
{
	fatal("syntax error: vimutti <image filename> [<wildcard>]");
}

static void decrypt(uint8_t* data, int datalen)
{
	const uint16_t lfsr_taps = 0xe03e;
	uint16_t lfsr_state = 0x0084;
	uint8_t xor = 0xef;

	while (datalen--)
	{
		*data++ ^= xor;

		uint32_t newbit = 0;
		uint32_t count = (lfsr_state & lfsr_taps) + 1;
		while (count)
		{
			count &= count - 1;
			newbit++;
		}

		newbit &= 1;
		lfsr_state >>= 1;
		lfsr_state |= newbit << 15;

		bool carry = (xor & 0x80) != 0;
		xor <<= 1;
		if (newbit ^ carry)
			xor ^= 0x21;
	}
}

static void load_directory(void)
{
	if (!image)
		fatal("no file loaded");

	decrypt(image, 2);
	directory_len = read16(image+0);
	decrypt(image, 2);

	directory = calloc(directory_len, sizeof(struct chunk));

	for (int i=0; i<directory_len; i++)
	{
		uint8_t* p = image + i*16;
		decrypt(p, 16);

		struct chunk* c = &directory[i];
		c->index = read16(p+0);
		c->length = read16(p+2);
		c->address = read16(p+6);
		c->offset = read16(p+10);
		c->dcrc = read16(p+12);
		c->tcrc = read16(p+14);
	}
}

static void show_directory(void)
{
	for (int i=0; i<directory_len; i++)
	{
		struct chunk* c = &directory[i];
		printf("index=%02x len=%04x address=%04x offset=%04x dcrc=%04x tcrc=%04x\n",
			c->index, c->length, c->address, c->offset, c->dcrc, c->tcrc);
	}
}

static void extract_file(int index)
{
	if (!image)
		fatal("no image specified");

	for (int i=0; i<directory_len; i++)
	{
		struct chunk* c = &directory[i];
		if (c->index != index)
			continue;

		char* filename;
		asprintf(&filename, "chunk-%02x.bin", c->index);

		printf("Extracting index 0x%x of %d bytes to %s\n", c->index, c->length, filename);

		FILE* fp = fopen(filename, "wb");
		if (!fp)
			fatal("cannot open output file: %s", strerror(errno));
		free(filename);

		uint8_t* p = image + c->offset;
		decrypt(p, c->length);
		uint16_t crc = crc16(0x1021, 0x0000, p, c->length);
		if (fwrite(p, c->length, 1, fp) != 1)
			fatal("couldn't write to output file");
		fclose(fp);

		if (crc != c->dcrc)
			fatal("CRC check failure during extraction --- wanted %04x, got %04x",
				c->dcrc, crc);
	}
}

void cmd_unpackapp(int argc, char* const* argv)
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
				extract_file(strtoul(optarg, NULL, 0));
				break;

			case 'h':
			default:
				syntax();
		}
	}
}

