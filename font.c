#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <xcb/xcb.h>

#define WIDTH 300
#define HEIGHT 100


static xcb_gc_t getFontGC (xcb_connection_t *c,
                                xcb_screen_t     *screen,
                                xcb_window_t      window,
                                const char       *font_name );


static void drawText (xcb_connection_t *c,
                        xcb_screen_t     *screen,
                        xcb_window_t      window,
                        int16_t           x1,
                        int16_t           y1,
                        const char       *label );

static void
drawText (xcb_connection_t  *connection,
            xcb_screen_t     *screen,
            xcb_window_t      window,
            int16_t           x,
            int16_t           y,
            const char       *label )
{

    xcb_gcontext_t gc = getFontGC (connection, screen, window, "fixed");
    xcb_image_text_8(connection, strlen(label), window, gc, x, y, label);
    xcb_free_gc (connection, gc);
}


static xcb_gc_t
getFontGC (xcb_connection_t  *connection,
            xcb_screen_t      *screen,
            xcb_window_t       window,
            const char        *font_name )
{
    xcb_font_t font = xcb_generate_id(connection);
    xcb_open_font(connection, font, strlen(font_name), font_name);

    /* create graphics context */
    xcb_gcontext_t  gc            = xcb_generate_id (connection);
    uint32_t        mask          = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
    uint32_t        value_list[3] = { screen->white_pixel,
                                        screen->black_pixel,
                                        font };

    xcb_create_gc(connection, gc, window, mask, value_list);
    xcb_close_font(connection, font);

    xcb_flush(connection);  // make sure window is drawn

    return gc;
}


int
main ()
{
    /* get the connection */
    int screenNum;
    xcb_connection_t *connection = xcb_connect (NULL, &screenNum);
    if (!connection) {
        fprintf (stderr, "ERROR: can't connect to an X server\n");
        return -1;
    }


    /* get the current screen */
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (connection));

    // we want the screen at index screenNum of the iterator
    for (int i = 0; i < screenNum; ++i) {
        xcb_screen_next (&iter);
    }

    xcb_screen_t *screen = iter.data;

    if (!screen) {
        fprintf (stderr, "ERROR: can't get the current screen\n");
        xcb_disconnect (connection);
        return -1;
    }

    /* create the window */
    xcb_window_t window = screen->root;

    // makes root_window listen EXPOSURE event
    uint32_t params[1];
    params[0] = XCB_EVENT_MASK_EXPOSURE;
    xcb_change_window_attributes(connection, window, XCB_CW_EVENT_MASK, params);

    xcb_flush(connection);  // make sure window is drawn

    xcb_generic_event_t *e;
    while ((e = xcb_wait_for_event (connection))) {
        switch (e->response_type) {
            case XCB_EXPOSE:
                printf("PENIS\n");
                drawText (connection, 
                            screen,
                            window,
                            10, HEIGHT - 10,
                            "Press ESC key to exit..." );
                break;
        }
        free (e);
    }
    return 0;
}
