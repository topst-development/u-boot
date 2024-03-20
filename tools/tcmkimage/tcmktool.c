// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Telechips Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <../../lib/sha256.c>

#define ALIGN_SIZE 64U
#define IMAGE_NAME_MAX_SIZE 12
#define IMAGE_VERSION_MAX_SIZE 16
#define BODY_OFFSET 4096

/* List of image name */
const static char *image_name[] = {
	"A72-BL3",
	"A53-BL3",
	NULL,
};

/* Image Certificate that is for SignTool */
struct image_cert {
	uint8_t marker[4];
	uint8_t reserved[252];
};

/* Image Header */
struct image_header {
	uint8_t marker[4];
	uint32_t bodylength;
	uint32_t bodyoffset;
	uint8_t reserved1[4];
	uint8_t socname[4];
	/* imagename is filled by tcmktool and will be checked by Sign Tool */
	uint8_t imagename[IMAGE_NAME_MAX_SIZE];
	uint8_t imageversion[IMAGE_VERSION_MAX_SIZE];
	uint64_t targetaddress;
	uint32_t execstate;
	uint8_t reserved2[36];
	uint8_t bodyhash[32];
	uint8_t reserved3[128]; /* This is for Sign Tool */
};

/* Image Footer that is for Sign Tool */
struct image_footer {
	uint8_t reserved[128];
};

/**
 * @brief Fill a dummy certificate that will be changed by Sign Tool.
 *
 * @param outputfile File pointer of output file.
 * @return Error status.
 */
static int fill_dummy_cert(FILE *outputfile)
{
	const struct image_cert dummy_cert = {
		.marker = { 'C', 'E', 'R', 'T' },
		.reserved = { 0, },
	};
	if (fwrite(
		&dummy_cert, sizeof(struct image_cert), 1, outputfile) == 1) {
		return 0;
	}
	return -1;
}

/**
 * @brief Calculate hash of image.
 *
 * @param hash Pointer to obtain hash of image.
 * @param inputfile File pointer of input file.
 * @return Error status.
 */
static int calc_hash(uint8_t *hash, FILE *inputfile)
{
	uint8_t buf[ALIGN_SIZE];
	size_t len;
	sha256_context ctx;

	fseek(inputfile, 0, SEEK_SET);

	sha256_starts(&ctx);
	while (1) {
		memset(buf, 0, ALIGN_SIZE);
		if (fread(buf, 1, ALIGN_SIZE, inputfile) <= 0) {
			break;
		}
		sha256_update(&ctx, buf, ALIGN_SIZE);
	}
	sha256_finish(&ctx, hash);

	return 0;
}

/**
 * @brief Fill a header
 *
 * @param outputfile File pointer of output file.
 * @param inputfile File pointer of input file.
 * @param imagename String of image name.
 * @param imageversion String of image version.
 * @return Error status.
 */
static int fill_header(
	FILE *outputfile, FILE *inputfile, const char *imagename,
	const char *imageversion, const char *targetaddress,
	const char *execstate)
{
	struct image_header header;
	uint32_t length;
	uint8_t *buf;
	uint32_t offset;
	int32_t i;

	fseek(inputfile, 0, SEEK_END);
	length = ftell(inputfile);

	memset(&header, 0, sizeof(struct image_header));

	memcpy(header.marker, "HDR\0", 4);
	memcpy(header.socname, SOC_NAME, 4);
	if (strlen(imagename) > IMAGE_NAME_MAX_SIZE) {
		return -101;
	}

	i = 0;
	while (1) {
		if (image_name[i] == NULL) {
			return -106;
		}
		if (memcmp(image_name[i], imagename, strlen(imagename)) == 0) {
			break;
		}
		i++;
	}

	memcpy(header.imagename, imagename, strlen(imagename));
	if (strlen(imageversion) > IMAGE_VERSION_MAX_SIZE) {
		return -102;
	}
	memcpy(header.imageversion, imageversion, strlen(imageversion));

	header.bodylength =
		((length + (ALIGN_SIZE - 1)) & ~(ALIGN_SIZE - 1)) +
		sizeof(struct image_footer);
	header.bodyoffset = BODY_OFFSET;
	header.targetaddress = strtol(targetaddress, NULL, 16);
	header.execstate = strtol(execstate, NULL, 16);
	calc_hash(header.bodyhash, inputfile);

	if (fwrite(&header, sizeof(struct image_header), 1, outputfile) != 1) {
		return -103;
	}

	offset = ftell(outputfile);
	if (header.bodyoffset > offset) {
		buf = malloc(header.bodyoffset - offset);
		if (buf == NULL) {
			return -104;
		}
		memset(buf, 0, ALIGN_SIZE);
		if (fwrite(
			buf, header.bodyoffset - offset, 1, outputfile) != 1) {
			free(buf);
			return -105;
		}
		free(buf);
	}
	return 0;
}

