#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <limits.h>

#include "config.h"

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
int nfd=0, wd=0;

void* watch_folder(const char *path) {
    char buf[BUF_LEN];
    ssize_t num_read;
    struct inotify_event *event;

    // Initialize notify
    nfd = inotify_init();
    if (nfd == -1) {
        printf("[HOTRELOAD] ERROR: inotify_init error: %m\n");
        perror("");
        return NULL;
    }

    wd = inotify_add_watch(nfd, path, IN_CLOSE_WRITE | IN_CREATE | IN_DELETE);
    if (wd == -1) {
        printf("[HOTRELOAD] ERROR: inotify_add_watch error: %m\n");
        return NULL;
    }

    printf("[HOTRELOAD] Watching '%s'\n", path);
    char* status = malloc(sizeof(char) * 8);

    while (1) {
        num_read = read(nfd, buf, BUF_LEN);
        if (num_read == 0) {
            printf("[HOTRELOAD] ERROR: read() from inotify nfd returned 0!\n");
            return NULL;
        }
        if (num_read == -1) {
            printf("[HOTRELOAD] ERROR: Read error!\n");
            return NULL;
        }

        for (char *ptr = buf; ptr < buf + num_read; ptr += sizeof(struct inotify_event) + event->len) {
            event = (struct inotify_event *)ptr;

            bool isExcludedFile = false;
            for(size_t i=0; i<HOTRELOAD_EXCLUDE_FILES_COUNT; i++){
                if(strcmp(HOTRELOAD_EXCLUDED_FILES[i], event->name) == 0){
                    isExcludedFile = true;
                }
            }
            if(isExcludedFile){
                continue;
            }

            status[0] = '\0';
            if (event->mask & IN_CLOSE_WRITE) {
                strcpy(status,"saved");
            }
            if (event->mask & IN_CREATE) {
                strcpy(status,"created");
            }
            if (event->mask & IN_DELETE) {
                strcpy(status,"deleted");
            }
            if(status[0] != '\0'){
                printf("[HOTRELOAD] ----------------------\n");
                printf("[HOTRELOAD] '%s/%s' file %s\n[HOTRELOAD] Reloading...\n", path, event->name, status);
                system("make hotreload");
                printf("[HOTRELOAD] ----------------------\n");
            }
        }
        struct timespec ts;
        ts.tv_sec = 2;
        ts.tv_nsec = 0;
        nanosleep(&ts, NULL);
    }
    free(status);
    // İzleme sonlandır
    inotify_rm_watch(nfd, wd);
    close(nfd);
    return NULL;
}

pthread_t hotreload_onchange(){
    pthread_t tid;
    if(pthread_create(&tid, NULL, watch_folder, HOTRELOAD_FOLDER) !=0){
        printf("[HOTRELOAD] ERROR: Some errors ocure when thread creating.%m!\n");
        return NULL;
    }
    return tid;
}
