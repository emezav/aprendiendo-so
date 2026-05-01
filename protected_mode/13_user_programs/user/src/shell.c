/**
 * @file
 * @brief Shell minima de usuario para `12_user_shell`.
 */

#include <stdlib.h>
#include <userlib.h>

#define SHELL_LINE_MAX 127

static void shell_print_prompt(void);
static void shell_show_help(void);
static void shell_read_line(char * line, unsigned int line_size);
static void shell_build_boot_path(const char * name, char * path,
        unsigned int path_size);

int main(void) {
    char line[SHELL_LINE_MAX + 1];
    char path[SHELL_LINE_MAX + 1];

    user_printf("shell: ready\n");
    shell_show_help();

    for (;;) {
        shell_print_prompt();
        shell_read_line(line, sizeof(line));

        if (line[0] == 0) {
            continue;
        }

        if (user_strcmp(line, "help") == 0) {
            shell_show_help();
            continue;
        }

        if (user_strcmp(line, "ticks") == 0) {
            user_printf("ticks=%u\n", sys_get_ticks());
            continue;
        }

        if (user_strcmp(line, "exit") == 0) {
            exit(0);
        }

        if (line[0] == '/') {
            if (sys_exec(line) != 0) {
                user_printf("shell: could not exec %s\n", line);
            }
            continue;
        }

        shell_build_boot_path(line, path, sizeof(path));
        if (sys_exec(path) != 0) {
            user_printf("shell: command not found: %s\n", line);
        }
    }
}

static void shell_print_prompt(void) {
    user_printf("ush> ");
}

static void shell_show_help(void) {
    user_printf("builtins: help ticks exit\n");
    user_printf("external: hello.elf ls.elf cat.elf uname.elf\n");
}

static void shell_read_line(char * line, unsigned int line_size) {
    unsigned int length;
    unsigned int value;
    char ch;

    length = 0;
    user_memset(line, 0, line_size);

    for (;;) {
        value = sys_read_char();
        if (value == 0) {
            continue;
        }

        ch = (char)value;
        if (ch == '\r' || ch == '\n') {
            user_printf("\n");
            line[length] = 0;
            return;
        }

        if (ch == '\b') {
            if (length > 0) {
                length--;
                line[length] = 0;
                sys_write("\b \b", 3);
            }
            continue;
        }

        if (ch >= ' ' && ch <= '~' && length < (line_size - 1)) {
            line[length++] = ch;
            line[length] = 0;
            sys_write(&ch, 1);
        }
    }
}

static void shell_build_boot_path(const char * name, char * path,
        unsigned int path_size) {
    const char * prefix;
    unsigned int prefix_len;
    unsigned int name_len;
    unsigned int copy_len;

    prefix = "/boot/";
    prefix_len = user_strlen(prefix);
    name_len = user_strlen(name);

    user_memset(path, 0, path_size);
    user_memcpy(path, prefix, prefix_len);
    if (prefix_len >= path_size) {
        path[path_size - 1] = 0;
        return;
    }

    copy_len = name_len;
    if ((prefix_len + copy_len) >= path_size) {
        copy_len = path_size - prefix_len - 1;
    }

    user_memcpy(path + prefix_len, name, copy_len);
    path[prefix_len + copy_len] = 0;
}
