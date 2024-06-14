#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_PATH_LENGTH 1024
#define MAX_HEADER_LENGTH 256
#define MAX_FILES 1000

// Structure to store header inclusion information
typedef struct {
    char header[MAX_HEADER_LENGTH];
    char files[MAX_FILES][MAX_PATH_LENGTH];
    int file_count;
} HeaderInfo;

// Function prototypes
void analyze_project(const char *project_path);
void process_file(const char *filepath, HeaderInfo *included_headers);
void list_unused_and_missing_headers(const HeaderInfo *included_headers);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <path_to_C_project>\n", argv[0]);
        return 1;
    }

    analyze_project(argv[1]);

    return 0;
}

void analyze_project(const char *project_path) {
    HeaderInfo included_headers[MAX_FILES] = {0};

    DIR *dir = opendir(project_path);
    if (!dir) {
        perror("Error opening directory");
        exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && (strstr(entry->d_name, ".c") || strstr(entry->d_name, ".h"))) {
            char filepath[MAX_PATH_LENGTH];
            snprintf(filepath, sizeof(filepath), "%s/%s", project_path, entry->d_name);
            process_file(filepath, included_headers);
        }
    }

    closedir(dir);

    list_unused_and_missing_headers(included_headers);
}

void process_file(const char *filepath, HeaderInfo *included_headers) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    char line[MAX_HEADER_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "#include <", strlen("#include <")) == 0) {
            char header[MAX_HEADER_LENGTH];
            sscanf(line, "#include <%[^>]>", header);

            for (int i = 0; i < MAX_FILES; ++i) {
                if (strlen(included_headers[i].header) == 0) {
                    strcpy(included_headers[i].header, header);
                    strcpy(included_headers[i].files[0], filepath);
                    included_headers[i].file_count++;
                    break;
                } else if (strcmp(included_headers[i].header, header) == 0) {
                    strcpy(included_headers[i].files[included_headers[i].file_count], filepath);
                    included_headers[i].file_count++;
                    break;
                }
            }
        }
    }

    fclose(file);
}

void list_unused_and_missing_headers(const HeaderInfo *included_headers) {
    printf("Unused headers:\n");
    for (int i = 0; i < MAX_FILES; ++i) {
        if (strlen(included_headers[i].header) > 0 && included_headers[i].file_count == 1) {
            printf("%s\n", included_headers[i].header);
        }
    }

    printf("\nMissing headers:\n");
    for (int i = 0; i < MAX_FILES; ++i) {
        if (strlen(included_headers[i].header) > 0) {
            struct stat st;
            if (stat(included_headers[i].header, &st) == -1) {
                printf("%s\n", included_headers[i].header);
            }
        }
    }
}