#ifndef DIRECTORY_H
#define DIRECTORY_H
#include "File.h"
#include "RadixTree.h"
/** A Directory node. */
class Directory : public File {
        friend class Filesystem;

    public:

        /** Eases the pain of casting, and performs a sanity check. */
        static Directory * fromFile ( File * pF ) {
            if ( !pF->isDirectory() ) {
                PERR ( "not a directory" );
            }

            return reinterpret_cast<Directory *> ( pF );
        }

        /** Constructor, creates an invalid directory. */
        Directory();


    public:

        /** Constructor, should be called only by a Filesystem. */
        Directory ( char * name, size_t inode, class Filesystem * pFs, size_t size, File * pParent );
        /** Destructor - doesn't do anything. */
        virtual ~Directory();

        /** Returns true if the File is actually a directory. */
        virtual bool isDirectory() {
            return true;
        }

        /** Returns the n'th child of this directory, or an invalid file. */
        File * getChild ( size_t n );

        /** Load the directory's contents into the cache. */
        virtual void cacheDirectoryContents();

    public:
        /** Directory contents cache. */
        RadixTree<File *> m_Cache;
        bool m_bCachePopulated;
};

#endif
