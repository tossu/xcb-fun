#include <xcb/xcb.h>
#include <sys/types.h>
#include <stdlib.h> // exit
#include <stdio.h> // printf
#include <xcb/xcb.h> // color

int
main ()
{
    xcb_connection_t*       connection;
    xcb_screen_t*           screen;
    xcb_window_t            root_window;
    xcb_alloc_color_reply_t *color_reply;
    xcb_colormap_t          colormap = { 0 };
    uint32_t                params[1];
    uint16_t                r,g,b;

    connection = xcb_connect(NULL, NULL);
    if (!connection) {
        fprintf(stderr, "ERROR: can't connect to an X server\n");
        return -1;
    }

    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    if (!screen)
    {
        fprintf(stderr, "ERROR: can't setup a screen\n");
        xcb_disconnect(connection);
        return -1;
    }

    root_window = screen->root;
    colormap = screen->default_colormap;

    r = 60000, g = 20000, b = 20000;
    color_reply = xcb_alloc_color_reply(connection,
            xcb_alloc_color(connection, colormap, r, g, b), NULL);

    if(!color_reply)
    {
        fprintf(stderr, "ERROR: can't alloc an color\n");
        xcb_disconnect(connection);
        return 0;
    }

    params[0] = color_reply->pixel;

    // sets the root_window back_pixel to pixel(params)
    xcb_change_window_attributes(connection, root_window, XCB_CW_BACK_PIXEL, params);

    // free color resources
    free(color_reply);

    // waits all things are send??
    xcb_flush(connection);

    // xcb_destroy_window(connection, window);
    xcb_disconnect(connection);

    return 0;
}
