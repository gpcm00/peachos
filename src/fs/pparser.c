#include "pparser.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/scrap/scrap.h"

#define ALLOC_ERROR     ((void*)(-1))


struct parser_object {
    struct scrap_memory* scrappad;
    struct path_root root;
};

static struct parser_object* init_praser_object()
{
    struct scrap_memory* scrappad = scrap_alloc_memory(sizeof(uint32_t));
    struct parser_object* pobj = scrap_alloc_data(scrappad, sizeof(struct parser_object));
    pobj->scrappad = scrappad;
    return pobj;
}

static bool pathparser_path_valid_format(const char* filename)
{
    int len = strnlen(filename, PEACHOS_MAX_PATH);
    return (len >= 3) && isdigit(filename[0]) && !memcmp(&filename[1], ":/", 2);
}

static int pathparser_get_drive_by_path(const char** path)
{
    if (!pathparser_path_valid_format(*path)) {
        return -EBADPATH;
    }

    int drive_no = tonumericdigit(*path[0]);

    *path += 3;
    return drive_no;
}

static void pathparser_create_root(int drive_number, struct path_root* path_r)
{
    path_r->drive_no = drive_number;
    path_r->first = NULL;
}

static char* pathparser_get_path_part(char** path, struct scrap_memory* scrappad)
{
    if (**path == '\0') {
        return NULL;
    }
    
    char* result = *path;

    while (**path != '/' && **path != '\0') {
        (*path)++;
    }

    if (**path == '/') {
        **path = '\0';
        (*path)++;
    }

    return result;
}

static struct path_part* pathparser_parse_path_part(struct path_part* last_part, char** path, 
                                                                            struct scrap_memory* scrappad)
{
    const char* parse_part = pathparser_get_path_part(path, scrappad);
    if (!parse_part) {
        return NULL;
    }

    struct path_part* part = scrap_alloc_data(scrappad, sizeof(struct path_part));
    if (!part) {
        return ALLOC_ERROR;
    }

    part->part = parse_part;
    part->next = NULL;
    if (last_part) {
        last_part->next = part;
    }

    return part;
}

void pathparser_free(struct path_root* path_root)
{
    struct parser_object* obj = container_of(path_root, struct parser_object, root);
    scrap_destroy_memory(obj->scrappad);
}

struct path_root* pathparser_parse(const char* path, const char* current_directory_path)
{
    int drive_no = pathparser_get_drive_by_path(&path);
    if (drive_no < 0) {
        return NULL;
    }

    size_t len = strnlen(path, PEACHOS_MAX_PATH);
    struct parser_object* pobj = init_praser_object();
    if (!pobj) {
        return NULL;
    }
    
    char* ipath = scrap_alloc_data(pobj->scrappad, len);
    memcpy(ipath, path, len);

    struct path_root* root = &pobj->root;

    pathparser_create_root(drive_no, root);
    root->first = pathparser_parse_path_part(NULL, &ipath, pobj->scrappad);
    if (!root->first) {
        goto Fail_Out;
    }

    struct path_part* last = root->first;
    while (last) {
        last = pathparser_parse_path_part(last, &ipath, pobj->scrappad);
        if (last == ALLOC_ERROR) {
            goto Fail_Out;
        }
    }
    
    return &pobj->root;

Fail_Out:
    pathparser_free(root);
    return NULL;
}