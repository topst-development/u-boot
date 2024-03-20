/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define to_8888(r, g, b, a) \
	((((a) & 0xFF) << 24) | (((r) & 0xFF) << 16) | \
	(((g) & 0xFF) << 8) | ((b) & 0xFF))

void to_8888_raw(void)
{
	unsigned char in[4];
	unsigned int out;

	while (read(0, in, 4) == 4) {
		out = (to_8888(in[0], in[1], in[2], in[3]));
		write(1, &out, 4);
	}
}

#define COLOR_MAX_V (0xFFFFFF)
void to_8888_rle(void)
{
	unsigned char in[4];
	unsigned int last, color, count;
	unsigned int total = 0;

	count = 0;

	while (read(0, in, 4) == 4) {
		color = to_8888(in[0], in[1], in[2], in[3]);
		if (count) {
			if (color == last && count != COLOR_MAX_V) {
				count++;
				continue;
			} else {
				write(1, &count, 4);
				write(1, &last, 4);
				total += count;
			}
		}
		last = color;
		count = 1;
	}
	if (count) {
		write(1, &count, 4);
		write(1, &last, 4);
		total += count;
	}

	fprintf(stderr, "%d pixels\n", total);
}

int main(int argc, char **argv)
{
	if (argc == 2 && (!strcmp(argv[1], "-rle"))) {
		// rle code
		to_8888_rle();
	} else {
		// raw
		to_8888_raw();
	}
	return 0;
}
