#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "raylib.h"
#include "plugin.h"

#define FONT_SIZE 18
#define INPUT_FILE_PATH "./input.txt"
#define FONT_PATH "./resources/LiberationMono-Regular.ttf"
#define SCROLL_SENSIVITY 3
#define LINE_SPACING 2

static Plugin* p = NULL;
int screenHeight = 0,
    screenWidth =0,
    startLineNumber = 0,
    lineHeight=0,
    screenLineCount = 0,
    // Sidebar
    sidebarWidth=16;

float scrollValue =0;

char* virtualizedText=0;

bool isFileEnd = false;

Color editorBg    = {17, 18, 18, 255},
      editorText  = WHITE,
      sidebarBg   = {40, 41, 41, 255},
      sidebarText = GRAY,
      scrollBg = GRAY,
      scrollHover = RED;

void draw_text(char* text, int x, int y, Color color){
    DrawTextEx(p->font, text, (Vector2){ .x=x, .y=y}, FONT_SIZE, 0, color);
}

void draw_text_ex(char* text, int x, int y, int size, Color color){
    DrawTextEx(p->font, text, (Vector2){ .x=x, .y=y}, size, 0, color);
}



void read_input(){
    int size = 0;

    FILE *f = fopen(INPUT_FILE_PATH, "r");
    if(f == NULL){
        TraceLog(LOG_ERROR, "%s file can not open! %m", INPUT_FILE_PATH);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if(p->input.items !=NULL){
        free(p->input.items);
    }
    p->input.items = malloc(sizeof(char) * size);
    p->input.count = size;
    p->input.line_count=1;
    for(int i =0; i<size; i++){
        char c = getc(f);
        p->input.items[i] = c;
        c == '\n' && p->input.line_count++;
    }

    fclose(f);
    virtualizedText = p->input.items;

    char* str[6]={0};
    sidebarWidth = MeasureTextEx(p->font, str, FONT_SIZE, 0).y +8;
}

void handle_scroll(){
    static int lastLineCount = 0;
    static int lastStartLineNum = 0;
    static int lastPos = 0;

    int largestLineLetterCount = 0;
    float mouseScrollY = GetMouseWheelMove();

    if(mouseScrollY<0 && lastLineCount+1 >= p->input.line_count){
        return;
    }
    Vector2 fontSize = MeasureTextEx(p->font, "A", p->font.baseSize, 0);
    int fontHeight = fontSize.y;
    int fontWidth = fontSize.x;

    lineHeight = fontHeight + LINE_SPACING;
    screenLineCount = screenHeight / (FONT_SIZE-3);

    scrollValue += mouseScrollY * SCROLL_SENSIVITY;
    startLineNumber = (int)(scrollValue < 0 ? abs(scrollValue) : 0);

    int curPos =0,
        line_count=0;
    // TODO: curPos'un bulunması için stirng en başından itibaren okunuyor. Bunun yerine son pozisyon tutulup oradan başlanabilir.
    for(int i=0; i< startLineNumber; i++){
        if(line_count + screenLineCount >= p->input.count){
            break;
        }
        int letterCount=0;

        // Read line
        while(p->input.items[curPos] != '\n'){
           curPos++;
           letterCount++;
        }

                // For vertical line scrolling
        if(letterCount>largestLineLetterCount){
            largestLineLetterCount = letterCount;
        }
            // printf("%i:%i %i %i\n",line_count ,largestLineLetterCount, startLineNumber, curPos);
        line_count++;
        curPos++;
    }
    /*
    // scrolled line count and scroll direction
    int step =0;
        startLineNumber > lastStartLineNum
            ? startLineNumber - lastStartLineNum
            : lastLineCount   - startLineNumber;

    for(int i=0; i < abs(step); i++){
        // if(line_count + screenLineCount >= p->input.count){
        //     break;
        // }
        int letterCount=0;
        // Read line
        // stepe göre direction değişecek ve pos arttırılacak veya azaltılacak
        while(p->input.items[lastPos + curPos] != '\n'){
           curPos++;
           letterCount++;
        }

                // For vertical line scrolling
        if(letterCount>largestLineLetterCount){
            largestLineLetterCount = letterCount;
        }
            printf("%i:%i %i %i\n",line_count ,largestLineLetterCount, startLineNumber, curPos);
        line_count++;
        curPos++;
    }
*/
    // TODO: tüm string görünmese bile ekrana yazılıyor. Burada gerçekten kesilme yapılmalı
    // Set editor text
    virtualizedText = p->input.items + curPos;

    lastPos = curPos;

    // Draw horizental scroll
    int width = 6;
    bool isHover = false;
    Rectangle r = {
        .x      = screenWidth - width,
        .y      = (screenHeight /  p->input.line_count )* startLineNumber,
        .width  = width,
        .height = screenHeight / (p->input.line_count / screenLineCount) ,
    };
    Vector2 m = GetMousePosition();
    if(CheckCollisionPointRec( m, r)){
        isHover = true;
    }
    DrawRectangleRec(r, isHover ? scrollHover : scrollBg);

    // Draw vertical scroll
    if(largestLineLetterCount * (fontWidth + LINE_SPACING) > screenWidth ){
        DrawRectangle(
            0,
            screenHeight,
            50,
            width,
            RED
        );
    }

    if(line_count != lastLineCount){
        lastLineCount = line_count + screenLineCount;
    }
}

void plugin_task(){
    screenHeight = GetScreenHeight();
    screenWidth = GetScreenWidth();
    BeginDrawing();
        ClearBackground(editorBg);
        if(IsKeyPressed(KEY_F)){
            flush_plugin();
        }

        // Draw text
        draw_text(
            virtualizedText,
            (sidebarWidth +4) + ( FONT_SIZE/2 ) - (0* FONT_SIZE/2), // sidebar + pading + back
            0,
            editorText
        );

        // LOGIC STARTS HERE
        handle_scroll();

        // Draw line numbers
        DrawRectangle(
            0,
            0,
            sidebarWidth+6,
            screenHeight,
            sidebarBg
        );

        char str[p->input.line_count];
        memset(str, 0, p->input.line_count);
        for(int i=0; i<= screenLineCount; i++){
                 sprintf(str, "%s%d\n",str, startLineNumber + i + 1);
        }
        draw_text(
            str,
            0,
            0,
            sidebarText
        );

    EndDrawing();
}

void load_assets(){
    if(p == NULL){
        return TraceLog(LOG_ERROR, "[LOAD ASSET] Plugin is null!");
    }
    Font f = LoadFontEx(FONT_PATH, 32, NULL, 0);
    memcpy(&p->font, &f, sizeof(f));
    if (p->font.texture.id == 0) {
        return TraceLog(LOG_ERROR, "[LOAD ASSET] %s font can not loading!", FONT_PATH);
    }
    TraceLog(LOG_INFO, "[LOAD ASSET] Assets loaded.");
}

void unload_assets(){
    if(p == NULL){
        return TraceLog(LOG_ERROR, "[LOAD ASSET] Plugin is null!");
    }
    UnloadFont(p->font);
    TraceLog(LOG_INFO, "[LOAD ASSET] Assets unloaded.");
}

void flush_plugin(){
    if(p == NULL){
        p = malloc(sizeof(Plugin));
        TraceLog(LOG_INFO, "Plugin data allocated");
    }
    assert(p != NULL && "Plugin malloc error!");
    Font f = p->font;
    memset(p, 0, sizeof(Plugin));
    p->font = f;
    // memset(p->input.items, 0, sizeof(Plugin));
    read_input();

    TraceLog(LOG_INFO, "Plugin data flushed!");
}

Plugin* plugin_init(){
    TraceLog(LOG_INFO, "[PLUGIN] Plugin initialized!");
    flush_plugin();
    load_assets();
    return p;
}

Plugin* plugin_preload(){
    unload_assets();
    TraceLog(LOG_INFO, "[PLUGIN] Plugin preload run!");
    return p;
}

void plugin_postload(Plugin* plugin){
    TraceLog(LOG_INFO, "[PLUGIN] Plugin postload run!");
    p = plugin;
    load_assets();
}