#ifndef SVG_H
#define SVG_H
#include <gtk/gtk.h>
GdkPixbuf* load_svg_scaled(const char *filename, int size);
#endif // SVG_H