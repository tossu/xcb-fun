#include <xcb/xcb.h>
#include <sys/types.h>
#include <stdlib.h> // exit
#include <stdio.h> // printf
#include <math.h> // tan

struct vector { double x; double y; double z;};

struct triangle { struct vector a; struct vector b; struct vector c; };

struct mat4x4 { double m[4][4]; };

struct triangle cube[12] = {
		// SOUTH
		{ { 0.0, 0.0, 0.0},   { 0.0, 1.0, 0.0},   { 1.0, 1.0, 0.0 }},
		{ { 0.0, 0.0, 0.0},   { 1.0, 1.0, 0.0},   { 1.0, 0.0, 0.0 }},
		//{  EAT         }    {              }    {               }
		{ { 1.0, 0.0, 0.0},   { 1.0, 1.0, 0.0},   { 1.0, 1.0, 1.0 }},
		{ { 1.0, 0.0, 0.0},   { 1.0, 1.0, 1.0},   { 1.0, 0.0, 1.0 }},
		//{  NOTH        }    {              }    {               }
		{ { 1.0, 0.0, 1.0},   { 1.0, 1.0, 1.0},   { 0.0, 1.0, 1.0 }},
		{ { 1.0, 0.0, 1.0},   { 0.0, 1.0, 1.0},   { 0.0, 0.0, 1.0 }},
		//{  WET         }    {              }    {               }
		{ { 0.0, 0.0, 1.0},   { 0.0, 1.0, 1.0},   { 0.0, 1.0, 0.0 }},
		{ { 0.0, 0.0, 1.0},   { 0.0, 1.0, 0.0},   { 0.0, 0.0, 0.0 }},
		//{  TO          }    {              }    {               }
		{ { 0.0, 1.0, 0.0},   { 0.0, 1.0, 1.0},   { 1.0, 1.0, 1.0 }},
		{ { 0.0, 1.0, 0.0},   { 1.0, 1.0, 1.0},   { 1.0, 1.0, 0.0 }},
		//{  BOTOM       }    {              }    {               }
		{ { 1.0, 0.0, 1.0},   { 0.0, 0.0, 1.0},   { 0.0, 0.0, 0.0 }},
		{ { 1.0, 0.0, 1.0},   { 0.0, 0.0, 0.0},   { 1.0, 0.0, 0.0 }}
};


// input, output and matrix
void MultiplyMatrixVector(struct vector *i, struct vector *o, struct mat4x4 *m)
{
	o->x = i->x * m->m[0][0] + i->y * m->m[1][0] + i->z * m->m[2][0] + m->m[3][0];	
	o->y = i->x * m->m[0][1] + i->y * m->m[1][1] + i->z * m->m[2][1] + m->m[3][1];	
	o->z = i->x * m->m[0][2] + i->y * m->m[1][2] + i->z * m->m[2][2] + m->m[3][2];	
	
	double w = i->x * m->m[0][3] + i->y * m->m[1][3] + i->z * m->m[2][3] + m->m[3][3];	
	
	if(w != 0) {
		o->x /= w;
		o->y /= w;
		o->z /= w;
	}
}

void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, xcb_connection_t* connection, xcb_window_t window, xcb_gcontext_t context)
{
	xcb_point_t corner1[] = {{x1, y1}, {x2, y2}};
	xcb_poly_line (connection, XCB_COORD_MODE_ORIGIN, window, context, 2, corner1);
	xcb_point_t corner2[] = {{x2, y2}, {x3, y3}};
	xcb_poly_line (connection, XCB_COORD_MODE_ORIGIN, window, context, 2, corner2);
	xcb_point_t corner3[] = {{x3, y3}, {x1, y1}};
	xcb_poly_line (connection, XCB_COORD_MODE_ORIGIN, window, context, 2, corner3);
}

int height = 768;
int width = 1366;

