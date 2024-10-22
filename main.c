#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "sprites.c"

typedef struct MovingElement
{
    int currentX;
    int currentY;
    int color;
    int dirX;
    int dirY;
    bool toDraw;
} Element;

#define PIXEL_BUF_CTRL_BASE 0xFF203020
#define PS_2_ADDR 0xFF200100

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define GREEN_REGION 0xA7F0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00
#define BLACK 0x0000

/* MOVING ELEMENT SIZE */
#define BOX_LEN_RED 9
#define BOX_LEN_BLUE 7
#define BOX_LEN_YELLOW 5

/* CONSTANTS */
#define MAX_ARRAY_LENGTH 100
#define NULL_INDICATOR 0

int LEVEL = 0;
int DRAWCOUNT = 0;
bool DRAWTOBACKBUFFERCOMPLETE = false;
bool FAILED = false;
bool PASSED = false;
int RESET_LEVEL = 0;
int COINS_REQUIRED = 0;

Element USER_BOX = {0, 0, RED, 0, 0, true};
Element LISTOFBLUEELEMENTS[MAX_ARRAY_LENGTH];
Element LISTOFYELLOWELEMENTS[MAX_ARRAY_LENGTH];

const uint16_t *AssembledStartPage = (uint16_t *)StartPage;
const uint16_t *AssembledFirstLevel = (uint16_t *)LevelOne;
const uint16_t *AssembledSecondLevel = (uint16_t *)LevelTwo;
const uint16_t *AssembledFailedLevel = (uint16_t *)FailedLevel;
const uint16_t *AssembledFinalScreen = (uint16_t *)FinalScreen;

void draw_final_screen(void);
void draw_failed_screen(void);
void draw_start_screen(void);
void plot_pixel(int x, int y, short int line_color);
void wait_for_vsync(void);
bool checkifInBounds(int dirX, int dirY);

volatile int pixel_buffer;

void draw_line(int x0, int x1, int y0, int y1, short int line_color);
void draw_level(void);
void draw_yellow_boxes(void);
void draw_blue_boxes(void);
void draw_level_one(void);
void draw_level_two(void);
void clear_array(Element List[]);
void undraw_box(int x, int y, int BOX_LEN, int speed);
void change_dir_level_one(Element *BoxToDraw);
void draw_box(int x, int y, int color, int BOX_LEN);
void CheckifLevelFailed(Element BlueBox);
void delay(int seconds);

