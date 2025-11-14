#include <stdlib.h>
#include <string.h>
#include "store.h"
#include "util.h"

#define START_CAP 16

static bool ensure_cap(Store *s, size_t need) {
    if (s->cap >= need) {
        return true;
    }
    size_t new_cap = s->cap ? s->cap : START_CAP;
    while (new_cap < need) {
        new_cap *= 2;
    }

    Student *new_alloc = realloc(s->data, new_cap * sizeof(Student));
    if (!new_alloc) {
        return false;
    }
    s->data = new_alloc;
    s->cap = new_cap;
    return true;
}

void store_init(Store *s) {
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

void store_free(Store *s) {
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

int store_find_index_by_id(const Store *s, int id) {
    for (size_t i = 0; i < s->size; i++) {
        if (s->data[i].id == id) {
            return (int)i;
        }
    }
    return -1;
}

bool store_insert(Store *s, Student st) {
    if (!valid_id(st.id) || !valid_mark(st.mark) || !valid_text(st.name) || !valid_text(st.programme)) {
        return false;
    }

    if (store_find_index_by_id(s, st.id) != -1) {
        return false; // Duplicate ID
    }

    if (!ensure_cap(s, s->size + 1)) {
        return false; // Memory allocation failed
    }

    s->data[s->size++] = st;
    return true;
}

bool store_update(Store *s, int id, const Student *patch) {
    int idx = store_find_index_by_id(s, id);
    if (idx < 0) return false;
    Student *cur = &s->data[idx];
    if (patch->id > 0 && patch->id != id) {
        if (!valid_id(patch->id)) return false;
        if (store_find_index_by_id(s, patch->id) != -1) return false;
        cur->id = patch->id;
    }

    if (patch->name[0] != '\0') {
        if (!valid_text(patch->name)) return false;
        strncpy(cur->name, patch->name, sizeof(cur->name));
        cur->name[sizeof(cur->name)-1] = '\0';
    }

    if (patch->programme[0] != '\0') {
        if (!valid_text(patch->programme)) return false;
        strncpy(cur->programme, patch->programme, sizeof(cur->programme));
        cur->programme[sizeof(cur->programme)-1] = '\0';
    }

    if (patch->mark >= 0.0f) {
        if (!valid_mark(patch->mark)) return false;
        cur->mark = patch->mark;
    }

    return true;
}

bool store_delete(Store *s, int id) {
    int idx = store_find_index_by_id(s, id);
    if (idx < 0) return false;
    s->data[idx] = s->data[s->size - 1]; // Swap with last student record
    s->size--;
    return true;
}