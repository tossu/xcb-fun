#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_icccm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include <png.h>

void abort_(const char * s, ...)
{
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        abort();
}

// read_png_file should return those
int width, height;
png_bytep *row_pointers;

void read_png_file(char* file_name)
{
        // TODO CLEAN THIS, IN OWN FILE?
        char header[8];    // 8 is the maximum size that can be checked

        png_structp png_ptr;
        png_infop info_ptr;

        /* open file and test for it being a png */
        FILE *fp = fopen(file_name, "rb");
        if (!fp)
                abort_("[read_png_file] File %s could not be opened for reading", file_name);
        fread(header, 1, 8, fp);
        if (png_sig_cmp(header, 0, 8))
                abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);

        /* initialize stuff */
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr)
                abort_("[read_png_file] png_create_read_struct failed");

        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
                abort_("[read_png_file] png_create_info_struct failed");

        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[read_png_file] Error during init_io");

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);

        png_read_info(png_ptr, info_ptr);

        width = png_get_image_width(png_ptr, info_ptr);
        height = png_get_image_height(png_ptr, info_ptr);
        //color_type = png_get_color_type(png_ptr, info_ptr);
        //bit_depth = png_get_bit_depth(png_ptr, info_ptr);

        //number_of_passes = png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);

        /* read file */
        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[read_png_file] Error during read_image");

        row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
        int y;
        for (y=0; y<height; y++)
                row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

        png_read_image(png_ptr, row_pointers);

        fclose(fp);

        /* if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
                abort_("[process_file] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
                       "(lacks the alpha channel)");

        if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
                abort_("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
                       PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));
        */
}


static xcb_format_t *
find_format (xcb_connection_t * c, uint8_t depth, uint8_t bpp)
{
    // TODO: Wtf this actually does?
    const xcb_setup_t *setup = xcb_get_setup(c);
    xcb_format_t *fmt = xcb_setup_pixmap_formats(setup);
    xcb_format_t *fmtend = fmt + xcb_setup_pixmap_formats_length(setup);

    for(; fmt != fmtend; ++fmt)
    {
        if((fmt->depth == depth) && (fmt->bits_per_pixel == bpp)) {
        /* printf("fmt %p has pad %d depth %d, bpp %d\n",
            fmt,fmt->scanline_pad, depth,bpp); */
            return fmt;
        }
    }
    return 0;
}

void
fillimage(unsigned char *p)
{
    // TODO CLEAN THIS
    int x,y;
    /* height, width, row_pointers */
    for (y=0; y<height; y++) {
            png_byte *row = row_pointers[y];
            for (x=0; x<width; x++) {
                    png_byte *ptr = &(row[x*4]); // needs to be 4 if alpha
                    //printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
                    //       x, y, ptr[0], ptr[1], ptr[2], ptr[3]);

                    *p++=ptr[2] % 256; // blue
                    *p++=ptr[1] % 256; // green
                    *p++=ptr[0] % 256; // red
                    p++;
            }
    }
}

xcb_image_t *
CreateTrueColorImage(xcb_connection_t *c)
{
    // TODO CLEAN THIS
    // I want xcb_image_t from PNG file with library
    const xcb_setup_t *setup = xcb_get_setup(c);
    unsigned char *image32=(unsigned char *)malloc(width*height*4);
    xcb_format_t *fmt = find_format(c, 24, 32);
    if (fmt == NULL)
    {
        return NULL;
    }

    fillimage(image32);

    return xcb_image_create(width,
        height,
        XCB_IMAGE_FORMAT_Z_PIXMAP,
        fmt->scanline_pad,
        fmt->depth,
        fmt->bits_per_pixel,
        0,
        setup->image_byte_order,
        XCB_IMAGE_ORDER_LSB_FIRST,
        image32,
        width*height*4,
        image32);
}

int
main (int argc, char **argv)
{
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    xcb_window_t root_window;

    xcb_pixmap_t pmap;
    xcb_gcontext_t gc;
    xcb_image_t *image;

    xcb_expose_event_t *ee;
    xcb_generic_event_t *e;

    uint32_t params[1];
    uint32_t mask;
    uint32_t values[2];

    read_png_file("dick.png");

    // connects server
    connection = xcb_connect (NULL, NULL);

    // first screen
    screen = xcb_setup_roots_iterator (xcb_get_setup (connection)).data;

    // we use root window
    root_window = screen->root;

    // makes root_window listen EXPOSURE event
    params[0] = XCB_EVENT_MASK_EXPOSURE;
    xcb_change_window_attributes(connection, root_window, XCB_CW_EVENT_MASK, params);

    /* create image */
    image = CreateTrueColorImage(connection);
    if (image == NULL) {
        printf ("Cannot create iamge\n");
        xcb_disconnect(connection);
        return 1;
    }

    /* create backing pixmap */
    pmap = xcb_generate_id(connection);
    xcb_create_pixmap(connection, 24, pmap, root_window,
            image->width, image->height);

    // creating graphical context
    mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    values[0] = screen->white_pixel;
    values[1] = 0;
    gc = xcb_generate_id (connection);
    xcb_create_gc (connection, gc, pmap, mask, values);

    /* put the image into the pixmap */
    xcb_image_put(connection, pmap, gc, image, 0, 0, 0);

    xcb_flush (connection);

    /* event loop */
    while ((e = xcb_wait_for_event (connection))) {
        switch (e->response_type) {
            case XCB_EXPOSE:
                ee=(xcb_expose_event_t *)e;
                // printf ("expose %d,%d - %d,%d\n",
                //    ee->x,ee->y,ee->width,ee->height);
                xcb_copy_area(connection, pmap, root_window, gc,
                        ee->x,
                        ee->y,
                        ee->x,
                        ee->y,
                        ee->width,
                        ee->height);
                xcb_flush (connection);
                break;
        }
        free (e);
    }

    /* free pixmap */
    xcb_free_pixmap(connection, pmap);

    /* close connection to server */
    xcb_disconnect (connection);

    return 0;
}