int main(void)
{
    clear_array(LISTOFBLUEELEMENTS);
    clear_array(LISTOFYELLOWELEMENTS);
    
    volatile int *pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
    volatile int *PS2_ptr = (int *)PS_2_ADDR;

    LEVEL = 0;
    DRAWCOUNT = 0;
    DRAWTOBACKBUFFERCOMPLETE = false;
    FAILED = false;
    PASSED = false;
    RESET_LEVEL = 0;
    COINS_REQUIRED = 0;

    

    *(pixel_ctrl_ptr + 1) = 0xC8000000;

    wait_for_vsync();

    pixel_buffer = *pixel_ctrl_ptr;
    draw_start_screen();

    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer = *(pixel_ctrl_ptr + 1);
    draw_start_screen();

    int PS2_data, RVALID;
    while (1)
    {
        PS2_data = *(PS2_ptr)&0xFF;
        if(PS2_data == 0x16){
            LEVEL = 1;
            DRAWCOUNT = 0;
            DRAWTOBACKBUFFERCOMPLETE = false;
        }

        if(PS2_data == 0x1E){
            LEVEL = 2;
            DRAWCOUNT = 0;
            DRAWTOBACKBUFFERCOMPLETE = false;
        }

        if (PS2_data == 0x5A)
        {
            LEVEL++;
            DRAWCOUNT = 0;
            DRAWTOBACKBUFFERCOMPLETE = false;
        }


        draw_level();

        if (PASSED)
        {
            PASSED = false;
            LEVEL++;
            DRAWCOUNT = 0;
            DRAWTOBACKBUFFERCOMPLETE = false;
        }

        if (FAILED)
        {
            draw_failed_level();
            FAILED = false;
            DRAWCOUNT = 0;
            DRAWTOBACKBUFFERCOMPLETE = false;
        }

        if (DRAWCOUNT % 2 == 0 && DRAWCOUNT > 1)
        {
            // up
            if (PS2_data == 0x75 && LEVEL > 0 && checkifInBounds(0, -4))
            {
                undraw_box(USER_BOX.currentX - USER_BOX.dirX, USER_BOX.currentY - USER_BOX.dirY, BOX_LEN_RED, 4);
                USER_BOX.currentY = USER_BOX.currentY - 4;
                USER_BOX.dirX = 0;
                USER_BOX.dirY = -4;
                draw_box(USER_BOX.currentX, USER_BOX.currentY, RED, BOX_LEN_RED);
            }

            // left
            if (PS2_data == 0x6B && LEVEL > 0 && checkifInBounds(-4, 0))
            {
                undraw_box(USER_BOX.currentX - USER_BOX.dirX, USER_BOX.currentY - USER_BOX.dirY, BOX_LEN_RED, 4);
                USER_BOX.currentX = USER_BOX.currentX - 4;
                USER_BOX.dirX = -4;
                USER_BOX.dirY = 0;
                draw_box(USER_BOX.currentX, USER_BOX.currentY, RED, BOX_LEN_RED);
            }

            // down
            if (PS2_data == 0x72 && LEVEL > 0 && checkifInBounds(0, 4))
            {
                undraw_box(USER_BOX.currentX - USER_BOX.dirX, USER_BOX.currentY - USER_BOX.dirY, BOX_LEN_RED, 4);
                USER_BOX.currentY = USER_BOX.currentY + 4;
                USER_BOX.dirX = 0;
                USER_BOX.dirY = 4;
                draw_box(USER_BOX.currentX, USER_BOX.currentY, RED, BOX_LEN_RED);
            }

            //  right
            if (PS2_data == 0x74 && LEVEL > 0 && checkifInBounds(4, 0))
            {
                undraw_box(USER_BOX.currentX - USER_BOX.dirX, USER_BOX.currentY - USER_BOX.dirY, BOX_LEN_RED, 4);
                USER_BOX.currentX = USER_BOX.currentX + 4;
                USER_BOX.dirX = 4;
                USER_BOX.dirY = 0;
                draw_box(USER_BOX.currentX, USER_BOX.currentY, RED, BOX_LEN_RED);
            }
        }
        else
        {
            if (LEVEL > 0 && RESET_LEVEL == 0)
            {
                undraw_box(USER_BOX.currentX - USER_BOX.dirX, USER_BOX.currentY - USER_BOX.dirY, BOX_LEN_RED, 4);
                draw_box(USER_BOX.currentX, USER_BOX.currentY, RED, BOX_LEN_RED);
            }
        }

        wait_for_vsync();                     // swap front and back buffers on VGA vertical sync
        pixel_buffer = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer + (y << 10) + (x << 1)) = line_color;
}

void draw_start_screen()
{
    for (int y = 0; y < RESOLUTION_Y; y++)
    {
        for (int x = 0; x < RESOLUTION_X; x++)
        {
            plot_pixel(x, y, AssembledStartPage[x + y * RESOLUTION_X]);
        }
    }
}

void draw_level()
{
    if (LEVEL == 0)
    {
        draw_start_screen();
    }
    if (LEVEL == 1)
    {
        draw_level_one();
    }
    if (LEVEL == 2){
        draw_level_two();
    }

    if(LEVEL > 2){
        
        draw_start_screen();
        main();
    }
}

