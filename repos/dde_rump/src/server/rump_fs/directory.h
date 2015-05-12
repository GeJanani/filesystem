/**
 * \brief  File-system directory node
 * \author Norman Feske
 * \author Christian Helmuth
 * \author Sebastian Sumpf
 * \date   2013-11-11
 */

/*
 * Copyright (C) 2013-2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _DIRECTORY_H_
#define _DIRECTORY_H_

/* Genode include */
#include <os/path.h>

/* local includes */
#include "node.h"
#include "util.h"
#include "file.h"
#include "symlink.h"

namespace File_system {
    class Directory;
}

class File_system::Directory : public Node {
    public:

        enum { BUFFER_SIZE = 4096 };

        typedef Genode::Path<MAX_PATH_LEN> Path;

        int        _fd;
        Path       _path;
        Allocator & _alloc;

        unsigned long _inode ( char const * path, bool create ) {
            return 5;
        }

        int _open ( char const * path ) {
            /*struct stat s;
            int ret = rump_sys_lstat(path, &s);
            if (ret == -1 || !S_ISDIR(s.st_mode))
            	throw Lookup_failed();

            int fd = rump_sys_open(path, O_RDONLY);
            if (fd == -1)
            	throw Lookup_failed();

            return fd;*/
            return 23;
        }

        static char * _buffer() {
            /* buffer for directory entries */
            static char buf[BUFFER_SIZE];
            return buf;
        }

    public:

        Directory ( Allocator & alloc, char const * path, bool create )
            :
            Node ( _inode ( path, create ) ),
            _fd ( _open ( path ) ),
            _path ( path, "./" ),
            _alloc ( alloc ) {
            //PERR("dir start\n");
            Node::name ( basename ( path ) );
            //PERR("dir conss\n");
        }

        virtual ~Directory() {
        }

        File * file ( char const * name, Mode mode, bool create ) {
            //PERR("READ3\n");
            return new ( &_alloc ) File ( _fd, name, mode, create );
        }

        Symlink * symlink ( char const * name, bool create ) {
            return new ( &_alloc ) Symlink ( _path.base(), name, create );
        }

        Directory * subdir ( char const * path, bool create ) {
            //PERR("i am in \n");
            Path dir_path ( path, _path.base() );
            //PERR("subdir start\n");
            Directory * dir = new ( &_alloc ) Directory ( _alloc, dir_path.base(), create );
            //PERR("done\n");
            return dir;
        }

        Node * node ( char const * path ) {
            Path node_path ( path, _path.base() );
            //	struct stat s;
            Node * node = 0;
            node = new ( &_alloc ) Directory ( _alloc, node_path.base(), false );
            return node;
        }

        size_t read ( char * dst, size_t len, seek_off_t seek_offset ) {
            //PERR("READ\n");
            return 0;
        }

        size_t write ( char const * src, size_t len, seek_off_t seek_offset ) {
            /* writing to directory nodes is not supported */
            //PERR("READ2\n");
            return 0;
        }

        size_t num_entries() const {
            return 0;
        }

        void unlink ( char const * path ) {
            //PERR("Error during unlink of %s", node_path.base());
        }
};

#endif /* _DIRECTORY_H_ */
