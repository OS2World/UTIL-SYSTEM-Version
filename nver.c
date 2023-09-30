/****************************************************************************
 *
 *  ver.c -- OS/2 - ECS - ArcaOS version command 
 *
 *  ========================================================================
 *
 *    Version 1.0       Michael K Greene <mikeos2@mail.com>
 *                      September 2023
 *
 *  ========================================================================
 *
 *  Description: Simple version command for OS/2 - ArcaOS
 *
 *  ========================================================================
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

#define INCL_DOSMISC

#include <os2.h>
#include <stdio.h>
#include <stdlib.h> //getenv
#include <string.h>
#include <ctype.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>

#define VER_BUFFER_SIZE 20

#pragma pack(push, 1)

typedef struct _SYSLEVELDATA
{
    unsigned char d_reserved1[2];
    unsigned char d_kind;
    unsigned char d_version[2];
    unsigned char d_reserved2[2];
    unsigned char d_clevel[7];
    unsigned char d_reserved3;
    unsigned char d_plevel[7];
    unsigned char d_reserved4;
    unsigned char d_title[80];
    unsigned char d_cid[9];
    unsigned char d_revision;
    unsigned char d_type[1];
} SYSLEVELDATA;

typedef struct _SYSLEVELHEADER
{
    unsigned char h_magic[2];
    unsigned char h_name[9];
    unsigned char h_reserved1[4];
    unsigned char h_updated;
    unsigned char h_reserved2[17];
    unsigned long h_data;
} SYSLEVELHEADER;

void SysECSVer(void);
void SysArcaVer(char *syspath);
void SysOS2Ver(void);

void main(void)
{
    // path to OSDIR - I expect this only on ArcaOS
    char *syspath = getenv("OSDIR");

    // I assume if OSDIR does not exist then we are not running on
    // ArcaOS so I will head to SysECSVer and try there. If that
    // fails then the return will be from SysOS2Ver
    if (syspath == NULL)
    {
        SysECSVer(); // no OSDIR - try ECS
    }
    else
        SysArcaVer(syspath); // must be ArcaOS

    return;
}

void SysECSVer(void)
{
    int handle;

    unsigned long BootDrive[1];
    unsigned long Versions[2];

    char lvlpath[CCHMAXPATH] = {0};

    SYSLEVELHEADER *lvlheader = NULL;
    SYSLEVELDATA *lvldata = NULL;

    DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, BootDrive, sizeof(BootDrive));

    sprintf(lvlpath, "%c:\\OS2\\INSTALL\\SYSLEVEL.ECS", (char)(BootDrive[0] + 64));

    // if cannot find SYSLEVEL.ECS fall through to SysOS2Ver
    if (access(lvlpath, F_OK) == -1)
        SysOS2Ver();
    else
    {

        char *buffer = calloc(filelength(handle), sizeof(char));

        handle = open(lvlpath, O_RDONLY | O_BINARY);
        read(handle, buffer, filelength(handle));
        close(handle);

        lvlheader = (SYSLEVELHEADER *)buffer;
        lvldata = (SYSLEVELDATA *)&(buffer[lvlheader->h_data]);

        Versions[0] = lvldata->d_version[0] >> 4;
        Versions[1] = lvldata->d_version[0] & 15;

        printf("eCS %lu.%02lu\n", Versions[0], Versions[1]);

        free(buffer);
    }
}

void SysArcaVer(char *syspath)
{
    char arcafile[CCHMAXPATH] = {0};
    char buffer[VER_BUFFER_SIZE];
    char *arcaflg = "\\install\\install.flg";

    strncpy(arcafile, syspath, strlen(syspath));
    strncat(arcafile, arcaflg, strlen(arcaflg));

    // if cannot find install.flg fall through to SysOS2Ver
    if (access(arcafile, F_OK) == 0)
    {
        // found install file
        FILE *fp = fopen(arcafile, "r");
        fgets(buffer, VER_BUFFER_SIZE, fp);
        fclose(fp);

        buffer[strcspn(buffer, "\n")] = 0;

        printf("%s", buffer);
    }
    else
        SysOS2Ver();
}

void SysOS2Ver(void)
{
    unsigned long Versions[2]; /* Major version number       */

    DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_MINOR,
                    Versions, sizeof(Versions));

    printf("OS/2 %lu.%02lu", (Versions[0] / 10), Versions[1]);
}
