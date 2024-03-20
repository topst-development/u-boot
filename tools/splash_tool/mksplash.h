#define IMAGE_SIZE_MAX 10
#define SPLASH_TAG_SIZE 16

#define _WIDTH 800
#define _HEIGHT 480

#define DEFAULT_HEADER_SIZE 512
#define DEFAULT_PAGE_SIZE 512

struct splash_image_info {
	unsigned char			uc_image_name[16];
	unsigned int			ul_image_addr;
	unsigned int			ul_image_size;
	unsigned int			ul_image_width;
	unsigned int			ul_image_height;
	unsigned int			padding;
	unsigned char			uc_fmt[5];
	unsigned char			uc_rev[7];
};

struct splash_image_header_info {
	unsigned char			uc_partition[20];
	unsigned int			ul_number;
	unsigned char			uc_rev[4];
	struct splash_image_info	splash_image[IMAGE_SIZE_MAX];
};

struct splash_buffer {
	unsigned char			*data;
	unsigned int			size;
};
