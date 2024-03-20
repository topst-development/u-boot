#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/stat.h>

#include "mksplash.h"

#define PAGE_MASK(page_size)	((page_size) - 1)
#define PAGE_ALIGN(x, y)	(((x) + (PAGE_MASK(y))) & (~(PAGE_MASK(y))))

static int get_file_size(int fd)
{
	return lseek(fd, 0, SEEK_END);
}

static unsigned char padding[16384] = {
	0,
};

static int write_padding(int fd, unsigned int pagesize, unsigned int itemsize)
{
	unsigned int page_aligned_size = PAGE_ALIGN(itemsize, pagesize);
	int count;

	if (itemsize == page_aligned_size) {
		// padding is not needed
		return 0;
	}

	count = page_aligned_size - itemsize;
	if (write(fd, padding, count) != count) {
		// byte of write is wrong
		return -1;
	}

	return count;
}

static void *load_file(char *file_name, int *size)
{
	int sz, fd;
	char *data = 0;
	int read_cnt = 0;

	fd = open(file_name, O_RDONLY, 0644);
	if (fd < 0) {
		printf(" File [%s] Open Failed !!\n", file_name);
		goto opps;
	}

	sz = get_file_size(fd);

	printf("image read size = %d\n", sz);

	if (sz < 0) {
		printf(" Get File [%d]  Size Failed !!\n", sz);
		goto opps;
	}

	lseek(fd, 0, SEEK_SET);
	data = (char *)malloc(sz);
	if (data == 0) {
		// failure of allocation
		goto opps;
	}
	memset(data, 0x0, sz);

	read_cnt = read(fd, data, sz);
	if (read_cnt != sz) {
		// failure of read
		goto opps;
	}

	close(fd);

	if (size) {
		// update size
		*size = sz;
	}

	return data;

opps:
	close(fd);
	if (data != 0) {
		// free data
		free(data);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 5) {
		fprintf(stderr, "Wrong arguments\n");
		return 1;
	}

	int image_cnt = atoi(argv[2]);

	struct splash_image_header_info splash_image;
	struct splash_buffer *splash_buffer =
	    malloc(sizeof(struct splash_buffer) * image_cnt);
	unsigned int p_addr = 0, p_size = 0, p_pad = 0,
		     page_size = DEFAULT_PAGE_SIZE;
	int fin, fout;
	int ret, idx, size;
	char **image_name;
	char **image_resolution;
	char **image_fmt;
	char *part_name;
	char partition_name[SPLASH_TAG_SIZE];

	page_size = atoi(argv[1]);

	printf("pagesize : %d\n", page_size);

	strcpy(partition_name, argv[3]);

	memset(&splash_image, 0x0, sizeof(splash_image));
	memcpy(splash_image.uc_partition, partition_name, SPLASH_TAG_SIZE);

	image_name = argv + 4;
	image_resolution = image_name + image_cnt;
	image_fmt = image_resolution + image_cnt;
	splash_image.ul_number = image_cnt;

	printf(" ## %s image number = %d\n",
	       splash_image.uc_partition, splash_image.ul_number);

	for (idx = 0; idx < image_cnt; idx++) {
		splash_buffer[idx].data = load_file(image_name[idx], &size);
		if (splash_buffer[idx].data) {
			if (strrchr(image_name[idx], '/') != NULL) {
				part_name = strtok(
				    (strrchr(image_name[idx], '/') + 1), ".");
			} else {
				part_name = strtok(image_name[idx], ".");
			}
			memcpy(splash_image.SPLASH_IMAGE[idx].uc_image_name,
			       part_name, strlen(part_name)); // save 16 ch
			memcpy(splash_image.SPLASH_IMAGE[idx].uc_fmt,
			       image_fmt[idx], strlen(image_fmt[idx]));

			if (idx == 0) {
				splash_image.SPLASH_IMAGE[idx].ul_image_addr =
				    PAGE_ALIGN(sizeof(splash_image), page_size);
			} else {
				splash_image.SPLASH_IMAGE[idx].ul_image_addr =
				    (p_addr + p_size + p_pad);
			}

			splash_image.SPLASH_IMAGE[idx].ul_image_size = size;
			splash_buffer[idx].size = size;
			splash_image.SPLASH_IMAGE[idx].ul_image_width =
			    atoi(strtok(image_resolution[idx], "x"));
			splash_image.SPLASH_IMAGE[idx].ul_image_height =
			    atoi(strtok(NULL, " "));

			if (PAGE_ALIGN(size, page_size) != size) {
				splash_image.SPLASH_IMAGE[idx].padding =
				    PAGE_ALIGN(size, page_size) - size;
			} else {
				splash_image.SPLASH_IMAGE[idx].padding = 0;
			}

			p_addr = splash_image.SPLASH_IMAGE[idx].ul_image_addr;
			p_size = splash_image.SPLASH_IMAGE[idx].ul_image_size;
			p_pad = splash_image.SPLASH_IMAGE[idx].padding;

			printf(" ### %s\n",
			       splash_image.SPLASH_IMAGE[idx].uc_image_name);
			printf(" ### %d\n",
			       splash_image.SPLASH_IMAGE[idx].ul_image_addr);
			printf(" ### %d\n",
			       splash_image.SPLASH_IMAGE[idx].ul_image_size);
			printf(" ### %d\n",
			       splash_image.SPLASH_IMAGE[idx].padding);
			printf(" ### %d\n",
			       splash_image.SPLASH_IMAGE[idx].ul_image_width);
			printf(" ### %d\n",
			       splash_image.SPLASH_IMAGE[idx].ul_image_height);
			printf(" ### %s\n",
			       splash_image.SPLASH_IMAGE[idx].uc_fmt);
		}
	}

	fout = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (fout < 0) {
		printf("[error] Can not open header for writing\n");
		return -1;
	}

	ret = write(fout, &splash_image, sizeof(splash_image));
	if (ret == 0) {
		// failure of write
		printf("[error] Can not make file !!!\n");
	}

	ret = write_padding(fout, page_size, sizeof(splash_image));
	if (ret < 0) {
		// failure of write_paading
		return -1;
	}

	for (idx = 0; idx < image_cnt; idx++) {
		ret = write(fout, splash_buffer[idx].data,
			    splash_buffer[idx].size);
		if (ret == 0) {
			// failure of write( )
			printf("[error] Can not make file !!!\n");
		} else if (ret < 0) {
			// failure of write( )
			perror("write error : ");
		} else if (ret != splash_buffer[idx].size) {
			// success of write( )
			printf("%d write success(input size : %d)\n",
			       ret, splash_buffer[idx].size);
		}

		ret = write_padding(fout, page_size, splash_buffer[idx].size);
		if (ret < 0) {
			// ret of write_padding is error
			return -1;
		}

		free(splash_buffer[idx].data);
	}

	printf("sizeof(struct display_header): %d\n",
	       (unsigned int)sizeof(splash_image));

	free(splash_buffer);

	return 0;
}
