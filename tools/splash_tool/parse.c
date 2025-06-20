#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include "mksplash.h"
#include <linux/errno.h>

#define max_size_with_null 8

void usage();

int main(int argc, char* argv[]) {

	struct splash_image_header_info header = {0};
	int option = 0;
	int buffer_len = -1;
	int partition_index = -1;
	bool global_header = (bool)true;
	bool read_or_write = (bool)false;
	char buffer[max_size_with_null] = {0};
	int fd = -1;
	int ret = 0;
	bool help = (bool)false;


	while (!ret && (option = getopt(argc, argv, "w:s:f:rh")) != EOF) {
		switch (option) {
			case 'f' :
				if (optarg == NULL) {
					ret = -EINVAL;
                                	break;
				} else {
					fd = open(optarg, O_RDWR);
                                	if (fd < 0) {
                                        	printf("No such file [%s]\n", optarg);
                                        	ret  = -EINVAL;
                                        }
				}
				break;

			case 's' :
				if (optarg == NULL) {
					ret = -EINVAL;
				} else {
					partition_index = atoi(optarg);
					if ((partition_index < 0) || (partition_index > 9)) {
						ret = -EINVAL;
					} else {
						global_header = (bool)false;
					}
				}
				break;

			case 'w' :
				if (optarg == NULL) {
					ret = -EINVAL;
				} else {
					buffer_len = strlen(optarg);
					strncpy(buffer, optarg, buffer_len);
					read_or_write = (bool)true;
				}
				break;

			case 'h' :
				help = (bool)true;
				break;

			case '?' :
				help = (bool)true;
				break;
			}
	}

	if (help || (ret < 0) || (fd < 0)) {
		usage();
		goto out_close;
        }

	read(fd, &header, sizeof(header));

	if (read_or_write) { //write
		lseek(fd, 0, SEEK_SET);
		if (global_header) {
			if (buffer_len > sizeof(header.uc_rev)) {
				usage();
				ret = -EINVAL;
			} else {
				memset(header.uc_rev, 0, sizeof(header.uc_rev));
				strncpy(header.uc_rev, buffer, buffer_len);
				write(fd, &header, sizeof(header));
			}
		} else {
			if (buffer_len > sizeof(header.splash_image[partition_index].uc_rev)) {
				usage();
				ret = -EINVAL;
			} else {
				memset(header.splash_image[partition_index].uc_rev, 0, sizeof(header.splash_image[partition_index].uc_rev));
				strncpy(header.splash_image[partition_index].uc_rev, buffer, buffer_len);
				write(fd, &header, sizeof(header));
			}
		}
	} else { //read
		if (global_header) {
			strncpy(buffer, header.uc_rev, sizeof(header.uc_rev));
		} else {
			strncpy(buffer, header.splash_image[partition_index].uc_rev, sizeof(header.splash_image[partition_index].uc_rev));
		}
		printf("%s\n", buffer);
	}

out_close:
	if (fd >= 0) {
		close(fd);
        }
	return ret;
}

void usage() {
	printf("\nUsage\n");
	printf("[-f optarg] : Read splash image, optarg is splash image name.\n");
	printf("[-w optarg] : You can wirte the datas in global reserved area.\n");
	printf("              The max size of global reserved area is 4 and each picture of reserved area is 7\n");
	printf("[-r]        : You can read the data in global reserved area.\n");
	printf("[-s optarg] : You have to use this option with -r or -w.\n");
	printf("              Thanks to optarg, you can approach each partition.\n");
	printf("              The range of optarg is 0~9.\n");
	printf("Example     : ./reserved_parsing -f splash.img -r             ->   read global reserved area\n");
	printf("Example     : ./reserved_parsing -f splash.img -w 1234 -s 3   ->   write datas in forth reserved area\n");
}
