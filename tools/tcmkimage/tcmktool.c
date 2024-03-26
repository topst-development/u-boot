// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 Telechips Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sha256.h"

#define ALIGN_SIZE 64U
#define IMAGE_NAME_MAX_SIZE 12U
#define IMAGE_VERSION_MAX_SIZE 16U
#define BUILD_INFO_MAX_SIZE 14U

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
	uint8_t imagename[IMAGE_NAME_MAX_SIZE];
	uint8_t imageversion[IMAGE_VERSION_MAX_SIZE];
	uint64_t targetaddress;
	uint32_t execstate;
	uint8_t reserved2[4];
	uint8_t buildinfo[16];
	uint8_t reserved3[16];
	uint8_t bodyhash[32];
	uint8_t reserved4[128]; /* This is for Sign Tool */
};

/* Image Footer that is for Sign Tool */
struct image_footer {
	uint8_t reserved[128];
};

/**
 * @brief Fill a dummy certificate that will be changed by Sign Tool
 *
 * @param outputfile File pointer of output file
 * @return Error status
 */
static int fill_dummy_cert(FILE *outputfile)
{
	size_t ret;

	const struct image_cert dummy_cert = {
		.marker = { 'C', 'E', 'R', 'T' },
		.reserved = { 0, },
	};

	ret = fwrite(&dummy_cert, sizeof(struct image_cert), 1, outputfile);

	return (ret == 1) ? 0 : -1;
}

/**
 * @brief Calculate hash of image
 *
 * @param hash Pointer to obtain hash of image
 * @param inputfile File pointer of input file
 * @return Error status
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
 * @param outputfile	File pointer of output file
 * @param inputfile	File pointer of input file
 * @param imagename	String of image name
 * @param imageversion	String of image version
 * @param buildinfo	String of image commit-id (abbrev=7)
 * @param targetaddress	String of image target address
 * @param execstate	String of image execution state (0x32 or 0x64)
 * @return Error status
 */
static int fill_header(FILE *outputfile, FILE *inputfile,
		       const char *imagename, const char *imageversion,
		       const char *buildinfo, const char *targetaddress,
		       const char *execstate)
{
	struct image_header header;
	uint32_t length;
	uint8_t *buf;
	uint32_t offset;
	size_t ret;

	fseek(inputfile, 0, SEEK_END);
	length = (ftell(inputfile) + (ALIGN_SIZE - 1)) & ~(ALIGN_SIZE - 1);

	memset(&header, 0, sizeof(struct image_header));

	memcpy(header.marker, "HDR\0", 4);
	memcpy(header.socname, SOC_NAME, 4);
	if (strlen(imagename) > IMAGE_NAME_MAX_SIZE) {
		return -101;
	}

	memcpy(header.imagename, imagename, strlen(imagename));

	if ((strlen(imageversion) > IMAGE_VERSION_MAX_SIZE) ||
	    (strlen(buildinfo) > BUILD_INFO_MAX_SIZE)) {
		return -102;
	}
	memcpy(header.imageversion, imageversion, strlen(imageversion));
	memcpy(header.buildinfo, buildinfo, strlen(buildinfo));

	header.bodylength = length + sizeof(struct image_footer);
	header.bodyoffset = BODY_OFFSET;
	header.targetaddress = strtol(targetaddress, NULL, 16);
	header.execstate = strtol(execstate, NULL, 16);
	calc_hash(header.bodyhash, inputfile);

	ret = fwrite(&header, sizeof(struct image_header), 1, outputfile);
	if (ret != 1) {
		return -103;
	}

	offset = ftell(outputfile);
	if (header.bodyoffset > offset) {
		buf = malloc(header.bodyoffset - offset);
		if (buf == NULL) {
			return -104;
		}
		memset(buf, 0, ALIGN_SIZE);
		ret = fwrite(buf, header.bodyoffset - offset, 1, outputfile);
		if (ret != 1) {
			free(buf);
			return -105;
		}
		free(buf);
	}
	return 0;
}

/**
 * @brief Fill an image
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
 * @brief Fill a dummy footer that will be changed by Sign Tool
 *
 * @param outputfile File pointer of output file
 * @return Error status
 */
static int fill_dummy_footer(FILE *outputfile)
{
	size_t ret;

	const struct image_footer dummy_footer = {
		.reserved = { 0, },
	};

	ret = fwrite(&dummy_footer, sizeof(struct image_footer), 1, outputfile);

	return (ret == 1) ? 0 : -1;
}

/**
 * @brief Make an image
 *
 * @param inputfile File pointer of input file
 * @param outputfile File pointer of output file
 * @param argv Argument value
 * @return Error status
 */
static int make_image(FILE *inputfile, FILE *outputfile, char *argv[])
{
	if (fill_dummy_cert(outputfile) != 0) {
		return -1;
	}
	if (fill_header(outputfile, inputfile, argv[3], argv[4], argv[5],
			argv[6], argv[7]) != 0) {
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
	printf("\n");
	printf("Telechips Image Maker for %s\n", SOC_NAME);
	printf("\n");
	printf("Usage: tcmktool [PARAMS]\n");
	printf("Params:\n");
	printf("  INPUT_FILE     input file name\n");
	printf("  OUTPUT_FILE    output file name\n");
	printf("  NAME           image name");
	printf("  VERSION        version in string (max 16 bytes)\n");
	printf("  BUILD_INFO     build info\n");
	printf("  TARGET_ADDRESS target address\n");
	printf("  EXEC_STATE     execution state (0x32 or 0x64)\n");
	printf("\n");
	printf("  * Input params in above order\n");
}

/**
 * @brief Main function
 *
 * @param argc Argument count
 * @param argv Argument value
 * @param envp Environment parameter
 * @return Error status
 */
int main(int argc, char *argv[], char **envp)
{
	FILE *inputfile, *outputfile;
	int ret = -1;

	if (argc != 8) {
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
		printf("ERROR: output file generation error (error code: %d)\n",
		       ret);
	}

	return ret;
}
