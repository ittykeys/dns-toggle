#include "svg.h"
#include <math.h>
#include <librsvg/rsvg.h>
#include <cairo.h>
#include <stdio.h>
GdkPixbuf* load_svg_scaled(const char *filename, int size) {
    GError *error = NULL;
    RsvgHandle *handle = rsvg_handle_new_from_file(filename, &error);
    if (!handle) { g_printerr("%s\n", error->message); g_error_free(error); return NULL; }
    gdouble w=0, h=0;
    rsvg_handle_get_intrinsic_size_in_pixels(handle, &w, &h);
    if (!w || !h) w = h = size;
    double scale = (double)size / fmax(w,h);
    int tw = (int)(w*scale), th=(int)(h*scale);
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, tw, th);
    cairo_t *cr = cairo_create(surface);
    cairo_scale(cr, scale, scale);
    RsvgRectangle vp = {0,0,w,h};
    if (!rsvg_handle_render_document(handle, cr, &vp, &error)) {
        g_printerr("%s\n", error->message); g_error_free(error);
    }
    GdkPixbuf *pixbuf = gdk_pixbuf_get_from_surface(surface, 0,0, tw, th);
    cairo_destroy(cr); cairo_surface_destroy(surface);
    g_object_unref(handle);
    return pixbuf;
}