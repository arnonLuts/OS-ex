#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

void copy_file(const char *src, const char *dest, int copy_symlinks, int copy_permissions)
{
    struct dirent *entry;
    DIR *dp;
    struct stat st;
    if (copy_symlinks)
    {
        if (!lstat(src, st))
        {
            if (S_ISLNK(st.st_mode))
            {
                char linked_file[strlen(src) + 1];
                ssize_t len = readlink(src, linked_file, sizeof(linked_file) - 1);
                if (len < 0)
                {
                    perror("readlink error");
                    return;
                }
                else
                {
                    linked_file[len] = "\0";
                    if (symlink(linked_file, dest) < 0)
                    {
                        perror("symlink error");
                    }
                    return;
                }
            }
        }
        else
        {
            perror("lstat error");
            return;
        }
    }
    if (!link(src, dest))
    {
        if (copy_permissions)
        {
            if (chmod(dest, st.st_mode) == -1)
            {
                perror("chmod failed");
            }
        }
    }
    else
    {
        perror("link error");
        return;
    }
}
void create_directory(const char *dir_name)
{
    int len = strlen(dir_name);
    char dir[len];
    struct stat st = {0};
    strcpy(dir, dir_name);
    if (dir[len - 1] == '/')
    {
        dir[len - 1] = 0;
    }
    for (char *p = dir + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;

            if (stat(dir, &st) == -1)
            {
                if (mkdir(dir, S_IRWXU) != 0)
                {
                    perror("mkdir failed");
                    return;
                }
            }

            *p = '/';
        }
    }
    if (stat(dir, &st) == -1)
    {
        if (mkdir(dir, S_IRWXU) != 0)
        {
            perror("mkdir failed");
            return;
        }
    }
}
void copy_directory(const char *src, const char *dest, int copy_symlinks, int copy_permissions)
{
    struct dirent *entry;
    DIR *dp;
    struct stat st;
    dp = opendir(src);
    if (dp == NULL)
    {
        perror("opendir error");
        return -1;
    }
    create_directory(dest);
    while ((entry = readdir(dp)) != NULL)
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }
        char *src_path, dest_path;
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);
        if (!lstat(src_path, &st))
        {
            if (S_ISDIR(st.st_mode))
            {
                copy_directory(src_path, dest_path, copy_symlinks, copy_permissions);
            }
            else
            {
                copy_file(src_path, dest_path, copy_symlinks, copy_permissions);
            }
        }
        else
        {
            perror("lstat failed");
        }
    }
    closedir(dp);
}