void draw_level_one()
{

    if (!DRAWTOBACKBUFFERCOMPLETE || RESET_LEVEL == 1)
    {
        for (int y = 0; y < RESOLUTION_Y; y++)
        {
            for (int x = 0; x < RESOLUTION_X; x++)
            {
                plot_pixel(x, y, AssembledFirstLevel[x + y * RESOLUTION_X]);
            }
        }

        clear_array(LISTOFBLUEELEMENTS);
        clear_array(LISTOFYELLOWELEMENTS);
        USER_BOX.currentX = 60;
        USER_BOX.currentY = 120;

        draw_box(USER_BOX.currentX, USER_BOX.currentY, RED, BOX_LEN_RED);
        LISTOFBLUEELEMENTS[0] = (Element){112, 97, BLUE, 4, 0, true};
        LISTOFBLUEELEMENTS[1] = (Element){213, 113, BLUE, 4, 0, true};
        LISTOFBLUEELEMENTS[2] = (Element){112, 131, BLUE, 4, 0, true};
        LISTOFBLUEELEMENTS[3] = (Element){213, 149, BLUE, 4, 0, true};
        LISTOFBLUEELEMENTS[4] = (Element){112, 165, BLUE, 4, 0, true};

        if (RESET_LEVEL)
        {
            DRAWCOUNT = 0;
            DRAWTOBACKBUFFERCOMPLETE = false;
        }
        RESET_LEVEL = 0;
    }

    if (USER_BOX.currentX > 230)
    {
        PASSED = true;
        return;
    }

    if (DRAWCOUNT == 2)
    {
        DRAWTOBACKBUFFERCOMPLETE = true;
    }

    DRAWCOUNT++;

    int index = 0;

    if (DRAWCOUNT % 2 == 0 && DRAWCOUNT > 1)
    {
        while (LISTOFBLUEELEMENTS[index].color != NULL_INDICATOR)
        {
            undraw_box(LISTOFBLUEELEMENTS[index].currentX - LISTOFBLUEELEMENTS[index].dirX, LISTOFBLUEELEMENTS[index].currentY - LISTOFBLUEELEMENTS[index].dirY, BOX_LEN_BLUE, 4);
            change_dir_level_one(&LISTOFBLUEELEMENTS[index]);
            LISTOFBLUEELEMENTS[index].currentX = LISTOFBLUEELEMENTS[index].currentX + LISTOFBLUEELEMENTS[index].dirX;
            index++;
        }
    }
    else
    {
        while (LISTOFBLUEELEMENTS[index].color != NULL_INDICATOR)
        {
            undraw_box(LISTOFBLUEELEMENTS[index].currentX - LISTOFBLUEELEMENTS[index].dirX, LISTOFBLUEELEMENTS[index].currentY - LISTOFBLUEELEMENTS[index].dirY, BOX_LEN_BLUE, 4);
            index++;
        }
    }
    draw_yellow_boxes();
    draw_blue_boxes();

    index = 0;
    while (LISTOFBLUEELEMENTS[index].color != NULL_INDICATOR)
    {
        CheckifLevelFailed(LISTOFBLUEELEMENTS[index]);
        index++;
    }
}

void draw_failed_level(){
    for (int y = 0; y < RESOLUTION_Y; y++)
        {
            for (int x = 0; x < RESOLUTION_X; x++)
            {
                plot_pixel(x, y, AssembledFailedLevel[x + y * RESOLUTION_X]);
            }
        }
}



