#include "pparser.h"
#include "string/string.h"
#include "memory/memory.h"
#include "memory/scrap/scrap.h"

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

static struct path_root* pathparser_create_root(int drive_number, struct scrap_memory* scrappad)
{
    struct path_root* path_r = scrap_alloc_data(scrappad, sizeof(struct path_root));
    path_r->drive_no = drive_number;
    path_r->first = NULL;
    return path_r;
}

static const char* pathparser_get_path_part(const char** path, struct scrap_memory* scrappad)
{
    char* result = scrap_alloc_data(scrappad, PEACHOS_MAX_PATH);
    if (!result) {
        return NULL;
    }

    size_t i = 0;
    while (**path != '/' && **path != '\0') {
        result[i] = **path;
        (*path)++;
        i++;
    }

    if (**path == '/') {
        (*path)++;
    }

    if (i == 0) {
        scrap_dealloc_data(scrappad, PEACHOS_MAX_PATH);
        return NULL;
    }

    return result;
}

static struct path_part* pathparser_parse_path_part(struct path_part* last_part, const char** path, struct scrap_memory* scrappad)
{
    const char* parse_part = pathparser_get_path_part(path, scrappad);
    if (!parse_part) {
        return NULL;
    }

    struct path_part* part = scrap_alloc_data(scrappad, sizeof(struct path_part));
    if (!part) {
        return NULL;
    }

    part->part = parse_part;
    part->next = NULL;
    if (last_part) {
        last_part->next = part;
    }

    return part;
}

void pathparser_free(struct path_part* root)
{
    scrap_destroy_memory(root);
}