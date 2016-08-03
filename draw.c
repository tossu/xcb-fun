#include <xcb/xcb.h>
#include <sys/types.h>
#include <stdlib.h> // exit
#include <stdio.h> // printf

int
main ()
{
    xcb_connection_t*       connection;
    xcb_screen_t*           screen;
    xcb_window_t            root_window;

    // x, y, w, h, angle1, angle2
    xcb_arc_t arcs[] = {
        {10, 100, 60, 40, 0, 360 << 6},
        {70, 100, 60, 40, 0, 360 << 6},
        {55, 100, 30, 200, 0, 360 << 6}};

    // connects server
    connection = xcb_connect(NULL, NULL);

    // first screen
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    root_window = screen->root;

    // wait all things done
    xcb_flush(connection);

    // makes root_window listen EXPOSURE event
    uint32_t params[1] = {XCB_EVENT_MASK_EXPOSURE};
    xcb_change_window_attributes(connection, root_window, XCB_CW_EVENT_MASK, params);

    // creating graphical context
    xcb_gcontext_t foreground = xcb_generate_id (connection);
    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    uint32_t values[2] = {screen->white_pixel, 0};
    xcb_create_gc (connection, foreground, root_window, mask, values);

    xcb_flush (connection);

    // listens EXPOSURE events
    xcb_generic_event_t *event;
    while ((event = xcb_wait_for_event (connection))) {
        switch (event->response_type & ~0x80) {
            case XCB_EXPOSE:
                // draws penis
                xcb_poly_arc (connection, root_window, foreground, 3, arcs);
                xcb_flush (connection);
                break;
            default:
                break;
        }
        free (event);
    };

    // waits all things are send??
    xcb_flush(connection);

    // xcb_destroy_window(connection, window);
    xcb_disconnect(connection);

    return 0;
}
