/* This files provides address values that exist in the system */

#define SDRAM_BASE            0xC0000000
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_CHAR_BASE        0xC9000000

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030

/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00

#define ABS(x) (((x) > 0) ? (x) : -(x))

/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

#define FALSE 0
#define TRUE 1

// Begin part3.c code for Lab 7


volatile int pixel_buffer_start; // global variable

void clear_screen();
void wait_for_vsync();
void draw_box(int x, int y, short int color);
void change_dir(int x, int y, int* dirX, int* dirY);

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    int next;
    int x_old1, y_old1, dirX_old1, dirY_old1;
    int x_old2, y_old2, dirX_old2, dirY_old2;
	int x_init[NUM_BOXES], y_init[NUM_BOXES], dirX_init[NUM_BOXES], dirY_init[NUM_BOXES], dir[NUM_BOXES]/*dir comes in 8 values, 1being down-left, and the numbers going anticlockwise from thier*/ ; //Need to fill in these boxes to plot initial points
	int scale[NUM_BOXES];
		int count = 1;
    // initialize location and direction of rectangles(not shown)
    int box_X[NUM_BOXES], box_Y[NUM_BOXES], dirX[NUM_BOXES], dirY[NUM_BOXES];
    short int color[10] = {WHITE, YELLOW, RED, GREEN, BLUE, CYAN, MAGENTA, GREY, PINK, ORANGE};
    short int box_color[NUM_BOXES];
    for(int i=0;i<NUM_BOXES;i++){
        box_X[i] = x_init[i] % (RESOLUTION_X-1);
        box_Y[i] = y_init[i] % (RESOLUTION_Y-1);
        dirX[i] = dirX_init[i];
        dirY[i] = dirY_init[i];
        box_color[i] = BLUE;
        change_dir(box_X[i],box_Y[i],&dirX[i],&dirY[i], dir[0], scale[i]);
    }

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    while (1)
    {
        /* Erase any boxes and lines that were drawn in the last iteration */
		if(count==NUM_BOXES-1){
			count==0;
		}
        for(int i=0; i<NUM_BOXES; i++){
            x_old1 = box_X[i]-dirX[i];
            y_old1 = box_Y[i]-dirY[i];
            draw_box(x_old1-dirX_old1,y_old1-dirY_old1,0x0000);
            if(i==NUM_BOXES-1){
                next = 0;
            }
            else{
                next = i+1;
            }
            x_old2 = box_X[next]-dirX[next];
            y_old2 = box_Y[next]-dirY[next];
            dirX_old2 = dirX[next];
            dirY_old2 = dirY[next];
            change_dir(x_old2,y_old2,&dirX_old2,&dirY_old2, dir[count],scale[next]);
        }
		
        // code for drawing the boxes
        for(int i=0; i<NUM_BOXES; i++){
            draw_box(box_X[i],box_Y[i],BLUE);
           
        }
        // code for updating the locations of boxes
        for(int i=0; i<NUM_BOXES; i++){
            change_dir(box_X[i],box_Y[i],&dirX[i],&dirY[i],dir[count],scale[i]);
            box_X[i] += dirX[i];
            box_Y[i] += dirY[i];
        }
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
		count++;
    }
}

// code for subroutines (not shown)

void swap(int* x, int* y){
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

void wait_for_vsync(){
	volatile int* pixel_ctrl_ptr = (int*)0XFF203020;
	int status;
	*pixel_ctrl_ptr = 1;
	status = *(pixel_ctrl_ptr + 3);
	
	while((status & 0x01)!= 0){
		status=*(pixel_ctrl_ptr + 3);
	}
}

void draw_box(int x, int y, short int color){
    for(int i=0;i<BOX_LEN;i++){
        for(int j=0;j<BOX_LEN;j++){
            plot_pixel(x+i,y+j,color);
        }
    }
}

void change_dir(int x, int y, int*dirX, int*dirY, int dir, int scale){
    if(dir==1||7||8)
        *dirX = -1*scale;	
    if(dir==1||2||3)
        *dirX = -1*scale;
    if(dir==2||6)
        *dirX = 0*scale;
    if(dir==4||8)
        *dirY = 0*scale;
	if(dir==3||4||5)
        *dirX = 1*scale;
	if(dir==5||6||7)
        *dirY = 1*scale;
}
