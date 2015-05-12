#ifndef EXT2_DIRECTORY_H
#define EXT2_DIRECTORY_H

#include "ext2.h"
#include <base/allocator_avl.h>
#include "Ext2Node.h"
#include "Directory.h"
#include "Ext2Filesystem.h"

/** A File is a file, a directory */
class Ext2Directory : public Directory, public Ext2Node {
    public:
        /** Constructor, should be called only by a Filesystem. */
        Ext2Directory ( char * name, size_t inode_num, Inode * inode,
                        class Ext2Filesystem * pFs, File * pParent );
        /** Destructor */
        virtual ~Ext2Directory();

        uint64_t read ( uint64_t location, uint64_t size, uintptr_t buffer ) {
            return 0;
        }
        uint64_t write ( uint64_t location, uint64_t size, uintptr_t buffer ) {
            return 0;
        }

        void truncate() {
        }

        /** Reads directory contents into File* cache. */
        virtual void cacheDirectoryContents();

        /** Adds a directory entry. */
        virtual bool addEntry ( char * filename, File * pFile, size_t type );
        /** Removes a directory entry. */
        virtual bool removeEntry ( char * filename, Ext2Node * pFile );

        /** Updates inode attributes. */
        void fileAttributeChanged();
        static Genode::Allocator * m_alloc;
        bool listEntries();

};

#endif