void draw_level_two(){
    if (!DRAWTOBACKBUFFERCOMPLETE || RESET_LEVEL == 2)
    {
        for (int y = 0; y < RESOLUTION_Y; y++)
        {
            for (int x = 0; x < RESOLUTION_X; x++)
            {
                plot_pixel(x, y, AssembledSecondLevel[x + y * RESOLUTION_X]);
            }

            clear_array(LISTOFBLUEELEMENTS);
            clear_array(LISTOFYELLOWELEMENTS);
            USER_BOX.currentX = 40;
            USER_BOX.currentY = 130;

            draw_box(USER_BOX.currentX, USER_BOX.currentY, RED, BOX_LEN_RED);
            
            COINS_REQUIRED = 4;
            LISTOFBLUEELEMENTS[0] = (Element){93, 180, BLUE, 0, 4, true};
            LISTOFBLUEELEMENTS[1] = (Element){106, 88, BLUE, 0, 4, true};
            LISTOFBLUEELEMENTS[2] = (Element){119, 180, BLUE, 0, 4, true};
            LISTOFBLUEELEMENTS[3] = (Element){132, 88, BLUE, 0, 4, true};
            LISTOFBLUEELEMENTS[4] = (Element){145, 180, BLUE, 0, 4, true};
            LISTOFBLUEELEMENTS[5] = (Element){158, 88, BLUE, 0, 4, true};
            LISTOFBLUEELEMENTS[6] = (Element){171, 180, BLUE, 0, 4, true};
            LISTOFBLUEELEMENTS[7] = (Element){184, 88, BLUE, 0, 4, true};
            LISTOFBLUEELEMENTS[8] = (Element){197, 180, BLUE, 0, 4, true};
            LISTOFBLUEELEMENTS[9] = (Element){210, 88, BLUE, 0, 4, true};
            LISTOFBLUEELEMENTS[10] = (Element){223, 180, BLUE, 0, 4, true};

            LISTOFYELLOWELEMENTS[0] = (Element){160, 126, YELLOW, 0, 0, true};
            LISTOFYELLOWELEMENTS[1] = (Element){154, 132, YELLOW, 0, 0, true};
            LISTOFYELLOWELEMENTS[2] = (Element){154, 126, YELLOW, 0, 0, true};
            LISTOFYELLOWELEMENTS[3] = (Element){160, 132, YELLOW, 0, 0, true};

              if (RESET_LEVEL == 2)
                {
                    DRAWCOUNT = 0;
                    DRAWTOBACKBUFFERCOMPLETE = false;
                }
                RESET_LEVEL = 0;
        }
    }

    if (USER_BOX.currentX > 245 && COINS_REQUIRED == 0)
    {
        PASSED = true;
        return;
    }

    if (DRAWCOUNT == 2)
    {
        DRAWTOBACKBUFFERCOMPLETE = true;
    }

    DRAWCOUNT++;

  
    int index = 0;
    if (DRAWCOUNT % 2 == 0 && DRAWCOUNT > 1)
    {
        while (LISTOFBLUEELEMENTS[index].color != NULL_INDICATOR)
        {
            undraw_box(LISTOFBLUEELEMENTS[index].currentX - LISTOFBLUEELEMENTS[index].dirX, LISTOFBLUEELEMENTS[index].currentY - LISTOFBLUEELEMENTS[index].dirY, BOX_LEN_BLUE, 4);
            change_dir_level_two(&LISTOFBLUEELEMENTS[index]);
            LISTOFBLUEELEMENTS[index].currentY = LISTOFBLUEELEMENTS[index].currentY + LISTOFBLUEELEMENTS[index].dirY;
            index++;
        }
    }
    else
    {
        while (LISTOFBLUEELEMENTS[index].color != NULL_INDICATOR)
        {
            undraw_box(LISTOFBLUEELEMENTS[index].currentX - LISTOFBLUEELEMENTS[index].dirX, LISTOFBLUEELEMENTS[index].currentY - LISTOFBLUEELEMENTS[index].dirY, BOX_LEN_BLUE, 4);
            index++;
        }
    }
    index = 0;
    if (DRAWCOUNT  % 2 == 0 && DRAWCOUNT > 1)
    {
        while (LISTOFYELLOWELEMENTS[index].color != NULL_INDICATOR)
        {
            CheckifElementCollected(&LISTOFYELLOWELEMENTS[index]);
            index++;
        }
    }
    else
    {
        while (LISTOFYELLOWELEMENTS[index].color != NULL_INDICATOR)
        {
            CheckifElementCollected(&LISTOFYELLOWELEMENTS[index]);
            index++;
        }
    }
    draw_yellow_boxes();
    draw_blue_boxes();

    index = 0;
    while (LISTOFBLUEELEMENTS[index].color != NULL_INDICATOR)
    {
        CheckifLevelFailed(LISTOFBLUEELEMENTS[index]);
        index++;
    }
}

void change_dir_level_one(Element *BoxToDraw)
{
    if (BoxToDraw->currentX + (BOX_LEN_BLUE - 1) / 2 > 213)
    {
        BoxToDraw->dirX = -4;
    }
    if (BoxToDraw->currentX + (BOX_LEN_BLUE - 1) / 2 < 112)
    {
        BoxToDraw->dirX = 4;
    }
}

void change_dir_level_two(Element *BoxToDraw)
{
    if (BoxToDraw->currentY + (BOX_LEN_BLUE - 1) / 2 < 80)
    {
        BoxToDraw->dirY = 4;
    }
    if (BoxToDraw->currentY + (BOX_LEN_BLUE - 1) / 2 > 188)
    {
        BoxToDraw->dirY = -4;
    }
}

