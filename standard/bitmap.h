#ifndef BITMAP_UTILS
#define BITMAP_UTILS

inline void bit_set (uint32_t *map, uint32_t bit) {
	map[bit / 32] |= (1 << (bit % 32));
}

inline void bit_unset (uint32_t *map, uint32_t bit) {
	map[bit / 32] &= ~ (1 << (bit % 32));
}

inline bool bit_test (uint32_t *map, uint32_t bit) {
	return map[bit / 32] &  (1 << (bit % 32));
}

#endif
