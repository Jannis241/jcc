#include "../include/vector.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    VECTOR_INITIAL_CAPACITY = 256,
};

bool vector_reserve(void **data, size_t *cap, size_t min_cap,
                    size_t elem_size) {
    if (data == NULL || cap == NULL || elem_size == 0) {
        return false;
    }

    if (*cap >= min_cap) {
        return true;
    }

    size_t new_cap = *cap == 0 ? VECTOR_INITIAL_CAPACITY : *cap;
    while (new_cap < min_cap) {
        if (new_cap > SIZE_MAX / 2) {
            new_cap = min_cap;
            break;
        }
        new_cap *= 2;
    }

    if (new_cap > SIZE_MAX / elem_size) {
        return false;
    }

    void *new_data = realloc(*data, new_cap * elem_size);
    if (new_data == NULL) {
        return false;
    }

    *data = new_data;
    *cap = new_cap;
    return true;
}

bool vector_push(void **data, size_t *len, size_t *cap, const void *value,
                 size_t elem_size) {
    if (data == NULL || len == NULL || cap == NULL || value == NULL ||
        *len == SIZE_MAX) {
        return false;
    }

    if (!vector_reserve(data, cap, *len + 1, elem_size)) {
        return false;
    }

    unsigned char *bytes = *data;
    memcpy(bytes + (*len * elem_size), value, elem_size);
    *len += 1;
    return true;
}

void vector_free(void **data, size_t *len, size_t *cap) {
    if (data == NULL || len == NULL || cap == NULL) {
        return;
    }

    free(*data);
    *data = NULL;
    *len = 0;
    *cap = 0;
}