void CheckifLevelFailed(Element BlueBox)
{
    for (int i1 = -(BOX_LEN_RED - 1) / 2; i1 <= (BOX_LEN_RED - 1) / 2; i1++)
    {
        for (int j1 = -(BOX_LEN_RED - 1) / 2; j1 <= (BOX_LEN_RED - 1) / 2; j1++)
        {
            for (int i2 = -(BOX_LEN_BLUE - 1) / 2; i2 <= (BOX_LEN_BLUE - 1) / 2; i2++)
            {
                for (int j2 = -(BOX_LEN_BLUE - 1) / 2; j2 <= (BOX_LEN_BLUE - 1) / 2; j2++)
                {
                    if (USER_BOX.currentX + i1 == BlueBox.currentX + i2 && USER_BOX.currentY + j1 == BlueBox.currentY + j2)
                    {
                        FAILED = true;
                        if(LEVEL == 1) RESET_LEVEL = 1;
                        if(LEVEL == 2) RESET_LEVEL = 2;
                    }
                }
            }
        }
    }
}

void CheckifElementCollected(Element *YellowBox)
{
    for (int i1 = -(BOX_LEN_RED - 1) / 2; i1 <= (BOX_LEN_RED - 1) / 2; i1++)
    {
        for (int j1 = -(BOX_LEN_RED - 1) / 2; j1 <= (BOX_LEN_RED - 1) / 2; j1++)
        {
            for (int i2 = -(BOX_LEN_YELLOW - 1) / 2; i2 <= (BOX_LEN_BLUE - 1) / 2; i2++)
            {
                for (int j2 = -(BOX_LEN_YELLOW - 1) / 2; j2 <= (BOX_LEN_BLUE - 1) / 2; j2++)
                {
                    if (USER_BOX.currentX + i1 == YellowBox->currentX + i2 && USER_BOX.currentY + j1 == YellowBox->currentY + j2)
                    {
                        
                        if(YellowBox->toDraw){
                            YellowBox->toDraw = false;
                            COINS_REQUIRED--;
                        }
                    }
                }
            }
        }
    }
}

void draw_box(int x, int y, int color, int BOX_LEN)
{
    for (int i = -(BOX_LEN - 1) / 2; i <= (BOX_LEN - 1) / 2; i++)
    {
        for (int j = -(BOX_LEN - 1) / 2; j <= (BOX_LEN - 1) / 2; j++)
        {
            if (i == (BOX_LEN - 1) / 2 || j == (BOX_LEN - 1) / 2 || i == -(BOX_LEN - 1) / 2 || j == -(BOX_LEN - 1) / 2)
            {
                plot_pixel(x + i, y + j, BLACK);
            }
            else
            {
                plot_pixel(x + i, y + j, color);
            }
        }
    }
}

void undraw_box(int x, int y, int BOX_LEN, int speed)
{

    for (int i = -(BOX_LEN - 1) / 2; i <= (BOX_LEN - 1) / 2; i++)
    {
        for (int j = -(BOX_LEN - 1) / 2; j <= (BOX_LEN - 1) / 2; j++)
        {
            if (x < 84 && LEVEL == 1)
            {
                plot_pixel(x + i, y + j, GREEN_REGION);
            }
            else if (x < 63 && LEVEL == 2)
            {
                plot_pixel(x + i, y + j, GREEN_REGION);
            }
            else
            {
                plot_pixel(x + i, y + j, WHITE);
            }
        }
    }
}

void draw_yellow_boxes()
{
    int index = 0;
    while (LISTOFYELLOWELEMENTS[index].color != NULL_INDICATOR)
    {
        Element CurrentBox = LISTOFYELLOWELEMENTS[index];
        if(CurrentBox.toDraw){
            draw_box(CurrentBox.currentX, CurrentBox.currentY, CurrentBox.color, BOX_LEN_YELLOW);
        }
        else{
            undraw_box(CurrentBox.currentX, CurrentBox.currentY, BOX_LEN_YELLOW,0);
        }
        index++;
    }
}

void draw_blue_boxes()
{
    int index = 0;
    while (LISTOFBLUEELEMENTS[index].color != NULL_INDICATOR)
    {
        Element CurrentBox = LISTOFBLUEELEMENTS[index];

        draw_box(CurrentBox.currentX, CurrentBox.currentY, CurrentBox.color, BOX_LEN_BLUE);
        index++;
    }
}

void wait_for_vsync(void)
{
    volatile int *pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
    register int status;

    *pixel_ctrl_ptr = 1;
    status = *(pixel_ctrl_ptr + 3);

    while ((status & 0x01) != 0)
    {
        status = *(pixel_ctrl_ptr + 3);
    }
}

void clear_array(Element List[])
{
    for (int i = 0; i < MAX_ARRAY_LENGTH; i++)
    {
        List[i] = (Element){0, 0, NULL_INDICATOR, 0, 0};
    }
}

