

#define DA_INIT_CAPACITY 255
#define DA_REALLOC(oldptr, oldsz, newsz) realloc(oldptr, newsz)

#define da_append(da, item)                                                 \
    do {                                                                    \
        if ((da)->count >= (da)->capacity) {                                \
            size_t new_capacity = (da)->capacity*2;                         \
            if (new_capacity == 0) {                                        \
                new_capacity = DA_INIT_CAPACITY;                            \
            }                                                               \
                                                                            \
            (da)->items = DA_REALLOC((da)->items,                           \
                                     (da)->capacity*sizeof((da)->items[0]), \
                                     new_capacity*sizeof((da)->items[0]));  \
            (da)->capacity = new_capacity;                                  \
        }                                                                   \
                                                                            \
        (da)->items[(da)->count++] = (item);                                \
    } while (0)

#define da_free(da)                 \
	do {                        \
		free((da)->items);  \
		(da)->count    = 0; \
		(da)->capacity = 0; \
	} while (0)
