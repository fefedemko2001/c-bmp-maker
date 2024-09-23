#include <stdio.h>
#include <dirent.h>
#include <ctype.h>

int main()
{
    DIR *dir;
    struct dirent *entry;
    char buffer[1024];
    dir = opendir("/proc");
    while ((entry = readdir(dir)))
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)continue;
        char sub_path[1024];
        snprintf(sub_path, sizeof(sub_path), "%s/%s", "/proc", entry->d_name);
        if (isdigit(entry->d_name[0]))
        {

        }
            
    }


    return 0;
}