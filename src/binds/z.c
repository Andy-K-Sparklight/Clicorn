#include "z.h"
#include "miniz.h"
#include "fs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int inflateGZ(const unsigned char *buf, unsigned long sourceLength, unsigned char *target, unsigned long *targetLength)
{
    int i;
    if ((i = uncompress(target, targetLength, buf, (mz_uint32)sourceLength)) == Z_OK)
    {
        return 0;
    }
    else
    {
        printf("%d\n", i);
        return -1;
    }
}

int unZip(const char *origin, const char *dest)
{
#ifdef WIN32
    char delimiter = '\\';
    char delimiterStr[] = "\\";
#else
    char delimiter = '/';
    char delimiterStr[] = "/";
#endif
    char sCwd[4096];
    if (getcwd(sCwd, 4096) == NULL)
    {
        return -1;
    }
    ensureDir(dest);

    mz_zip_archive zipFile;
    memset(&zipFile, 0, sizeof(mz_zip_archive));
    FILE *f = fopen(origin, "rb");
    if (!f)
    {
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    unsigned char *buf = malloc(sz * sizeof(unsigned char));
    if (fread(buf, sizeof(unsigned char), sz, f) != sz)
    {
        free(buf);
        return -1;
    }
    fclose(f);
    mz_bool status = mz_zip_reader_init_mem(&zipFile, buf, sz, 0);
    if (chdir(dest) != 0)
    {
        free(buf);
        return -1;
    }
    if (!status)
    {

        if (chdir(sCwd) != 0)
        {
            printf("Failed to chdir.");
        }
        free(buf);
        return -1;
    }

    for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zipFile); i++)
    {
        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&zipFile, i, &fileStat))
        {
            mz_zip_reader_end(&zipFile);
            if (chdir(sCwd) != 0)
            {
                printf("Failed to chdir.");
            }
            free(buf);
            return -1;
        }
        ensureDir(fileStat.m_filename);
        mz_zip_reader_extract_file_to_file(&zipFile, fileStat.m_filename, fileStat.m_filename, 0);
    }
    mz_zip_reader_end(&zipFile);
    if (chdir(sCwd) != 0)
    {
        printf("Failed to chdir.");
    }
    free(buf);
    return 0;
}
