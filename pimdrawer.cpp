#include <locale.h>
#include <ncurses.h>
#include <vector>
#include <string>

const int CANVAS_W = 80;
const int CANVAS_H = 40;
const int VIEW_W = 40;
const int VIEW_H = 20;
const int PALETTE_H = 2;
const int PREVIEW_X = VIEW_W + 5;

struct Color { char key; int pair; std::string name; };

int getColorIndex(char c){
    switch(c){
        case 'n': return 1;
        case 'b': return 5;
        case 'g': return 3;
        case 'c': return 7;
        case 'r': return 2;
        case 'm': return 6;
        case 'y': return 4;
        case 'w': return 8;
        default: return 1;
    }
}

void initCombinedPairs(){
    int pair=20;
    for(int fg=1; fg<=8; fg++){
        for(int bg=1; bg<=8; bg++){
            init_pair(pair++, fg-1, bg-1);
        }
    }
}

int getCombinedPair(int fg,int bg){
    return 20 + (fg-1)*8 + (bg-1);
}

int main(){
    setlocale(LC_ALL,"");
    initscr();
    cbreak(); noecho();
    keypad(stdscr,TRUE);
    curs_set(1);
    start_color();

    // Colores básicos
    init_pair(1, COLOR_BLACK,   COLOR_BLACK);   // n
    init_pair(5, COLOR_BLUE,    COLOR_BLACK);   // b
    init_pair(3, COLOR_GREEN,   COLOR_BLACK);   // g
    init_pair(7, COLOR_CYAN,    COLOR_BLACK);   // c
    init_pair(2, COLOR_RED,     COLOR_BLACK);   // r
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);   // m
    init_pair(4, COLOR_YELLOW,  COLOR_BLACK);   // y
    init_pair(8, COLOR_WHITE,   COLOR_BLACK);   // w

    initCombinedPairs();

    std::vector<Color> palette = {
        {'n',1,"Negro"},
        {'b',5,"Azul"},
        {'g',3,"Verde"},
        {'c',7,"Cian"},
        {'r',2,"Rojo"},
        {'m',6,"Magenta"},
        {'y',4,"Amarillo"},
        {'w',8,"Blanco"}
    };

    std::vector<std::vector<char>> canvas(CANVAS_H,std::vector<char>(CANVAS_W,'.'));
    int curX=0, curY=0, selected=0;
    int offsetX=0, offsetY=0;
    bool running=true;

    auto drawCanvas=[&](){
        for(int y=0;y<VIEW_H;y++){
            for(int x=0;x<VIEW_W;x++){
                int cx=x+offsetX;
                int cy=y+offsetY;
                if(cx>=CANVAS_W || cy>=CANVAS_H) continue;
                int pair = getColorIndex(canvas[cy][cx]);
                attron(COLOR_PAIR(pair));
                mvaddch(y,x,canvas[cy][cx]);
                attroff(COLOR_PAIR(pair));
            }
        }
    };

    auto drawPalette=[&](){
        for(size_t i=0;i<palette.size();i++){
            int x=i*4, y=VIEW_H+1;
            if((int)i==selected) attron(A_REVERSE);
            attron(COLOR_PAIR(palette[i].pair));
            mvprintw(y,x," %c ",palette[i].key);
            attroff(COLOR_PAIR(palette[i].pair));
            if((int)i==selected) attroff(A_REVERSE);
        }
        move(VIEW_H+PALETTE_H,0); clrtoeol();
        attron(COLOR_PAIR(palette[selected].pair)|A_BOLD);
        mvprintw(VIEW_H+PALETTE_H,0,"  ");
        attroff(COLOR_PAIR(palette[selected].pair)|A_BOLD);
        mvprintw(VIEW_H+PALETTE_H,3,"%s",palette[selected].name.c_str());
    };

    auto drawPreview=[&](){
        int py=0;
        for(int y=0;y<VIEW_H;y+=2){
            int px=PREVIEW_X;
            for(int x=0;x<VIEW_W;x++){
                int cx=x+offsetX;
                int cy=y+offsetY;
                if(cx>=CANVAS_W || cy>=CANVAS_H) continue;
                char top=canvas[cy][cx];
                char bottom=(cy+1<CANVAS_H)? canvas[cy+1][cx]:'n';
                if(bottom=='.') bottom='n';
                int fg=getColorIndex(top);
                int bg=getColorIndex(bottom);
                int pair=getCombinedPair(fg,bg);
                attron(COLOR_PAIR(pair));
                wchar_t up[2] = {L'\u2580', L'\0'};
                mvaddwstr(py,px,up);
                attroff(COLOR_PAIR(pair));
                px++;
            }
            py++;
        }
    };

    drawCanvas(); drawPalette(); drawPreview();
    move(curY-offsetY,curX-offsetX);
    refresh();

    while(running){
        int ch=getch();
        switch(ch){
            case KEY_UP: if(curY>0) curY--; break;
            case KEY_DOWN: if(curY<CANVAS_H-1) curY++; break;
            case KEY_LEFT: if(curX>0) curX--; break;
            case KEY_RIGHT: if(curX<CANVAS_W-1) curX++; break;
            case '\t': selected=(selected+1)%palette.size(); break;
            case ' ': case 10: canvas[curY][curX]=palette[selected].key; break;
            case 'd': canvas[curY][curX]='.'; break;
            case 'q': running=false; break;
            default:
                for(size_t i=0;i<palette.size();i++)
                    if(ch==palette[i].key) selected=i;
                break;
        }

        // Ajustar scroll
        if(curX<offsetX) offsetX=curX;
        if(curX>=offsetX+VIEW_W) offsetX=curX-VIEW_W+1;
        if(curY<offsetY) offsetY=curY;
        if(curY>=offsetY+VIEW_H) offsetY=curY-VIEW_H+1;

        drawCanvas(); drawPalette(); drawPreview();
        move(curY-offsetY,curX-offsetX);
        refresh();
    }

    endwin();
    return 0;
}

