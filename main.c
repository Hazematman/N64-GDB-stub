#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>

#include <libdragon.h>

#include "gdbstub.h"

static resolution_t res = RESOLUTION_320x240;
static bitdepth_t bit = DEPTH_16_BPP;
static display_context_t disp = 0;

int main(void)
{
    /* enable interrupts (on the CPU) */
    init_interrupts();

    dbg_start();

    /* Initialize peripherals */
    display_init( res, bit, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE );
    console_init();
    console_set_render_mode(RENDER_MANUAL);
    console_clear();
    
    while(!(disp = display_lock()));
    rdp_init();
    
    rdp_attach_display(disp);
    
    rdp_set_default_clipping();
    rdp_enable_primitive_fill();
    rdp_set_primitive_color(0xFF00FF00);
    rdp_draw_filled_rectangle(0,0,100,100);
    
    rdp_detach_display();
    
    display_show(disp);

    while(1)
    {
    }
}