int
main ()
{

    double fNear = 0.1;
    double fFar = 1000.0;
    double fFov = 90.0;

    // monitor height / width
    double fAspectRation = (double) height / (double) width;
    double fFovRad = 1.0 / tan(fFov * 0.5 / 180.0 * 3.14159);
    
    struct mat4x4 matProj;
    matProj.m[0][0] = (double) fAspectRation * fFovRad;
    matProj.m[0][1] = 0.0;
    matProj.m[0][2] = 0.0;
    matProj.m[0][3] = 0.0;

    matProj.m[1][0] = 0.0;
    matProj.m[1][1] = fFovRad;
    matProj.m[1][2] = 0.0;
    matProj.m[1][3] = 0.0;

    matProj.m[2][0] = 0.0;
    matProj.m[2][1] = 0.0;
    matProj.m[2][2] = (double) fFar / (fFar - fNear);
    matProj.m[2][3] = 1.0;

    matProj.m[3][0] = 0.0;
    matProj.m[3][1] = 0.0;
    matProj.m[3][2] = (double) (-fFar * fNear) / (fFar - fNear);
    matProj.m[3][3] = 0.0;

    xcb_connection_t*       connection;
    xcb_screen_t*           screen;
    xcb_window_t            root_window;

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

   double x = 1;

    // listens EXPOSURE events
    xcb_generic_event_t *event;
    while ((event = xcb_wait_for_event (connection))) {
        switch (event->response_type & ~0x80) {
            case XCB_EXPOSE:
		printf("");
		
		    struct mat4x4 penis;
		    penis.m[0][0] = cos(x);
		    penis.m[0][1] = -sin(x);
		    penis.m[0][2] = 0.0;
		    penis.m[0][3] = 0.0;

		    penis.m[1][0] = sin(x);
		    penis.m[1][1] = cos(x)+cos(x);
		    penis.m[1][2] = 0+-sin(x);
		    penis.m[1][3] = 0.0;

		    penis.m[2][0] = 0.0;
		    penis.m[2][1] = sin(x);
		    penis.m[2][2] = cos(x);
		    penis.m[2][3] = 1.0;

		    penis.m[3][0] = 0.0;
		    penis.m[3][1] = 0.0;
		    penis.m[3][2] = 0.0;
		    penis.m[3][3] = 0.0;
		
	        int i;
	        for ( i=0; i<12; i++) {
		    struct triangle triProjected, triTranslated, triRotatedZ, triRotatedZX, seppo;
		   
		    triTranslated = cube[i]; 

		    MultiplyMatrixVector(&triTranslated.a, &seppo.a, &penis);
		    MultiplyMatrixVector(&triTranslated.b, &seppo.b, &penis);
		    MultiplyMatrixVector(&triTranslated.c, &seppo.c, &penis);
		   
		    seppo.a.z = triTranslated.a.z + 3;
		    seppo.b.z = triTranslated.b.z + 3;
		    seppo.c.z = triTranslated.c.z + 3;
		   
		    MultiplyMatrixVector(&seppo.a, &triProjected.a, &matProj);
		    MultiplyMatrixVector(&seppo.b, &triProjected.b, &matProj);
		    MultiplyMatrixVector(&seppo.c, &triProjected.c, &matProj);
		    
		    triProjected.a.x += 1.0; triProjected.a.y += 1.0;
		    triProjected.b.x += 1.0; triProjected.b.y += 1.0;
		    triProjected.c.x += 1.0; triProjected.c.y += 1.0;

		    triProjected.a.x *= 0.5 * (double) width;
		    triProjected.a.y *= 0.5 * (double) height;

		    triProjected.b.x *= 0.5 * (double) width;
		    triProjected.b.y *= 0.5 * (double) height;

		    triProjected.c.x *= 0.5 * (double) width;
		    triProjected.c.y *= 0.5 * (double) height;

		    DrawTriangle(triProjected.a.x, triProjected.a.y, 
			         triProjected.b.x, triProjected.b.y, 
			         triProjected.c.x, triProjected.c.y, 
			         connection,
			         root_window,
			         foreground);
	        }
	
		x += 0.0005;	
		xcb_clear_area(connection, 1, root_window, 0, 0, width, height);

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
