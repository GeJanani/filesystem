#ifndef EXT2_NODE_H
#define EXT2_NODE_H
#include "ext2.h"

#include "Ext2Filesystem.h"
/** A node in an ext2 filesystem. */
class Ext2Node {

    public:
        /** Constructor, should be called only by a Filesystem. */

        Ext2Node ( size_t inode_num, Inode * pInode, class Ext2Filesystem * pFs );
        /** Destructor */
        virtual ~Ext2Node();

        Inode * getInode() {
            return m_pInode;
        }

        uint32_t getInodeNumber() {
            return m_InodeNumber;
        }

        /** Updates inode attributes. */
        void fileAttributeChanged ( size_t size );

        uint64_t doRead ( uint64_t location, uint64_t size, uintptr_t buffer );
        uint64_t doWrite ( uint64_t location, uint64_t size, uintptr_t buffer );

        /** Wipes the node of data - frees all blocks. */
        void wipe();

        uintptr_t readBlock ( uint64_t location );
        void writeBlock ( uint64_t location );

        void trackBlock ( uint32_t block );
        static Genode::Allocator * m_alloc;
    protected:
        /** Ensures the inode is at least 'size' big. */
        bool ensureLargeEnough ( size_t size );

        bool addBlock ( uint32_t blockValue );

        bool ensureBlockLoaded ( size_t nBlock );
        bool getBlockNumber ( size_t nBlock );

        bool setBlockNumber ( size_t blockNum, uint32_t blockValue );

        Inode * m_pInode;
        uint32_t m_InodeNumber;
        class Ext2Filesystem * m_pExt2Fs;

        uint32_t * m_pBlocks;
        uint32_t m_nBlocks;

        size_t m_nSize;

};

#endif
