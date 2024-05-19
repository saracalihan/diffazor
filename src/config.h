#ifndef CONFIG_H
#define CONFIG_H

#define HOTRELOAD_ONCHANGE
#define HOTRELOAD_WITH_KEYPRESS
#define HOTRELOAD_FOLDER "./src"


#define HOTRELOAD_EXCLUDE_FILES_COUNT 3
char* HOTRELOAD_EXCLUDED_FILES[HOTRELOAD_EXCLUDE_FILES_COUNT]={
    "main.c",
    "hotreload.c",
    "config.h",
};

#endif // CONFIG_H