/**
 * @brief Fill an image.
 *
 * @param outputfile File pointer of output file
 * @param inputfile File pointer of input file
 * @return Error status
 */
static int fill_image(FILE *outputfile, FILE *inputfile)
{
	char buf[ALIGN_SIZE];

	fseek(inputfile, 0, SEEK_SET);

	while (1) {
		memset(buf, 0, ALIGN_SIZE);
		if (fread(buf, 1, ALIGN_SIZE, inputfile) <= 0) {
			break;
		}
		fwrite(buf, 1, ALIGN_SIZE, outputfile);
	}
	return 0;
}

/**
 * @brief Fill a dummy footer that will be changed by Sign Tool.
 *
 * @param outputfile File pointer of output file
 * @return Error status
 */
static int fill_dummy_footer(FILE *outputfile)
{
	const struct image_footer dummy_footer = {
		.reserved = { 0, },
	};
	if (fwrite(&dummy_footer, sizeof(struct image_footer), 1,
		   outputfile) == 1) {
		return 0;
	}
	return -1;
}

/**
 * @brief Make an image.
 *
 * @param inputfile File pointer of input file.
 * @param outputfile File pointer of output file.
 * @param argc Argument Count
 * @param argv Argument Value
 * @return Error status
 */
static int make_image(FILE *inputfile, FILE *outputfile, char *argv[])
{
	if (fill_dummy_cert(outputfile) != 0) {
		return -1;
	}
	if (fill_header(outputfile, inputfile, argv[3], argv[4], argv[5],
			argv[6]) != 0) {
		return -2;
	}
	if (fill_image(outputfile, inputfile) != 0) {
		return -3;
	}
	if (fill_dummy_footer(outputfile) != 0) {
		return -4;
	}
	return 0;
}

/**
 * @brief Print help message
 *
 */
static void print_help(void)
{
	int i = 0;

	printf("\n");
	printf("Telechips Image Maker for %s\n\n", SOC_NAME);
	printf("Usage: tcmktool %s\n\n",
	       "[INPUT] [OUTPUT] [NAME] [VERSION] [TARGET_ADDRESS]");
	printf("  INPUT\t\t input file name.\n");
	printf("  OUTPUT\t output file name.\n");
	printf("  NAME\t\t image name. (%s", image_name[i++]);
	while (image_name[i] != NULL) {
		printf(", %s", image_name[i++]);
	}
	printf(")\n");
	printf("  VERSION\t string version. (max 16 bytes)\n");
	printf("  TARGET_ADDRESS\t target address\n");
}

/**
 * @brief Main function
 *
 * @param argc Argument Count
 * @param argv Argument Value
 * @param envp Environment Parameter
 * @return Error status
 */
int main(int argc, char *argv[], char **envp)
{
	FILE *inputfile, *outputfile;
	int ret = -1;

	if (argc != 7) {
		print_help();
		return -1;
	}

	inputfile = fopen(argv[1], "rb");
	if (!inputfile) {
		printf("ERROR: input file open error\n");
	} else {
		outputfile = fopen(argv[2], "wb+");
		if (!outputfile) {
			printf("ERROR: output file open error\n");
		} else {
			ret = make_image(inputfile, outputfile, argv);
			fclose(outputfile);
		}
		fclose(inputfile);
	}

	if (ret == 0) {
		printf("%s was generated successfully\n", argv[2]);
	} else {
		printf("ERROR: output file generation error (error code: %d)\n", ret);
	}

	return ret;
}
