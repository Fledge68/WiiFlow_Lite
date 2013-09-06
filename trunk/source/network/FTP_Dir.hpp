/****************************************************************************
 * Copyright (C) 2013 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef _FTP_DIR_H_
#define _FTP_DIR_H_

#include <sys/reent.h>
#include <sys/dirent.h>
#include <sys/unistd.h>
#include <gctypes.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void ftp_init(void);
const char *ftp_getpath(void);

DIR *ftp_diropen();
void ftp_dirclose(DIR *dir);
int ftp_dirnext(DIR *dir, char *name);
int ftp_changedir(char *path);
int ftp_makedir(char *path);

FILE *ftp_fopen(char *path, char *type);
void ftp_fclose(FILE *fp);

int ftp_stat(char *file, struct stat *st);
int ftp_rename(char *path, char *new_name);
int ftp_delete(char *path);

bool ftp_startThread(void);
void ftp_endTread(void);

void ftp_dbg_print(char *dbg_info);
bool ftp_dbg_print_update(void);
const char *ftp_get_prints(u8 i);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
