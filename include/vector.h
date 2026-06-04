#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>

bool vector_reserve(void **data, size_t *cap, size_t min_cap,
                    size_t elem_size);
bool vector_push(void **data, size_t *len, size_t *cap, const void *value,
                 size_t elem_size);
void vector_free(void **data, size_t *len, size_t *cap);

#define VECTOR_DEFINE(T, Name)                                               \
typedef struct {                                                             \
    T *data;                                                                 \
    size_t len;                                                              \
    size_t cap;                                                              \
} Name;                                                                      \
                                                                             \
static inline void Name##_init(Name *v) {                                    \
    if (v == NULL) {                                                         \
        return;                                                              \
    }                                                                        \
    v->data = NULL;                                                          \
    v->len = 0;                                                              \
    v->cap = 0;                                                              \
}                                                                            \
                                                                             \
static inline bool Name##_reserve(Name *v, size_t min_cap) {                 \
    if (v == NULL) {                                                         \
        return false;                                                        \
    }                                                                        \
    void *data = v->data;                                                    \
    if (!vector_reserve(&data, &v->cap, min_cap, sizeof(T))) {               \
        return false;                                                        \
    }                                                                        \
    v->data = data;                                                          \
    return true;                                                             \
}                                                                            \
                                                                             \
static inline bool Name##_push(Name *v, T value) {                           \
    if (v == NULL) {                                                         \
        return false;                                                        \
    }                                                                        \
    void *data = v->data;                                                    \
    if (!vector_push(&data, &v->len, &v->cap, &value, sizeof(T))) {          \
        return false;                                                        \
    }                                                                        \
    v->data = data;                                                          \
    return true;                                                             \
}                                                                            \
                                                                             \
static inline void Name##_free(Name *v) {                                    \
    if (v == NULL) {                                                         \
        return;                                                              \
    }                                                                        \
    void *data = v->data;                                                    \
    vector_free(&data, &v->len, &v->cap);                                    \
    v->data = data;                                                          \
}

#endif
