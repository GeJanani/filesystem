/**
 * \brief  Rump initialization and required header
 * \author Sebastian Sumpf
 * \date   2014-01-17
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */
#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

extern "C" {
#include <sys/cdefs.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
}

#include <base/signal.h>
#include <os/server.h>

namespace File_system {
    void init ( Server::Entrypoint & ep );
    bool supports_symlinks();
}
void readb ( int, int, char * );
void writeb ( int, int, char * );

extern int errno;

extern char buff[1025];
#endif /* _FILE_SYSTEM_H_ */
