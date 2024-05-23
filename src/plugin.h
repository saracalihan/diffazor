#ifndef PLUGIN_H
#define PLUGIN_H
#include <stddef.h>
#include <raylib.h>
#include <string.h>

typedef enum{
        ADD,
        DELETE,
        SAME,
}DiffType;

typedef struct{
        char c;
        DiffType diff;
} TextDiff;


typedef struct {
        TextDiff *items;
        int count;
        int capacity;
        int line_count;
} TextVector;

typedef struct{
        TextVector input1;
        TextVector input2;
        TextVector inputs_diff;
        TextVector virtualized_text;
        Font font;
} Plugin;


// void plugin_init();
// Plugin* plugin_preload();
// void plugin_load(Plugin*);
// Plugin* plugin_postload();
// void plugin_task();

#endif // PLUGIN_H
