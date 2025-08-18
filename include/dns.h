#ifndef DNS_H
#define DNS_H
#include <gtk/gtk.h>
#include <stdbool.h>
typedef enum { DNS1, DNS2, DNS3 } dns_mode_t;
extern dns_mode_t dns_state;
extern dns_mode_t dns_prev;
extern dns_mode_t dns_normal_prev;
#define DNS_MAX_LEN 64
#define DNS_COUNT 3
extern char dns_list[DNS_COUNT][DNS_MAX_LEN];
bool load_nameservers(const char *filename);
void check_resolv_conf_startup(void);
dns_mode_t detect_dns_state(void);
bool write_resolv(dns_mode_t desired_state);
void set_dns(dns_mode_t desired_state);
const char* get_active_dns_ip(void);
const char* get_dns_image(dns_mode_t state);
const char* third_button_label(dns_mode_t state);
#endif // DNS_H