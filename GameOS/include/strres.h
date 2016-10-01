#ifndef STRRES_H
#define STRRES_H

#pragma once

typedef const char* (*get_string_by_id_fptr)(unsigned int);

    typedef void(*init_string_resources_fptr)(void);
    typedef void(*free_string_resources_fptr)(void);

struct gos_StringRes {
    const char** strings;
    int num_strings;
    void* module;
    get_string_by_id_fptr getStringByIdFptr;
};

#endif // STRRES_H