bool checkifInBounds(int directionX, int directionY)
{
    if (LEVEL == 1)
    {
        // left, up, down, right
        if (USER_BOX.currentX - (BOX_LEN_RED - 1) / 2 + directionX > 32 && USER_BOX.currentY - (BOX_LEN_RED - 1) / 2 + directionY > 71 &&
            (USER_BOX.currentY + (BOX_LEN_RED - 1) / 2 + directionY < 192) && USER_BOX.currentX + (BOX_LEN_RED - 1) / 2 + directionX < 80)
        {
            return true;
        }
        if (USER_BOX.currentX - (BOX_LEN_RED - 1) / 2 + directionX >= 71 && USER_BOX.currentY - (BOX_LEN_RED - 1) / 2 + directionY > 175 &&
            (USER_BOX.currentY + (BOX_LEN_RED - 1) / 2 + directionY < 192) && USER_BOX.currentX + (BOX_LEN_RED - 1) / 2 + directionX < 112)
        {
            return true;
        }
        if (USER_BOX.currentX - (BOX_LEN_RED - 1) / 2 + directionX >= 98 && USER_BOX.currentY - (BOX_LEN_RED - 1) / 2 + directionY > 87 &&
            (USER_BOX.currentY + (BOX_LEN_RED - 1) / 2 + directionY < 192) && USER_BOX.currentX + (BOX_LEN_RED - 1) / 2 + directionX < 112)
        {
            return true;
        }
        if (USER_BOX.currentX - (BOX_LEN_RED - 1) / 2 + directionX >= 98 && USER_BOX.currentY - (BOX_LEN_RED - 1) / 2 + directionY > 87 &&
            (USER_BOX.currentY + (BOX_LEN_RED - 1) / 2 + directionY < 173) && USER_BOX.currentX + (BOX_LEN_RED - 1) / 2 + directionX < 223)
        {
            return true;
        }
        if (USER_BOX.currentX - (BOX_LEN_RED - 1) / 2 + directionX >= 210 && USER_BOX.currentY - (BOX_LEN_RED - 1) / 2 + directionY > 71 &&
            (USER_BOX.currentY + (BOX_LEN_RED - 1) / 2 + directionY < 173) && USER_BOX.currentX + (BOX_LEN_RED - 1) / 2 + directionX < 223)
        {
            return true;
        }
        if (USER_BOX.currentX - (BOX_LEN_RED - 1) / 2 + directionX >= 210 && USER_BOX.currentY - (BOX_LEN_RED - 1) / 2 + directionY > 71 &&
            (USER_BOX.currentY + (BOX_LEN_RED - 1) / 2 + directionY < 88) && USER_BOX.currentX + (BOX_LEN_RED - 1) / 2 + directionX < 260)
        {
            return true;
        }
    }

    if(LEVEL == 2){
        if (USER_BOX.currentX - (BOX_LEN_RED - 1) / 2 + directionX >= 32 && USER_BOX.currentY - (BOX_LEN_RED - 1) / 2 + directionY > 108 &&
            (USER_BOX.currentY + (BOX_LEN_RED - 1) / 2 + directionY < 156) && USER_BOX.currentX + (BOX_LEN_RED - 1) / 2 + directionX < 63)
        {
            return true;
        }
        if (USER_BOX.currentX - (BOX_LEN_RED - 1) / 2 + directionX >= 45 && USER_BOX.currentY - (BOX_LEN_RED - 1) / 2 + directionY > 125 &&
            (USER_BOX.currentY + (BOX_LEN_RED - 1) / 2 + directionY < 139) && USER_BOX.currentX + (BOX_LEN_RED - 1) / 2 + directionX < 266)
        {
            return true;
        }
        if (USER_BOX.currentX - (BOX_LEN_RED - 1) / 2 + directionX >= 81 && USER_BOX.currentY - (BOX_LEN_RED - 1) / 2 + directionY > 72 &&
            (USER_BOX.currentY + (BOX_LEN_RED - 1) / 2 + directionY < 192) && USER_BOX.currentX + (BOX_LEN_RED - 1) / 2 + directionX < 239)
        {
            return true;
        }
    }
    return false;
}

void delay(int seconds){
    clock_t startTime = clock();
    while(clock() < startTime + 1000 * seconds){
        continue;
    }
}