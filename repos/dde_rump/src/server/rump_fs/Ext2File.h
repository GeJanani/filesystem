#ifndef EXT2_FILE_H
#define EXT2_FILE_H

#include "ext2.h"
#include "Ext2Node.h"
#include "File.h"
#include "Ext2Filesystem.h"

/** A File is a file, a directory or a symlink. */
class Ext2File : public File, public Ext2Node {
    public:
        /** Constructor, should be called only by a Filesystem. */
        Ext2File ( char * name, size_t inode_num, Inode * inode,
                   class Ext2Filesystem * pFs, File * pParent = 0 );
        /** Destructor */
        virtual ~Ext2File();

        virtual void extend ( size_t newSize );

        virtual void truncate();

        /** Updates inode attributes. */
        void fileAttributeChanged();





    protected:

        size_t readBlock ( uint64_t location );
        void writeBlock ( uint64_t location, uintptr_t addr );

        size_t getBlockSize() const {
            return reinterpret_cast<Ext2Filesystem *> ( m_pFilesystem )->m_BlockSize;
        }
};

#endif
