#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "timer.h"
#include "board.h"
#include "lib.h"

#define SYSCTL_SHPR3    (*(volatile unsigned long *)(0xE000ED20))
#define NVIC_PRI1       (*(volatile unsigned long *)(0xE000E404))

#ifndef IOREG32
#define IOREG32(addr) (*(volatile unsigned long *) (addr))
#endif

#define RNG_TASK_START    IOREG32(0x4000D000)
#define RNG_TASK_STOP     IOREG32(0x4000D004)
#define RNG_VALUE         IOREG32(0x4000D508)

// function prototype.
int reset_game(int);
static void dump_stats(void);
void update_ball(void);
void update_frame(void);


volatile uint8_t frame_buffer[LED_NUM_ROWS][LED_NUM_COLS]
       ={
            { 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0 },
            { 0, 0, 1, 0, 0 }
        };

// (x,y) for catch and (x1,y1) for the ball
int x = LED_NUM_COLS/2;
int y = LED_NUM_ROWS-1;
int x1 = LED_NUM_COLS-1;
int y1 = 0;
int level = 1;

extern const uint8_t led_rows[];
extern const uint8_t led_cols[];

uint32_t row, col;

void led_row_refresh(void)
{
    uint32_t c;

    gpio_clear(led_rows[row]);
    
    row = (row + 1) % LED_NUM_ROWS;

    for (c = 0; c < LED_NUM_COLS; c++)
    {
        if (frame_buffer[row][c])   // is the pixel set?
            gpio_clear(led_cols[c]);    // col low to turn on
        else
            gpio_set(led_cols[c]);      // col high to turn off
    }

    gpio_set(led_rows[row]);
}

/* Debug counters */
int b0d, b1d, b0p, b1p;

/* Debounce routines are called after a debounce delay after the
 * button-press was detected.
 * Thereby rejecting any bouncing that could have happened
 * during debounce period.
 */

void button0_debounce(void)
{
    if (button_get(0))
    {
        b0d++;
        // left
        frame_buffer[y][x] = 0;     // clear
        x--;
        if (x < 0)
            x = 0;
        frame_buffer[y][x] = 1;     // draw next
        audio_beep(1000, 10);
    }



    timer_stop(1);
}

/* button-press routine is called by the GPIO interrupt. We may
 * get multiple interrupts in the beginning or end of a button-press.
 * Hence, we start a timer and check if the button was still pressed
 * after the delay to debounce.
 */
void button0_callback(void)
{
    b0p++;
    timer_start(1, 5, button0_debounce);
}

/* see button0_debounce for comments */
void button1_debounce(void)
{
    if (button_get(1))
    {
        b1d++;
        // right
        frame_buffer[y][x] = 0;     // clear
        x++;
        if (x >= LED_NUM_COLS)
            x = LED_NUM_COLS - 1;
        frame_buffer[y][x] = 1;     // draw next
        audio_beep(500, 10);
    }

    timer_stop(2);
}

/* see button0_callback for comments */
void button1_callback(void)
{
    b1p++;
    timer_start(2, 5, button1_debounce);
}

int main(void)
{
    /* Initialiazation */
    board_init();

    /* Greetings */
    puts("hello, world!\n");
    audio_sweep(100, 2000, 200);

    /* Start timer to set refresh routine */
    gpio_inten(BUTTON_0, 0, GPIO_BOTHEDGES, button0_callback);
    gpio_inten(BUTTON_1, 1, GPIO_BOTHEDGES, button1_callback);
    timer_start(0, 5, update_frame );
    timer_start(3, 1000, update_ball); // timer.c is modified to accomodate 4 timers

    // Starting the random number generator
    RNG_TASK_START = 1;


    /* Both ISRs are short enough not to delay other ISR's functionality.
     * So default priroity will work. The following code is just to show how
     * priorities can be set.
     */

    SYSCTL_SHPR3 |= ((0x3 << 5) << 24);     // systick ISR priority
    NVIC_PRI1 |= ((0x2 << 5) << 16);        // gpio ISR priority

    while (1)
    {
        dump_stats();
    }

    return 0;
}


void dump_stats(void)
{
    static int prev_counter;
    extern volatile int gpio_irq_counter;
    extern volatile int gpio_irq_event_counters[8];

    if (prev_counter != gpio_irq_counter)
    {
        int i;
        char str[100];
        puts("\n\rgpio_irq_event_counters : ");
        for (i = 0; i < 8; i++){
            sprintf(str, "%d", gpio_irq_event_counters[i]);            
            puts(str);            
        }
        puts("\n\rgpio_irq_counter :");
        sprintf(str, "%d", gpio_irq_counter); 
        puts(str);

        puts("\n\rb0d: ");
        sprintf(str, "%d", b0d); 
        puts(str);
        puts(" / b0p: ");
        sprintf(str, "%d", b0p); 
        puts(str);
        puts("\n\rb1d: ");
        sprintf(str, "%d", b1d); 
        puts(str);
        puts(" / b1p: ");
        sprintf(str, "%d", b1p); 
        puts(str);
        prev_counter = gpio_irq_counter;
        puts("\r\n--------------------------------------------------");
    }
}


int reset_game(int Success)
{
   
    
    if (Success == 1 )
    {
        if(level > 10)   level = 10;
        else             level++;
        timer_start(3, 1000/level, update_ball);                 
    }
    else
    {
        level = 1;
        timer_start(3, 1000, update_ball);
    }
    
    // clear
    frame_buffer[y][x] = 0;     
    frame_buffer[y1][x1] = 0;    
    x = LED_NUM_COLS/2;
    y = LED_NUM_ROWS-1;
    x1 = RNG_VALUE % 5;
    y1 = 0;

    // draw next
    frame_buffer[y][x] = 1;     
    //frame_buffer[y1][x1] = 1;   
    update_ball();  
    audio_sweep(100, 2000, 500); 

    return 0;
}
void update_ball(void)
{
    frame_buffer[y1][x1] = 0;
    if (y1 == 5) 
        y1 = 0;
    else 
        y1++;
    frame_buffer[y1][x1] = 1;
}

void update_frame(void)
{
    if(y1 == LED_NUM_ROWS-1 && x == x1)
        reset_game(1);
    if(y1 == LED_NUM_ROWS-1 && x != x1)
        reset_game(0);

    led_row_refresh();
}