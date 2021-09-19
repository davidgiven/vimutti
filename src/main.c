#include "globals.h"

static void cmd_help(void)
{
	fatal("sorry, no help yet!");
}

int main(int argc, char* const argv[])
{
	const char* cmd = argv[1];
	if (!cmd)
		fatal("nothing to do! Try 'help' for a usage summary.");

	argv++;
	argc--;

	if (strcmp(cmd, "help") == 0)
		cmd_help();
	else if (strcmp(cmd, "unpackapp") == 0)
		cmd_unpackapp(argc, argv);
	else if (strcmp(cmd, "unpackimg") == 0)
		cmd_unpackimg(argc, argv);
	else
		fatal("unknown command! Try 'help' for a usage summary.");

	return 0;
}

