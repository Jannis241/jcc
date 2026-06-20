#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef VECTOR_INITIAL_CAPACITY
#define VECTOR_INITIAL_CAPACITY 8
#endif

static inline bool vector_reserve(void **data, size_t *cap, size_t min_cap,
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

static inline bool vector_push(void **data, size_t *len, size_t *cap,
                               const void *value, size_t elem_size) {
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

static inline void vector_free(void **data, size_t *len, size_t *cap) {
    if (data == NULL || len == NULL || cap == NULL) {
        return;
    }

    free(*data);
    *data = NULL;
    *len = 0;
    *cap = 0;
}

#define VECTOR_DEFINE(T, Name)                                                 \
    typedef struct {                                                           \
        T *data;                                                               \
        size_t len;                                                            \
        size_t cap;                                                            \
    } Name;                                                                    \
                                                                               \
    static inline void Name##_init(Name *v) {                                  \
        if (v == NULL) {                                                       \
            return;                                                            \
        }                                                                      \
        v->data = NULL;                                                        \
        v->len = 0;                                                            \
        v->cap = 0;                                                            \
    }                                                                          \
                                                                               \
    static inline bool Name##_reserve(Name *v, size_t min_cap) {               \
        if (v == NULL) {                                                       \
            return false;                                                      \
        }                                                                      \
        void *data = v->data;                                                  \
        if (!vector_reserve(&data, &v->cap, min_cap, sizeof(T))) {             \
            return false;                                                      \
        }                                                                      \
        v->data = data;                                                        \
        return true;                                                           \
    }                                                                          \
                                                                               \
    static inline bool Name##_push(Name *v, T value) {                         \
        if (v == NULL) {                                                       \
            return false;                                                      \
        }                                                                      \
        void *data = v->data;                                                  \
        if (!vector_push(&data, &v->len, &v->cap, &value, sizeof(T))) {        \
            return false;                                                      \
        }                                                                      \
        v->data = data;                                                        \
        return true;                                                           \
    }                                                                          \
                                                                               \
    static inline void Name##_free(Name *v) {                                  \
        if (v == NULL) {                                                       \
            return;                                                            \
        }                                                                      \
        void *data = v->data;                                                  \
        vector_free(&data, &v->len, &v->cap);                                  \
        v->data = data;                                                        \
    }

#endif
