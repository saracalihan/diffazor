#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "raylib.h"
#include "plugin.h"
#include "algorithm.c"

#define FONT_SIZE        18
#define INPUT_FILE1_PATH "./input1.txt"
#define INPUT_FILE2_PATH "./input2.txt"
#define FONT_PATH        "./resources/LiberationMono-Regular.ttf"
#define SCROLL_SENSIVITY 3
#define LINE_SPACING     2

static Plugin* p = NULL;
int SCREEN_HEIGHT = 0,
    SCREEN_WIDTH =0,
    startLineNumber = 0,
    lineHeight=0,
    screenLineCount = 0,
    // Sidebar
    sidebarWidth=16;

int scrollValue =0, scrollWidth = 7;
int total_addition =0, total_deletion =0;

bool isFileEnd = false;

Color editorBg    = {17, 18, 18, 255},
      editorText  = WHITE,
      sidebarBg   = {40, 41, 41, 255},
      sidebarText = GRAY,
      scrollBg = DARKGRAY,
      scrollHover = GRAY,
      scrollActive = LIGHTGRAY;

 Vector2 fontSize;

void draw_text(char* text, int x, int y, Color color){
    DrawTextEx(p->font, text, (Vector2){ .x=x, .y=y}, FONT_SIZE, 0, color);
}

void draw_text_ex(char* text, int x, int y, int size, Color color){
    DrawTextEx(p->font, text, (Vector2){ .x=x, .y=y}, size, 0, color);
}



void read_input(TextVector *input, const char* file_path){
    int size = 0;

    FILE *f = fopen(file_path, "r");
    if(f == NULL){
        TraceLog(LOG_ERROR, "%s file can not open! %m", file_path);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if(input->items !=NULL){
        free(input->items);
        input->items = NULL;
    }
    input->items = malloc(sizeof(TextDiff) * size);
    memset(input->items, 0, sizeof(TextDiff)*size);
    input->count = size;
    input->line_count=1;
    for(int i =0; i<size; i++){
        char c = getc(f);
        input->items[i].c = c;
        input->items[i].diff = SAME;
        if(c == '\n'){
           input->line_count++;
        }
    }

    fclose(f);
    // inputs_diff = p->input.items;
}

void handle_scroll(){

    static int last_line_count = 0;
    static int last_pos = 0;

    int row_count = SCREEN_HEIGHT / fontSize.y;
    int column_count = SCREEN_WIDTH / fontSize.x;
    int total_char_count = row_count*column_count;
    int fontHeight = fontSize.y;
    int fontWidth = fontSize.x;
    int largestLineLetterCount = 0;
    int mouseScrollY = (int)GetMouseWheelMove();

    if(last_line_count == 0){
        last_line_count = p->inputs_diff.count < total_char_count ? p->inputs_diff.line_count : screenLineCount ;
    }

    // You are on the end of file and scroll down
    if(mouseScrollY<0 && last_line_count > p->inputs_diff.line_count){
        return;
    }
    //You are on the start of file and scroll up
    if(mouseScrollY>0 && last_line_count <= screenLineCount){
        return;
    }

    lineHeight = fontHeight + LINE_SPACING;
    screenLineCount = SCREEN_HEIGHT / (float)(FONT_SIZE-3);
    scrollValue += mouseScrollY * SCROLL_SENSIVITY;

    // If user draw the scroll bar, start line setted in code and dont want to change
    if(mouseScrollY !=0){
        startLineNumber = (scrollValue < 0 ? abs(scrollValue) : 0);
    }

    int curPos =0,
        letterCount=0;
    for(int i=0; i < abs(mouseScrollY)*SCROLL_SENSIVITY; i++){
        // scroll down
        if(mouseScrollY<0){
            while(p->inputs_diff.items[last_pos + curPos].c != '\n'){
                curPos++;
            }
            curPos++;
            last_line_count++;

        } else { // scroll up
            while(p->inputs_diff.items[last_pos - curPos].c != '\n'){
                curPos--;
            }
            curPos--;
            last_line_count--;
        }
    }

    // TODO: tüm string görünmese bile ekrana yazılıyor. Burada gerçekten kesilme yapılmalı.inputs_diff için (satır satısı)*(ekran genişliği / font gelişliği) lkadadr yer al ve  texti oraya ata
    // Set editor text
    last_pos += curPos;
    p->virtualized_text.items = p->inputs_diff.items + last_pos;
    p->virtualized_text.count = p->inputs_diff.count - last_pos;


    // Draw horizental scroll
    if(p->inputs_diff.line_count > screenLineCount){
        Rectangle bar = {
            .x      = SCREEN_WIDTH - scrollWidth,
            .y      = SCREEN_HEIGHT * ( (float)startLineNumber / p->inputs_diff.line_count),
            .width  = scrollWidth,
            .height = SCREEN_HEIGHT * ((float)screenLineCount / p->inputs_diff.line_count) ,
        };
        Vector2 m = GetMousePosition();
        Color c = scrollBg;

        // Hover
        if(CheckCollisionPointRec(m, bar)){
            c = scrollHover;

            // Click
            if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
                c = scrollActive;
                int bar_center = m.y;
                startLineNumber = (p->inputs_diff.line_count*bar_center) / SCREEN_HEIGHT;
            }
        }
        DrawRectangleRec(bar, c);
    }

    // Draw vertical scroll
    if(largestLineLetterCount * (fontWidth + LINE_SPACING) > SCREEN_WIDTH ){
        DrawRectangle(
            0,
            SCREEN_HEIGHT,
            50,
            scrollWidth,
            RED
        );
    }
}

