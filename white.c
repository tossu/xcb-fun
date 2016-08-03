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
    uint32_t                params[1];

    connection = xcb_connect(NULL, NULL);

    // first screen
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    if (!screen)
        return 0;

    root_window = screen->root;

    params[0] = screen->white_pixel;

    // sets the root_window back_pixel to pixel(params)
    xcb_change_window_attributes(connection, root_window, XCB_CW_BACK_PIXEL, params);

    // waits all things are send??
    xcb_flush(connection);

    // xcb_destroy_window(connection, window);
    xcb_disconnect(connection);

    return 0;
}
