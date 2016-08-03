#include <xcb/xcb.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void
newtext(xcb_connection_t  *connection,
            xcb_screen_t     *screen,
            xcb_window_t      window,
            int16_t           x,
            int16_t           y,
            const char       *label)
{
    xcb_font_t font;
    xcb_gcontext_t gc;
    uint32_t mask;
    uint32_t value_list[3];

    font = xcb_generate_id (connection);
    xcb_open_font (connection, font, strlen("fixed"), "fixed");

    gc            = xcb_generate_id (connection);
    mask          = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
    value_list[0] = screen->white_pixel;
    value_list[1] = screen->black_pixel;
    value_list[2] = font;

    xcb_create_gc(connection, gc, window, mask, value_list );
    xcb_close_font(connection, font);
    xcb_image_text_8(connection, strlen(label),
            window, gc, x, y, label);
    xcb_free_gc (connection, gc);
    xcb_flush(connection);
}

void
draw (xcb_connection_t *c, xcb_screen_t *s, xcb_window_t w)
{
    newtext(c, s, w, 100, 100, "SEPPO SEPPO SEPPO");
    newtext(c, s, w, 100, 130, "SEPPO SEPPO SEPPO");
}

int
main ()
{
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_window_t root_window;
    xcb_generic_event_t *e;
    uint32_t  params[1];

    connection = xcb_connect(NULL, NULL);
    if (!connection) {
        fprintf(stderr, "ERROR: can't connect to an X server\n");
        return -1;
    }

    // first avaible screen
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    if (!screen)
    {
        fprintf(stderr, "ERROR: can't setup a screen\n");
        xcb_disconnect(connection);
        return -1;
    }

    root_window = screen->root;

    // makes root_window listen EXPOSURE event
    params[0] = XCB_EVENT_MASK_EXPOSURE;
    xcb_change_window_attributes(connection, root_window,
            XCB_CW_EVENT_MASK, params);

    // waits all things are send??
    xcb_flush(connection);

    draw(connection, screen, root_window);

    while ((e = xcb_wait_for_event (connection))) {
        switch (e->response_type) {
            case XCB_EXPOSE:
                draw(connection, screen, root_window);
                break;
        }
        free (e);
    }

    xcb_disconnect(connection);

    return 0;
}
