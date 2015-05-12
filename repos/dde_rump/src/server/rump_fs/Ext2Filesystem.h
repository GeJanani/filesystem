#ifndef EXT2FILESYSTEM_H
#define EXT2FILESYSTEM_H
#include <file_system_session/rpc_object.h>
#include <os/server.h>
#include <os/session_policy.h>
#include <root/component.h>
extern "C" {
#include <sys/cdefs.h>
#include <sys/types.h>
}
#include <os/config.h>
#include <os/path.h>

#include "file_system.h"
#include "Filesystem.h"
#include "ext2.h"

/** This class provides an implementation of the second extended filesystem. */
class Ext2Filesystem : public Filesystem {
        friend class Ext2File;
        friend class Ext2Node;
        friend class Ext2Directory;



    public:
        Ext2Filesystem ( Genode::Allocator & md_alloc );

        virtual ~Ext2Filesystem();


        virtual bool initialise();

        Genode::Allocator & m_alloc;

        char * ext2allocateMemory ( size_t size );
        void ext2releaseMemory ( char * temp, size_t size );


        virtual File * getRoot();


        /** Size of a block. */
        uint32_t m_BlockSize;
    protected:
        virtual bool createFile ( File * parent, char * filename, uint32_t mask );
        virtual bool createDirectory ( File * parent, char * filename );

        virtual bool remove ( File * parent, File * file );

    private:
        virtual bool createNode ( File * parent, char * filename, uint32_t mask, char * value, size_t type );


        /** Reads a block of data from the disk. */
        char * readBlock ( uint32_t block );
        /** Writes a block of data to the disk. */
        void writeBlock ( uint32_t block, char * buff );

        uint32_t findFreeBlock ( uint32_t inode );
        uint32_t findFreeInode();

        void releaseBlock ( uint32_t block );
        /** Releases the given inode, returns true if the inode had no more links. */
        bool releaseInode ( uint32_t inode );

        Inode * getInode ( uint32_t num );
        void writeInode ( uint32_t num );

        void ensureFreeBlockBitmapLoaded ( size_t group );
        void ensureFreeInodeBitmapLoaded ( size_t group );
        void ensureInodeTableLoaded ( size_t group );

        bool checkOptionalFeature ( size_t feature );
        bool checkRequiredFeature ( size_t feature );
        bool checkReadOnlyFeature ( size_t feature );

        void increaseInodeRefcount ( uint32_t inode );
        bool decreaseInodeRefcount ( uint32_t inode );

        /** Our superblock. */
        Superblock * m_pSuperblock;

        /** Group descriptors, in a tree because each GroupDesc* may be in a different block. */
        GroupDesc ** m_pGroupDescriptors;


        char ** m_pInodeTables;
        char ** m_pInodeBitmaps;
        char ** m_pBlockBitmaps;

        /** Size of an Inode. */
        uint32_t m_InodeSize;

        /** Number of group descriptors. */
        size_t m_nGroupDescriptors;

        //In testing only two groups so size of 2 later can be expaned
        char Ibitmap[2];
        char Bbitmap[2];
        char Itable[2];


        /** The root filesystem node. */
        File * m_pRoot;

};



#endif
