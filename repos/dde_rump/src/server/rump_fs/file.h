/**
 * \brief  File node
 * \author Norman Feske
 * \author Christian Helmuth
 * \auhtor Sebastian Sumpf
 * \date   2013-11-11
 */

/*
 * Copyright (C) 2013-2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _FILE_H_
#define _FILE_H_

#include "node.h"
#include "util.h"

namespace File_system {
    class File;
}

class File_system::File : public Node {
    private:

        int _fd;

        int _access_mode ( Mode const & mode ) {
            switch ( mode ) {
                case STAT_ONLY:
                case READ_ONLY:
                    return O_RDONLY;

                case WRITE_ONLY:
                    return O_WRONLY;

                case READ_WRITE:
                    return O_RDWR;

                default:
                    return O_RDONLY;
            }
        }

        unsigned long _inode ( int dir, char const * name, bool create ) {
            return 4;
        }

        unsigned long _inode_path ( char const * path ) {
            return 4;
        }

        int _open ( int dir, char const * name, Mode mode ) {
            return 4;
        }

        int _open_path ( char const * path, Mode mode ) {
            return 4;
        }

    public:

        File ( int         dir,
               char const * name,
               Mode        mode,
               bool        create )
            : Node ( _inode ( dir, name, create ) ),
              _fd ( _open ( dir, name, mode ) ) {
            Node::name ( name );
        }

        File ( char const * path, Mode mode )
            :
            Node ( _inode_path ( path ) ),
            _fd ( _open_path ( path, mode ) ) {
            Node::name ( basename ( path ) );
        }

        virtual ~File() {  }

        size_t read ( char * dst, size_t len, seek_off_t seek_offset ) {
            ssize_t ret = 0;
            return ret == -1 ? 0 : ret;
        }

        size_t write ( char const * src, size_t len, seek_off_t seek_offset ) {
            return 0;
        }

        file_size_t length() const {
            return 0;
        }

        void truncate ( file_size_t size ) {
        }
};

#endif /* _FILE_H_ */
