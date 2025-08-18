#include "dns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
char dns_list[DNS_COUNT][DNS_MAX_LEN] = {0};
dns_mode_t dns_state = DNS2;
dns_mode_t dns_prev = DNS2;
dns_mode_t dns_normal_prev = DNS2;
const char *img_on = "assets/on.svg";
const char *img_off = "assets/off.svg";
bool load_nameservers(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return false;
    char line[DNS_MAX_LEN];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < DNS_COUNT) {
        char *trim = line;
        while (*trim == ' ' || *trim == '\t') trim++;
        char *newline = strchr(trim, '\n');
        if (newline) *newline = '\0';
        if (*trim == '\0') continue;
        strncpy(dns_list[count], trim, DNS_MAX_LEN-1);
        dns_list[count][DNS_MAX_LEN-1] = '\0';
        count++;
    }
    fclose(fp);
    for (int i=count; i<DNS_COUNT; i++) {
        snprintf(dns_list[i], DNS_MAX_LEN, "nameserver 0.0.0.0");
    }
    return true;
}
dns_mode_t detect_dns_state() {
    FILE *fp = fopen("/etc/resolv.conf", "r");
    if (!fp) return DNS2;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#') continue;
        if (strstr(line, dns_list[2])) { fclose(fp); return DNS3; }
        if (strstr(line, dns_list[0]) && !strstr(line, dns_list[1])) { fclose(fp); return DNS1; }
    }
    fclose(fp);
    return DNS2;
}
const char* get_active_dns_ip(void) {
    static char ip[64];
    FILE *fp = fopen("/etc/resolv.conf", "r");
    if (!fp) return "Unknown";
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char *trim = line;
        while (*trim == ' ' || *trim == '\t') trim++;
        if (*trim == '\0' || *trim == '\n' || *trim == '#') continue;
        if (strncmp(trim, "nameserver", 10) == 0) {
            char *space = strchr(trim, ' ');
            if (space) {
                strncpy(ip, space + 1, sizeof(ip) - 1);
                ip[sizeof(ip) - 1] = '\0';
                char *nl = strchr(ip, '\n');
                if (nl) *nl = '\0';
                fclose(fp);
                return ip;
            }
        }
    }
    fclose(fp);
    return "Unknown";
}
const char* get_dns_image(dns_mode_t state) {
    return (state == DNS1) ? img_on : img_off;
}
const char* third_button_label(dns_mode_t state) {
    return (state == DNS3) ? "Turn off 3rd DNS" : "Use 3rd DNS";
}
bool write_resolv(dns_mode_t desired_state) {
    FILE *fp = fopen("/etc/resolv.conf", "r");
    if (!fp) return false;
    char buffer[1024] = {0};
    char line[256];
    bool seen[DNS_COUNT] = {false};
    while (fgets(line, sizeof(line), fp)) {
        int matched = -1;
        for (int i = 0; i < DNS_COUNT; i++) {
            if (strstr(line, dns_list[i])) {
                matched = i;
                break;
            }
        }
        if (matched >= 0) {
            if (desired_state == matched)
                snprintf(buffer + strlen(buffer), sizeof(buffer)-strlen(buffer), "%s\n", dns_list[matched]);
            else
                snprintf(buffer + strlen(buffer), sizeof(buffer)-strlen(buffer), "# %s\n", dns_list[matched]);
            seen[matched] = true;
        } else {
            // leave other lines untouched (domain, search, comments)
            snprintf(buffer + strlen(buffer), sizeof(buffer)-strlen(buffer), "%s", line);
        }
    }
    fclose(fp);
    for (int i = 0; i < DNS_COUNT; i++) {
        if (!seen[i]) {
            if (i == desired_state)
                snprintf(buffer + strlen(buffer), sizeof(buffer)-strlen(buffer), "%s\n", dns_list[i]);
            else
                snprintf(buffer + strlen(buffer), sizeof(buffer)-strlen(buffer), "# %s\n", dns_list[i]);
        }
    }
    FILE *pipe = popen("pkexec tee /etc/resolv.conf > /dev/null", "w");
    if (!pipe) return false;
    if (fputs(buffer, pipe) == EOF) { pclose(pipe); return false; }
    int ret = pclose(pipe);
    if (!(WIFEXITED(ret) && WEXITSTATUS(ret)==0)) return false;
    dns_state = detect_dns_state();
    return true;
}
void set_dns(dns_mode_t desired_state) {
    if (!write_resolv(desired_state)) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Failed to write /etc/resolv.conf.\nUser authentication failed or was canceled.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
}
void check_resolv_conf_startup() {
    FILE *fp = fopen("/etc/resolv.conf", "r");
    if (!fp) return;
    char line[256];
    bool has_active = false;
    while (fgets(line, sizeof(line), fp)) {
        char *trim = line;
        while (*trim==' '||*trim=='\t') trim++;
        if (*trim=='\0'||*trim=='\n'||*trim=='#') continue;
        if (strstr(trim,"nameserver")) { has_active = true; break; }
    }
    fclose(fp);
    if (!has_active) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
            GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
            "/etc/resolv.conf has no active DNS entries.\nIt will be created when you toggle.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}