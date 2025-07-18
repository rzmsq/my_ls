#include <stdint.h>
#include <time.h>
#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

void check_on_error() {
    switch(errno) {
        case EACCES:
            fprintf(stderr, "Permission denied");
            exit(1);
        case ENFILE:
            fprintf(stderr, "Limit on the total number if open files has been reached");
            exit(1);
        case ENOENT:
            fprintf(stderr, "Directory does not exists");
            exit(1);
        case ENOMEM:
            fprintf(stderr, "Insufficient memory to complete operation");
            exit(1);
        case ENOTDIR:
            fprintf(stderr, "Name is not directory");
            exit(1);
        case EBADF:
            fprintf(stderr, "Invalid directory stream descriptor");
            exit(1);

    }
}

void uintmax_to_str_mode(const uintmax_t src, char *dst) {
    
    const char sym[3] = {'r','w','x'};

    uintmax_t mask = 01, shift = 02;
    for (uintmax_t pos = 0; pos < 4 ; ++pos, --shift) {
        uintmax_t mode = (src >> shift) & mask;
        if (mode)
            dst[pos] = sym[pos]; 
        else 
            dst[pos] = '-';
    }

    dst[3] = '\0';
}

void parse_and_out_mode(const struct stat st) {
    int d = 0100;
    for (uintmax_t mask = 0700; mask > 0; mask >>= 03) {
        const uintmax_t mode_t = st.st_mode & mask;
        char mode_str[4];
        uintmax_to_str_mode(mode_t/d, mode_str);
        printf("%s", mode_str);
        d /= 010;
    }
}

void get_uid_and_print_u(const struct stat st) {
    struct passwd *uid = getpwuid(st.st_uid);
    printf(" %s", uid->pw_name);
}

void parse_and_out_size(const struct stat st) {
    off_t d_size = st.st_size;
    double dd_size = (double) st.st_size;
    if (dd_size >= 10000) {
        while (dd_size >= 100)
            dd_size /= 10;
        printf("  %4.0fk", dd_size);
    } else if (dd_size >= 1000) {
        while (dd_size >= 10)
            dd_size /= 10;
        printf("  %4.1fk", dd_size);
    } else if (dd_size == 0) {
        printf("\t-");
    }else {
        printf("  %5d", st.st_size);
    }
}

char *int_month_to_str(const int month) {
    switch (month) {
        case 1:
            return "Jan";
        case 2:
            return "Feb";
        case 3:
            return "Mar";
        case 4:
            return "Apr";
        case 5:
            return "May";
        case 6:
            return "Jun";
        case 7:
            return "Jul";
        case 8:
            return "Aug";
        case 9:
            return "Sep";
        case 10:
            return "Oct";
        case 11:
            return "Nov";
        case 12:
            return "Dec";
    }
}

int main(int argc, char *argv[]) {
    
    DIR             *dir;
    struct dirent   *dr;
    struct stat     st;
    char            *path;

    if (argc >= 2) {
        path = argv[1];
        dir = opendir(path);
    }
    else {
        path = ".";
        dir = opendir(path);
    }
    check_on_error();


    while ((dr = readdir(dir)) != NULL) {
        check_on_error();

        char temp_d[sizeof(char)*strlen(path)*strlen(dr->d_name)+2];

        strcpy(temp_d, path);
        strcat(temp_d, "/");
        strcat(temp_d, dr->d_name);

        if (!stat(temp_d, &st)) {
            printf("%c", st.st_mode & S_IFDIR ? 'd' : '.');

            parse_and_out_mode(st);

            parse_and_out_size(st);

            get_uid_and_print_u(st);

            struct tm *time = gmtime(&st.st_ctime);
            char *mon_s = int_month_to_str(time->tm_mon);
            printf(" %2d %s %2d:%2d", time->tm_mday, mon_s, time->tm_hour, time->tm_min);
                        
            printf(" %s\n", dr->d_name);
        } else {
            check_on_error();
        }
    }
    
    return 0;
}