void draw_texts(){
    if(p->virtualized_text.items == NULL || p->virtualized_text.count ==0){
        return;
    }
     Vector2 font = MeasureTextEx(p->font, "A", FONT_SIZE, 0);
        int column=0, row=0;
        bool is_newline = false;
        for(int i =0; i< p->virtualized_text.count; i++){
            Color c;
            switch(p->virtualized_text.items[i].diff){
                case ADD:
                    c= DARKGREEN;
                    break;
                case DELETE:
                    c= MAROON;
                    break;
                case SAME:
                    c= editorBg;
                    break;
            }
            char t[3] ={0};
            is_newline = p->virtualized_text.items[i].c == '\n';

            // If it is a new line character go to next line
            if(is_newline){
                column=0;
                row++;
            }
            int x = 30 + column*(font.x),
                y = row * (font.y-3);

            DrawRectangle(x+1,y, font.x, font.y, c);

            // New line character doesnt need to draw
            if(!is_newline) {
                sprintf(&t, "%c",p->virtualized_text.items[i].c);
                DrawTextEx(
                    p->font,
                    t,
                    (Vector2){ .x=x, .y=y},
                    FONT_SIZE,
                    0,
                    WHITE
                );
                column++;
            }
        }
}

void flush_plugin();

void plugin_task(){
    SCREEN_HEIGHT = GetScreenHeight();
    SCREEN_WIDTH = GetScreenWidth();
        ClearBackground(editorBg);
        if(IsKeyPressed(KEY_F)){
            flush_plugin();
        }

        #pragma region draw_text
        draw_texts();
        #pragma endregion

        #pragma region draw_scrolls
        // Draw scroll sticks
        handle_scroll();
        // End of draw scroll sticks
        #pragma endregion

        #pragma region draw_sidebar
        // Draw line numbers
        DrawRectangle(
            0,
            0,
            sidebarWidth+6,
            SCREEN_HEIGHT,
            sidebarBg
        );
        // End of draw line numbers

        // Draw sidebar
        screenLineCount = SCREEN_HEIGHT / MeasureTextEx(p->font, "A", FONT_SIZE, 0).y;
        int num_count = p->inputs_diff.line_count < screenLineCount ? p->inputs_diff.line_count : screenLineCount;
        char str[num_count*3];
        memset(str, 0, num_count*2);
        for(int i=0; i< num_count+5; i++){
            sprintf(str, "%s%d\n",str, startLineNumber + i + 1);
        }
        draw_text(
            str,
            0,
            0,
            sidebarText
        );
        //End of draw sidebar
        #pragma endregion

        // Bottom menu
        {
            int y = SCREEN_HEIGHT - sidebarWidth;
            // Draw border
            DrawRectangle(
                0,
                y-2,
                SCREEN_WIDTH,
                2,
                VIOLET
            );

            // Draw background
            DrawRectangle(
                0,
                y,
                SCREEN_WIDTH,
                sidebarWidth +2,
                (Color){11, 11, 11, 255}
            );

            // padding for texts
            y+=1;
            char str[128];
            int text_font_size = FONT_SIZE*0.9;
            int text_start_pos = fontSize.x *3;
            sprintf(str, "%s -> %s", INPUT_FILE1_PATH, INPUT_FILE2_PATH);
            DrawTextEx(
                p->font,
                str,
                (Vector2){text_start_pos, y},
                text_font_size,
                0,
                WHITE
            );

            sprintf(str, "%i+ %i+ %i changes", total_addition, total_deletion, total_addition+total_deletion );
            Vector2 pos = {
                SCREEN_WIDTH - text_start_pos - MeasureTextEx(p->font, str, text_font_size, 0).x,
                y
            };
            sprintf(str, "%i+ ", total_addition);
            pos.x += MeasureTextEx(p->font, str, text_font_size, 0).x;
            DrawTextEx(
                p->font,
                str,
                pos,
                text_font_size,
                0,
                GREEN
            );

            sprintf(str, "%i- ", total_deletion);
            pos.x += MeasureTextEx(p->font, str, text_font_size, 0).x;
            DrawTextEx(
                p->font,
                str,
                pos,
                text_font_size,
                0,
                RED
            );
            pos.x += MeasureTextEx(p->font, str, text_font_size, 0).x;
            sprintf(str, "%i changes", total_deletion + total_addition);
            DrawTextEx(
                p->font,
                str,
                pos,
                text_font_size,
                0,
                WHITE
            );
        }

        // Bottom bar

}

void load_assets(){
    if(p == NULL){
        TraceLog(LOG_ERROR, "[LOAD ASSET] Plugin is null!");
        return exit(1);
    }
    Font f = LoadFontEx(FONT_PATH, 32, NULL, 250);
    memcpy(&p->font, &f, sizeof(f));
    if (p->font.texture.id == 0) {
        return TraceLog(LOG_ERROR, "[LOAD ASSET] %s font can not loading!", FONT_PATH);
    }
    fontSize = MeasureTextEx(f, "A",f.baseSize, 0);
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
        assert(p != NULL && "Plugin malloc error!");
        memset(p, 0, sizeof(Plugin));
    }

    Font f = p->font;
    if(p->input1.items != NULL)
        free(p->input1.items);
    if(p->input2.items != NULL)
        free(p->input2.items);
    if(p->inputs_diff.items != NULL)
        free(p->inputs_diff.items);
    memset(p, 0, sizeof(Plugin));
    p->font = f;

    read_input(&p->input1, INPUT_FILE1_PATH);
    read_input(&p->input2, INPUT_FILE2_PATH);
    p->inputs_diff= lcs(&p->input1, &p->input2);

    // Count total addition and deletion
    total_addition=0;
    total_deletion=0;
    for(int i=0; i<p->inputs_diff.count; i++){
        if(p->inputs_diff.items[i].diff == ADD)
            total_addition++;

        if(p->inputs_diff.items[i].diff == DELETE)
            total_deletion++;

    }

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