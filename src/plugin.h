#ifndef PLUGIN_H
#define PLUGIN_H
#include <stddef.h>
#include <raylib.h>
#include <string.h>

typedef struct {
        char *items;
        int count;
        int capacity;
        int line_count;
} TextVector;

typedef struct{
        TextVector input;
        Font font;
} Plugin;


// void plugin_init();
// Plugin* plugin_preload();
// void plugin_load(Plugin*);
// Plugin* plugin_postload();
// void plugin_task();

#endif // PLUGIN_H
