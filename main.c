
#define PIXEL_BUF_CTRL_BASE   0xFF203020

int main(void){
    volatile int * pixel_ctrl_ptr = (int *) PIXEL_BUF_CTRL_BASE;
    draw_start_screen();

    while(1){

    }
}