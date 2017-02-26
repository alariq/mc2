#include<dirent.h>

    HGOSAUDIO hgosaudio = NULL;
    char music_buf[256];

    DIR *d;
    struct dirent *dir;
    d = opendir("./data/sound");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            sprintf(music_buf, "./data/sound/%s", dir->d_name);
            if(strstr(dir->d_name, ".wav")) {
                printf("%s\n", dir->d_name);
                gosAudio_CreateResource(&hgosaudio, gosAudio_StreamedFile, music_buf);
            }
        }

    }
    closedir(d);
