#include "Ext2Filesystem.h"
#include "Ext2File.h"



Ext2File::Ext2File ( char * name, size_t inode_num, Inode * inode,
                     Ext2Filesystem * pFs, File * pParent ) :
    File ( name, inode_num,
           static_cast<Filesystem *> ( pFs ),
           inode->i_size, /// \todo Deal with >4GB files here.
           pParent ),
    Ext2Node ( inode_num, inode, pFs )
{
    // Enable cache writebacks for this file.
    uint32_t mode = inode->i_mode;
    uint32_t permissions = 0;

    if ( mode & EXT2_S_IRUSR )
    {
        permissions |= FILE_UR;
    }

    if ( mode & EXT2_S_IWUSR )
    {
        permissions |= FILE_UW;
    }

    if ( mode & EXT2_S_IXUSR )
    {
        permissions |= FILE_UX;
    }

    if ( mode & EXT2_S_IRGRP )
    {
        permissions |= FILE_GR;
    }

    if ( mode & EXT2_S_IWGRP )
    {
        permissions |= FILE_GW;
    }

    if ( mode & EXT2_S_IXGRP )
    {
        permissions |= FILE_GX;
    }

    if ( mode & EXT2_S_IROTH )
    {
        permissions |= FILE_OR;
    }

    if ( mode & EXT2_S_IWOTH )
    {
        permissions |= FILE_OW;
    }

    if ( mode & EXT2_S_IXOTH )
    {
        permissions |= FILE_OX;
    }
}

Ext2File::~Ext2File()
{
}

void Ext2File::extend ( size_t newSize )
{
    if ( newSize > m_Size )
    {
        ensureLargeEnough ( newSize );
        m_Size = newSize;
    }
}

size_t Ext2File::readBlock ( uint64_t location )
{
    /*
        m_FileBlockCache.startAtomic();
        uintptr_t buffer = m_FileBlockCache.insert(location);
        static_cast<Ext2Node*>(this)->doRead(location, getBlockSize(), buffer);

        // Clear any dirty flag that may have been applied to the buffer
        // by performing this read. Cache uses the dirty flag to figure
        // out whether or not to write the block back to disk...
        VirtualAddressSpace &va = Processor::information().getVirtualAddressSpace();
        for (size_t off = 0; off < getBlockSize(); off += PhysicalMemoryManager::getPageSize())
        {
            void *p = reinterpret_cast<void *>(buffer + off);
            if(va.isMapped(p))
            {
                physical_uintptr_t phys = 0;
                size_t flags = 0;
                va.getMapping(p, phys, flags);

                if(flags & VirtualAddressSpace::Dirty)
                {
                    flags &= ~(VirtualAddressSpace::Dirty);
                    va.setFlags(p, flags);
                }
            }
        }
        m_FileBlockCache.endAtomic();
    */
    return 0;
}

void Ext2File::writeBlock ( uint64_t location, size_t addr )
{
    // Don't accidentally extend the file when writing the block.
    size_t sz = getBlockSize();
    uint64_t end = location + sz;

    if ( end > getSize() )
    {
        sz = getSize() - location;
    }

    static_cast<Ext2Node *> ( this )->doWrite ( location, sz, addr );
}

void Ext2File::truncate()
{
    // Wipe all our blocks. (Ext2Node).
    Ext2Node::wipe();
    m_Size = m_nSize;
}

void Ext2File::fileAttributeChanged()
{
    static_cast<Ext2Node *> ( this )->fileAttributeChanged ( m_Size );
}

