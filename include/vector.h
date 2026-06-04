#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define VECTOR_DEFINE(T, Name)                                      \
typedef struct {                                                    \
    T *data;                                                        \
    size_t len;                                                     \
    size_t cap;                                                     \
} Name;                                                            \
                                                                    \
static inline void Name##_init(Name *v) {                           \
    if (v == NULL) {                                                \
        return;                                                     \
    }                                                               \
    v->data = NULL;                                                 \
    v->len = 0;                                                     \
    v->cap = 0;                                                     \
}                                                                   \
                                                                    \
static inline bool Name##_reserve(Name *v, size_t min_cap) {        \
    if (v == NULL) {                                                \
        return false;                                               \
    }                                                               \
    if (v->cap >= min_cap) {                                        \
        return true;                                                \
    }                                                               \
                                                                    \
    size_t new_cap = v->cap == 0 ? 256 : v->cap;                    \
    while (new_cap < min_cap) {                                     \
        if (new_cap > SIZE_MAX / 2) {                               \
            new_cap = min_cap;                                      \
            break;                                                  \
        }                                                           \
        new_cap *= 2;                                               \
    }                                                               \
                                                                    \
    if (sizeof(T) == 0 || new_cap > SIZE_MAX / sizeof(T)) {         \
        return false;                                               \
    }                                                               \
                                                                    \
    T *new_data = realloc(v->data, new_cap * sizeof(T));            \
    if (new_data == NULL) {                                         \
        return false;                                               \
    }                                                               \
                                                                    \
    v->data = new_data;                                             \
    v->cap = new_cap;                                               \
    return true;                                                    \
}                                                                   \
                                                                    \
static inline bool Name##_push(Name *v, T value) {                  \
    if (v == NULL || v->len == SIZE_MAX) {                          \
        return false;                                               \
    }                                                               \
    if (!Name##_reserve(v, v->len + 1)) {                           \
        return false;                                               \
    }                                                               \
    v->data[v->len++] = value;                                      \
    return true;                                                    \
}                                                                   \
                                                                    \
static inline void Name##_free(Name *v) {                           \
    if (v == NULL) {                                                \
        return;                                                     \
    }                                                               \
    free(v->data);                                                  \
    Name##_init(v);                                                 \
}                                                                   \
                                                                    \

#endif
