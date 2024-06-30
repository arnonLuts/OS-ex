#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>

void get_directory_path(const char *file_path, char *dir_path) {
    const char *last_slash = strrchr(file_path, '/');
    if (last_slash == NULL) {
        strcpy(dir_path, ".");
    } else {
        size_t dir_length = last_slash - file_path + 1;
        strncpy(dir_path, file_path, dir_length);
        dir_path[dir_length] = '\0';
    }
}

void copy_file(const char *src, const char *dest, int copy_symlinks, int copy_permissions) {
    struct stat st;

    if (lstat(src, &st) == -1) {
        perror("lstat error");
        return;
    }

    if (S_ISLNK(st.st_mode)) {
        char linked_file[st.st_size + 1];
        ssize_t len = readlink(src, linked_file, sizeof(linked_file) - 1);
        if (len < 0) {
            perror("readlink error");
            return;
        } else {
            linked_file[len] = '\0';
            if (copy_symlinks) {
                if (symlink(linked_file, dest) < 0) {
                    perror("symlink error");
                }
                return;
            } else {
                char dir_path[PATH_MAX];
                get_directory_path(src, dir_path);
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s%s", dir_path, linked_file);

                const char *file_name = strrchr(linked_file, '/');
                if (file_name == NULL) {
                    file_name = linked_file;  
                } else {
                    file_name++; 
                }
                char dest_path[PATH_MAX];
                get_directory_path(dest, dest_path);
                char new_dest[PATH_MAX];
                snprintf(new_dest, sizeof(new_dest), "%s%s", dest_path, file_name);

                if (!link(full_path, new_dest)) {
                    if (copy_permissions) {
                        if (chmod(dest, st.st_mode) == -1) {
                            perror("chmod failed");
                        }
                    }
                } else {
                    perror("link error");
                    return;
                }
                return;
            }
        }
    }

    if (!link(src, dest)) {
        if (copy_permissions) {
            if (chmod(dest, st.st_mode) == -1) {
                perror("chmod failed");
            }
        }
    } else {
        perror("link error");
        return;
    }
}


void create_directory(const char *dir_name){
    int len = strlen(dir_name);
    char dir[len + 1];
    struct stat st = {0};
    
    strcpy(dir, dir_name);
    if (dir[len - 1] == '/'){
        dir[len - 1] = 0;
    }
    for (char *p = dir + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (stat(dir, &st) == -1) {
                if (mkdir(dir, S_IRWXU) != 0) {
                    perror("mkdir failed");
                    return;
                }
            }
            *p = '/';
        }
    }
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, S_IRWXU) != 0) {
            perror("mkdir failed");
            return;
        }
    }
}

void copy_directory(const char *src, const char *dest, int copy_symlinks, int copy_permissions){
    struct dirent *entry;
    DIR *dp;
    struct stat st;
    struct stat st_tmp;
    DIR *dir_tmp = opendir(dest);
    if (dir_tmp) {
        if ((stat(dest, &st_tmp) == 0)) {
        struct dirent *entry;
        while ((entry = readdir(dir_tmp)) != NULL) {
            if (entry->d_name[0] != '.' || (entry->d_name[1] != '\0' && (entry->d_name[1] != '.' || entry->d_name[2] != '\0'))) {
                fprintf(stderr, "Directory is not empty");  
                return;
            }

            }
        }
        closedir(dir_tmp);
    }
    dp = opendir(src);
    if(dp == NULL){
        perror("opendir error");
        return;
    }
    create_directory(dest);
    while ((entry = readdir(dp)) != NULL)
    {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")){
            continue;
        }
        char src_path[PATH_MAX+1];
        char dest_path[PATH_MAX+1];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);
        if (!lstat(src_path, &st)) {
            if (S_ISDIR(st.st_mode)) {
                copy_directory(src_path, dest_path, copy_symlinks, copy_permissions);
            } else {
                copy_file(src_path, dest_path, copy_symlinks, copy_permissions);
            }
        } else {
            perror("lstat failed");
        }
    }
    closedir(dp);
}
