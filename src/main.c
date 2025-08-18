#include <gtk/gtk.h>
#include "dns.h"
#include "svg.h"
#define IMG_SIZE 200
static bool startup_done = false;
const char *serverlist = "nameservers.conf";
const char *logo = "assets/logo.svg";
GtkWidget *dns_ip_label;
GtkWidget *image, *button, *third_button;
static void update_ui() {
    if (dns_ip_label) {
        const char *ip = get_active_dns_ip();
        char label_text[128];
        snprintf(label_text, sizeof(label_text), "Current: %s", ip);
        gtk_label_set_text(GTK_LABEL(dns_ip_label), label_text);
    }
    GdkPixbuf *pb = load_svg_scaled(get_dns_image(dns_state), IMG_SIZE);
    if (pb) { gtk_image_set_from_pixbuf(GTK_IMAGE(image), pb); g_object_unref(pb); }
    gtk_button_set_label(GTK_BUTTON(third_button), third_button_label(dns_state));
}
static GtkWidget* create_button(const char *label, gboolean is_toggle, GCallback callback) {
    GtkWidget *btn = is_toggle ? gtk_toggle_button_new_with_label(label) : gtk_button_new();
    if (!is_toggle && image) gtk_button_set_image(GTK_BUTTON(btn), image);
    g_signal_connect(btn, is_toggle ? "toggled" : "clicked", callback, NULL);
    return btn;
}
static void dns_button_callback(GtkWidget *widget, gpointer data) {
    if (!startup_done) return;
    gboolean is_toggle = GTK_IS_TOGGLE_BUTTON(widget);
    if (is_toggle) { // Third DNS toggle
        gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
        if (active) { if(dns_state!=DNS3) dns_normal_prev=dns_state; set_dns(DNS3); }
        else set_dns(dns_normal_prev);
    } else { // Main DNS toggle
        if(dns_state==DNS1) set_dns(DNS2);
        else if(dns_state==DNS2) set_dns(DNS1);
        else if(dns_state==DNS3) set_dns(dns_prev);
    }
    // Update UI in both cases
    GdkPixbuf *pb = load_svg_scaled(get_dns_image(dns_state), IMG_SIZE);
    if(pb){ gtk_image_set_from_pixbuf(GTK_IMAGE(image), pb); g_object_unref(pb); }
    gtk_button_set_label(GTK_BUTTON(third_button), third_button_label(dns_state));
    gtk_widget_set_sensitive(button, dns_state!=DNS3);
    update_ui();
}
int main(int argc, char *argv[]) {
    gtk_init(&argc,&argv);
    if (!load_nameservers(serverlist)) {
        g_warning("Failed to load nameservers file, using defaults.");
    }
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GdkRGBA black;
    gdk_rgba_parse(&black, "black");
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &black);
    GdkPixbuf *icon = load_svg_scaled(logo, 192);
    if(icon){ gtk_window_set_icon(GTK_WINDOW(window), icon); g_object_unref(icon); }
    gtk_window_set_title(GTK_WINDOW(window), "DNS Toggle");
    gtk_window_set_default_size(GTK_WINDOW(window), IMG_SIZE, IMG_SIZE);
    image = gtk_image_new();
    button = create_button(NULL, FALSE, G_CALLBACK(dns_button_callback));
    third_button = create_button("Use 3rd DNS", TRUE, G_CALLBACK(dns_button_callback));
    dns_ip_label = gtk_label_new(get_active_dns_ip());
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_box_pack_start(GTK_BOX(vbox), dns_ip_label, TRUE, TRUE, 20);
    gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), third_button, TRUE, TRUE, 20);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    check_resolv_conf_startup();
    dns_state = detect_dns_state();
    if(dns_state!=DNS3) dns_normal_prev=dns_state;
    dns_prev = (dns_state==DNS3)? DNS2: dns_state;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(third_button), dns_state==DNS3);
    gtk_widget_set_sensitive(button, dns_state!=DNS3);
    update_ui();
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    startup_done = true;
    gtk_main();
    return 0;
}