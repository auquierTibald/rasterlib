#ifndef RASTERLIB_UTILS_H
#define RASTERLIB_UTILS_H

#include <stdio.h>

#define da(type) struct { \
    type* data; \
    size_t size; \
    size_t capacity; \
}

#define da_alloc(type, sz) {.data = (type*)malloc(sizeof(type) * (sz) ), .size = 0, .capacity = (sz) }

#define da_append(da_ptr, type, element) \
do { \
    if( (da_ptr)->size >= (da_ptr)->capacity ) { \
        (da_ptr)->capacity *= 2; \
        (da_ptr)->data = realloc( (da_ptr)->data, (da_ptr)->capacity * sizeof(type) ); \
    } \
    (da_ptr)->data[(da_ptr)->size] = (element); \
    (da_ptr)->size++; \
} while (0)

#define da_clear(da_ptr) (da_ptr)->size = 0;

#define da_foreach(da_ptr, type) for(type *element = (da_ptr)->data; element != NULL && element < (da_ptr)->data + (da_ptr)->size; element++)
#define da_range(da_ptr, type, start, end) for(type *element = ((da_ptr)->data) + start; element != NULL && element < ((da_ptr)->data) + end; element++)


#define da_free(da_ptr) do { \
    free(da_ptr); \
    (da_ptr)->size = 0; \
    (da_ptr)->capacity = 0; \
} while(0)

#endif //RASTERLIB_UTILS_H