#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "lexer.h"
#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"
#include "error.h"

#define READSIZE 512

static char *program_name;

static char *shortopt = "hv";
static struct option longopt[] = {
	{ "help", no_argument, NULL, (int)'h' },
	{ "version", no_argument, NULL, (int)'v' },
	{ 0, 0, 0, 0 }
};

static void help(void) {
	fprintf(stderr, "\
Usage: %s [FILE] ... \n\
Interpret FILE(s) as shibe. Let FILE be '-' for stdin.\n\
  -h, --help\t\toutput this help\n\
  -v, --version\t\tprogram version\n", program_name);
}

static void version (char *revision) {
	fprintf(stderr, "%s %s\n", program_name, revision);
}

int main(int argc, char **argv)
{
	unsigned int size = 0;
	unsigned int length = 0;
	char *buffer = NULL;
	LexemeList *lexemes = NULL;
	Token **tokens = NULL;
	MainNode *node = NULL;
	char *fname = NULL;
	FILE *file = NULL;
	int ch;

	char *revision = "v0.0.1";
	program_name = argv[0];

	while ((ch = getopt_long(argc, argv, shortopt, longopt, NULL)) != -1) {
		switch (ch) {
			default:
				help();
				exit(EXIT_FAILURE);
			case 'h':
				help();
				exit(EXIT_SUCCESS);
			case 'v':
				version(revision);
				exit(EXIT_SUCCESS);
		}
	}

	for (; optind < argc; optind++) {
		size = length = 0;
		buffer = fname = NULL;
		lexemes = NULL;
		tokens = NULL;
		node = NULL;
		file = NULL;

		if (!strncmp(argv[optind],"-\0",2)) {
			file = stdin;
			fname = "stdin";
		}
		else {
			file = fopen(argv[optind], "r");
			fname = argv[optind];
		}

		if (!file) {
			error(MN_ERROR_OPENING_FILE, argv[optind]);
			return 1;
		}

		while (!feof(file)) {
			size += READSIZE;
			buffer = realloc(buffer, sizeof(char) * size);
			length += fread((buffer + size) - READSIZE,
					1,
					READSIZE,
					file);
		}

		if (fclose(file) != 0) {
			error(MN_ERROR_CLOSING_FILE, argv[optind]);
			if (buffer) free(buffer);
			return 1;
		}
		if (!buffer) return 1;
		buffer[length] = '\0';

		/* Remove hash bang line if run as a standalone script */
		if (buffer[0] == '#' && buffer[1] == '!') {
			unsigned int n;
			for (n = 0; buffer[n] != '\n' && buffer[n] != '\r'; n++)
				buffer[n] = ' ';
		}

		/*
		 * Remove UTF-8 BOM if present and add it to the output stream
		 * (we assume here that if a BOM is present, the system will
		 * also expect the output to include a BOM).
		 */
		if (buffer[0] == (char)0xef
				|| buffer[1] == (char)0xbb
				|| buffer[2] == (char)0xbf) {
			buffer[0] = ' ';
			buffer[1] = ' ';
			buffer[2] = ' ';
			printf("%c%c%c", 0xef, 0xbb, 0xbf);
		}

		/* Begin main pipeline */
		if (!(lexemes = scanBuffer(buffer, length, fname))) {
			free(buffer);
			return 1;
		}
		free(buffer);
		if (!(tokens = tokenizeLexemes(lexemes))) {
			deleteLexemeList(lexemes);
			return 1;
		}
		deleteLexemeList(lexemes);
		if (!(node = parseMainNode(tokens))) {
			deleteTokens(tokens);
			return 1;
		}
		deleteTokens(tokens);
		if (interpretMainNode(node)) {
			deleteMainNode(node);
			return 1;
		}
		deleteMainNode(node);
		/* End main pipeline */

	}

	return 0;
}
