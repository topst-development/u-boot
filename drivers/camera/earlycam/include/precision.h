#ifndef CAM_PRECISION_H
#define CAM_PRECISION_H

/* Returns the number of set bits */
static inline size_t popcount(unsigned long long num) {
	size_t precision = 0;
	while (num != 0U) {
		if (((num % 2U) == 1U) && (precision < UINT_MAX)) {
			precision++;
		}
		num >>= 1;
	}
	return precision;
}

#define PRECISION(umax_value) popcount(umax_value)

#endif
