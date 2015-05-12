#include "Ext2Filesystem.h"
#include "Ext2Directory.h"
#include "Ext2File.h"

Ext2Filesystem::Ext2Filesystem ( Genode::Allocator & md_alloc ) :
    m_alloc ( md_alloc ), m_pSuperblock ( 0 ), m_pGroupDescriptors(), m_BlockSize ( 0 ), m_InodeSize ( 0 ),
    m_nGroupDescriptors ( 0 )
{
}

Ext2Filesystem::~Ext2Filesystem()
{
    if ( m_pRoot )
    {
        m_alloc.free ( ( void * ) m_pRoot, sizeof ( File * ) );
    }
}

bool Ext2Filesystem::initialise()
{
    m_pSuperblock = new ( &m_alloc ) Superblock;
    readb ( 2, 2, buff );
    Genode::memcpy ( ( void * ) m_pSuperblock, ( void * ) buff, 1024 );
    PDBG ( "Ext2 Magic Number :: %x\n", m_pSuperblock->s_magic );
    PDBG ( " Ext2 rev level :: %lu\n", m_pSuperblock->s_rev_level );
    // Read correctly?

    // Clean?
    if ( m_pSuperblock->s_state != 1 )
    {
        PINF ( "Ext2: filesystem   is not clean." );
    }

    m_InodeSize = sizeof ( Inode );
    PDBG ( "Ext2 :: block size %d :: First data block   %lu  \n", 1024 << m_pSuperblock->s_log_block_size, m_pSuperblock->s_first_data_block );
    // Calculate the block size.
    m_BlockSize = 1024 << m_pSuperblock->s_log_block_size;
    // Where is the group descriptor table?
    uint32_t gdBlock = m_pSuperblock->s_first_data_block + 1;
    // How many group descriptors do we have? Round up the result.
    uint32_t inodeCount = m_pSuperblock->s_inodes_count;
    PDBG ( "Ext2 Free Inodes Count :: %lu\n", m_pSuperblock->s_free_inodes_count );
    uint32_t inodesPerGroup = m_pSuperblock->s_inodes_per_group;
    m_nGroupDescriptors = ( inodeCount / inodesPerGroup ) + ( inodeCount % inodesPerGroup );
    // Add an entry to the group descriptor tree for each GD.
    PDBG ( "\nExt2 Inodes per group:: %lu Inode Count  %lu :: Inode size  %lu \n", inodesPerGroup, inodeCount, m_InodeSize );
    m_pGroupDescriptors = new ( &m_alloc ) GroupDesc*[m_nGroupDescriptors];

    for ( size_t i = 0; i < m_nGroupDescriptors; i++ )
    {
        uintptr_t idx = ( i * sizeof ( GroupDesc ) ) / m_BlockSize;
        uintptr_t off = ( i * sizeof ( GroupDesc ) ) % m_BlockSize;
        //uintptr_t block = readBlock(gdBlock+idx);
        //m_pGroupDescriptors[i] = reinterpret_cast<GroupDesc*>(block+off);
        m_pGroupDescriptors[i] = new ( &m_alloc ) GroupDesc;
        readb ( 2, 2 * ( gdBlock + idx ), buff );
        Genode::memcpy ( ( void * ) m_pGroupDescriptors[i], ( void * ) ( buff + off ), sizeof ( GroupDesc ) );
        //m_pGroupDescriptors[i]=reinterpret_cast<GroupDesc*>(temp+off);
    }

    // Create our bitmap arrays and tables.
    m_pInodeTables  = new ( &m_alloc ) char * [m_nGroupDescriptors];
    m_pInodeBitmaps = new ( &m_alloc ) char * [m_nGroupDescriptors];
    m_pBlockBitmaps = new ( &m_alloc ) char * [m_nGroupDescriptors];

    for ( size_t i = 0; i < m_nGroupDescriptors; i++ )
    {
        m_pBlockBitmaps[i] = new ( &m_alloc ) char[sizeof ( m_BlockSize )];
        m_pInodeBitmaps[i] = new ( &m_alloc ) char[sizeof ( m_BlockSize )];
    }

    //uint32_t inodesPerGroup = m_pSuperblock->s_inodes_per_group;
    size_t nBlocks = ( inodesPerGroup * m_InodeSize ) / m_BlockSize;

    if ( ( inodesPerGroup * m_InodeSize ) / m_BlockSize )
    {
        nBlocks ++;
    }

    for ( size_t i = 0; i < m_nGroupDescriptors; i++ )
    {
        m_pInodeTables[i] = new ( &m_alloc ) char[nBlocks * m_BlockSize];
    }

    for ( size_t i = 0; i < m_nGroupDescriptors; i++ )
    {
        //'N' symbolizing they the particular group are not loaded
        Ibitmap[i] = 'N';
        Bbitmap[i] = 'N';
        Itable[i] = 'N';
    }

    //m_alloc.free((void*)m_pSuperblock,sizeof(Superblock));
    return true;
}


bool Ext2Filesystem::checkRequiredFeature ( size_t feature )
{
    return true;
}



File * Ext2Filesystem::getRoot()
{
    if ( !m_pRoot )
    {
        Inode * inode = getInode ( EXT2_ROOT_INO );
        char * name = new ( &m_alloc ) char[1];
        name[0] = '\0';
        m_pRoot = new ( &m_alloc ) Ext2Directory ( name, EXT2_ROOT_INO, inode, this, 0 );
    }

    return m_pRoot;
}


bool Ext2Filesystem::createNode ( File * parent, char * filename, uint32_t mask, char * value, size_t type )
{
    // Find a free inode.
    uint32_t inode_num = findFreeInode();
    PINF ( " Creating File or Directory at Inode Number :: %lu \n", inode_num );

    if ( inode_num == 0 )
    {
        PERR ( "NO SPACE LEFT ON DEVICE" );
        return false;
    }

    // Populate the inode.
    Inode * newInode = getInode ( inode_num );
    Genode::memset ( reinterpret_cast<uint8_t *> ( newInode ), 0, m_InodeSize );
    newInode->i_mode = mask | type;
    // If we have a value to store, and it's small enough, use the block indices.
    Ext2Directory * pE2Parent = reinterpret_cast<Ext2Directory *> ( parent );
    // Create the new File object.
    File * pFile = 0;

    switch ( type )
    {
        case EXT2_S_IFREG:
            pFile = new ( &m_alloc ) Ext2File ( filename, inode_num, newInode, this, parent );
            break;

        case EXT2_S_IFDIR:
            {
                Ext2Directory * pE2Dir = new ( &m_alloc ) Ext2Directory ( filename, inode_num, newInode, this, parent );
                pFile = pE2Dir;
                Inode * parentInode = getInode ( pE2Parent->getInodeNumber() );
                // Create dot and dotdot entries.
                char current[2] = {'.', '\0'};
                char previous[3] = {'.', '.', '\0'};
                Ext2Directory * pDot = new ( &m_alloc ) Ext2Directory ( current, inode_num, newInode, this, pE2Dir );
                Ext2Directory * pDotDot = new ( &m_alloc ) Ext2Directory ( previous, pE2Parent->getInodeNumber(), parentInode, this, pE2Dir );
                // Add created dot/dotdot entries to the new directory.
                pE2Dir->addEntry ( current, pDot, EXT2_S_IFDIR );
                pE2Dir->addEntry ( previous, pDotDot, EXT2_S_IFDIR );
                break;
            }

        default:
            PERR ( "EXT2: Unrecognised file type: %x  ", type );
            break;
    }

    // Else case from earlier.
    if ( Genode::strlen ( value ) && Genode::strlen ( value ) >= 4 * 15 )
    {
        const char * pStr = value;
        pFile->write ( 0ULL, Genode::strlen ( value ), reinterpret_cast<uintptr_t> ( pStr ) );
    }

    // Add to the parent directory.
    if ( !pE2Parent->addEntry ( filename, pFile, type ) )
    {
        PERR ( "EXT2: Internal error adding directory entry." );
        return false;
    }

    // Write updated inodes.
    writeInode ( inode_num );
    PINF ( "Writing Inode back :: number %lu :: Filename %s\n", inode_num, filename );
    writeInode ( pE2Parent->getInodeNumber() );
    PINF ( "Parent Inode Number :: %lu \n", pE2Parent->getInodeNumber() );
    pE2Parent->listEntries();
    return true;
}

bool Ext2Filesystem::createFile ( File * parent, char * filename, uint32_t mask )
{
    return true;
    // return createNode(parent, filename, mask, String(""), EXT2_S_IFREG);
}


bool Ext2Filesystem::createDirectory ( File * parent, char * filename )
{
    char * temp = new ( &m_alloc ) char[1];
    temp = '\0';

    if ( !createNode ( parent, filename, 0755, temp , EXT2_S_IFDIR ) )
    {
        return false;
    }

    return true;
}


bool Ext2Filesystem::remove ( File * parent, File * file )
{
    if ( !parent->isDirectory() )
    {
        PERR ( "I0 Error in remove :: Ext2Filesystem.cc" );
        return false;
    }

    Ext2Node * pNode = 0;
    char * filename = new ( &m_alloc ) char[256];

    if ( file->isDirectory() )
    {
        Ext2Directory * pDirectory = static_cast<Ext2Directory *> ( file );
        pNode = pDirectory;
        Genode::memcpy ( filename, pDirectory->getName(), sizeof ( char ) * 256 );
    }

    else
    {
        Ext2File * pFile = static_cast<Ext2File *> ( file );
        pNode = pFile;
        Genode::memcpy ( filename, pFile->getName(), sizeof ( char ) * 256 );
    }

    Ext2Directory * pE2Parent = reinterpret_cast<Ext2Directory *> ( parent );
    return pE2Parent->removeEntry ( filename, pNode );
}



char * Ext2Filesystem::readBlock ( uint32_t block )
{
    readb ( 2, 2 * block, ( char * ) buff );
    return buff;
}


void Ext2Filesystem::writeBlock ( uint32_t block, char * tem )
{
    if ( block != 0 )
    {
        writeb ( 2, 2 * block, ( char * ) tem );
    }
}

uint32_t Ext2Filesystem::findFreeBlock ( uint32_t inode )
{
    inode--; // Inode zero is undefined, so it's not used.
    uint32_t group = inode / m_pSuperblock->s_inodes_per_group;

    for ( ; group < m_nGroupDescriptors; group++ )
    {
        // Any free blocks here?
        GroupDesc * pDesc = m_pGroupDescriptors[group];

        if ( !pDesc->bg_free_blocks_count )
        {
            // No blocks free in this group.
            continue;
        }

        ensureFreeBlockBitmapLoaded ( group );

        // 8 blocks per byte - i == bitmap offset in bytes.

        for ( size_t i = 0;
                i < ( m_pSuperblock->s_blocks_per_group ) / 8;
                i += sizeof ( uint8_t ) )
        {
            // Calculate block index into the bitmap.
            //size_t idx = i / (m_BlockSize * 8);
            size_t off = i % ( m_BlockSize * 8 );
            // Grab the specific block for the bitmap.
            //size_t block = &m_pBlockBitmaps[group][off];
            uint32_t * ptr = reinterpret_cast<uint32_t *> ( &m_pBlockBitmaps[group][off] );
            uint8_t tmp = *ptr;

            // Bitmap full of bits? Skip it.
            if ( tmp == ~0U )
            {
                continue;
            }

            // Check each bit in this field.
            for ( size_t j = 0; j < 8; j++, tmp >>= 1 )
            {
                // Free?
                if ( ( tmp & 1 ) == 0 )
                {
                    // This block is free! Mark used.
                    *ptr |= ( 1 << j );
                    pDesc->bg_free_blocks_count--;
                    //File_system::writeb(2,2,(char*)m_pSuperblock);
                    //since bit maps are cached not writing back these are written back only once while closing filesystem
                    // File_system::writeb(2,2*(m_pGroupDescriptors[group]->bg_block_bitmap),(char*)&m_pBlockBitmaps[group]);
                    // First block of this group...
                    uint32_t block = group * m_pSuperblock->s_blocks_per_group;
                    // Blocks skipped so far (i == offset in bytes)...
                    block += i * 8;
                    // Blocks skipped so far (j == bits ie blocks)...
                    block += j;
                    // Return block.
                    return block;
                }
            }

            // Shouldn't get here - if there were no available blocks here it should
            // have hit the "continue" above!
        }
    }

    return 0;
}

uint32_t Ext2Filesystem::findFreeInode()
{
    for ( uint32_t group = 0; group < m_nGroupDescriptors; group++ )
    {
        // Any free inodes here?
        GroupDesc * pDesc = m_pGroupDescriptors[group];

        if ( !pDesc->bg_free_inodes_count )
        {
            // No inodes free in this group.
            continue;
        }

        // Make sure this block group's inode bitmap has been loaded.
        ensureFreeInodeBitmapLoaded ( group );

        // 8 inodes per byte - i == bitmap offset in bytes.

        for ( size_t i = 0;
                i < m_pSuperblock->s_inodes_per_group / 8;
                i += sizeof ( char ) )
        {
            // Calculate block index into the bitmap.
            // size_t idx = i / (m_BlockSize * 8);
            size_t off = i % ( m_BlockSize * 8 );
            // Grab the specific block for the bitmap.
            //size_t block = &m_pInodeBitmaps[group][off];
            uint32_t * ptr = reinterpret_cast<uint32_t *> ( &m_pInodeBitmaps[group][off] );
            uint8_t tmp = *ptr;

            // If all bits set, avoid searching the bitmap.
            if ( tmp == ~0U )
            {
                continue;
            }

            // Check each bit for free inode.
            for ( size_t j = 0; j < 8; j++, tmp >>= 1 )
            {
                // Free?
                if ( ( tmp & 1 ) == 0 )
                {
                    // This inode is free! Mark used.
                    *ptr |= ( 1 << j );
                    pDesc->bg_free_inodes_count--;
                    // Update superblock.
                    PDBG ( "Free inodes count before ::%lu \n", m_pSuperblock->s_free_inodes_count );
                    m_pSuperblock->s_free_inodes_count--;
                    PDBG ( " Free Inode Count after using Inode :: %lu\n", m_pSuperblock->s_free_inodes_count );
                    writeb ( 2, 2, ( char * ) m_pSuperblock );
                    readb ( 2, 2, buff );
                    PDBG ( "Free inodes count again after writing to block :: %lu GEN CHECKING READ AND WRITE TO BLOCK DEVICES\n", ( ( Superblock * ) ( buff ) )->s_free_inodes_count );
                    //since bit maps are cached not writing back these are written back only once while closing filesystem
                    //File_system::writeb(2,2*(m_pGroupDescriptors[group]->bg_inode_bitmap),(char*)&m_pInodeBitmaps[group]);
                    // First inode of this group...
                    uint32_t inode = group * m_pSuperblock->s_inodes_per_group;
                    // Inodes skipped so far (i == offset in bytes)...
                    inode += i * 8;
                    // Inodes skipped so far (j == bits ie inodes)...
                    // Note: inodes start counting at one, not zero.
                    inode += j + 1;
                    // Return inode.
                    return inode;
                }
            }
        }
    }

    return 0;
}



void Ext2Filesystem::releaseBlock ( uint32_t block )
{
    uint32_t blocksPerGroup = m_pSuperblock->s_blocks_per_group;
    uint32_t group = block / blocksPerGroup;
    uint32_t index = block % blocksPerGroup;
    ensureFreeBlockBitmapLoaded ( group );
    // Free block.
    GroupDesc * pDesc = m_pGroupDescriptors[group];
    pDesc->bg_free_blocks_count++;
    m_pSuperblock->s_free_blocks_count++;
    // Index = block offset from the start of this block.
    //  size_t bitmapField = (index / 8) / m_BlockSize;
    size_t bitmapOffset = ( index / 8 ) % m_BlockSize;
    size_t diskBlock = m_pBlockBitmaps[group][bitmapOffset];
    uint8_t * ptr = reinterpret_cast<uint8_t *> ( diskBlock );
    *ptr &= ~ ( 1 << ( index % 8 ) );
    //since bit maps are cached not writing back these are written back only once while closing filesystem
    //File_system::writeb(2,2,(char*)m_pSuperblock);
    //File_system::writeb(2,2*(m_pGroupDescriptors[group]->bg_block_bitmap),(char*)&m_pBlockBitmaps[group]);
}



bool Ext2Filesystem::releaseInode ( uint32_t inode )
{
    //Inode *pInode = getInode(inode);
    --inode; // Inode zero is undefined, so it's not used.
    uint32_t inodesPerGroup = m_pSuperblock->s_inodes_per_group;
    uint32_t group = inode / inodesPerGroup;
    uint32_t index = inode % inodesPerGroup;
    bool bRemove = decreaseInodeRefcount ( inode + 1 );

    // Do we need to free this inode?
    if ( bRemove )
    {
        ensureFreeInodeBitmapLoaded ( group );
        // Free inode.
        GroupDesc * pDesc = m_pGroupDescriptors[group];
        pDesc->bg_free_inodes_count++;
        m_pSuperblock->s_free_inodes_count++;
        // Index = inode offset from the start of this block.
        //  size_t bitmapField = (index / 8) / m_BlockSize;
        size_t bitmapOffset = ( index / 8 ) % m_BlockSize;
        // size_t block = &m_pInodeBitmaps[group][bitmapOffset];
        uint8_t * ptr = reinterpret_cast<uint8_t *> ( &m_pInodeBitmaps[group][bitmapOffset] );
        *ptr &= ~ ( 1 << ( index % 8 ) );
        //since bit maps are cached not writing back these are written back only once while closing filesystem
        //	File_system::writeb(2,2,(char*)m_pSuperblock);
        //File_system::writeb(2,2*(m_pGroupDescriptors[group]->bg_inode_bitmap),(char*)&m_pInodeBitmaps[group]);
    }

    writeInode ( inode );
    return bRemove;
}

Inode * Ext2Filesystem::getInode ( uint32_t inode )
{
    inode--; // Inode zero is undefined, so it's not used.
    uint32_t inodesPerGroup = m_pSuperblock->s_inodes_per_group;
    uint32_t group = inode / inodesPerGroup;
    uint32_t index = inode % inodesPerGroup;
    PINF ( "loading Inode \n" );
    ensureInodeTableLoaded ( group );
    size_t blockNum = ( index * m_InodeSize ) / m_BlockSize;
    size_t blockOff = ( index * m_InodeSize ) % m_BlockSize;
    //size_t block = &m_pInodeTables[group][blockNum*m_BlockSize];
    Inode * pInode = reinterpret_cast<Inode *> ( &m_pInodeTables[group][blockNum * m_BlockSize + blockOff] );
    return pInode;
}

void Ext2Filesystem::writeInode ( uint32_t inode )
{
    inode--; // Inode zero is undefined, so it's not used.
    uint32_t inodesPerGroup = ( m_pSuperblock->s_inodes_per_group );
    uint32_t group = inode / inodesPerGroup;
    uint32_t index = inode % inodesPerGroup;
    //ensureInodeTableLoaded(group);
    size_t blockNum = ( index * m_InodeSize ) / m_BlockSize;
    //since bit maps are cached not writing back these are written back only once while closing filesystem
    writeb ( 2, 2 * ( ( m_pGroupDescriptors[group]->bg_inode_table ) + blockNum ), ( char * ) &m_pInodeTables[group][blockNum * m_BlockSize] );
}


void Ext2Filesystem::ensureFreeBlockBitmapLoaded ( size_t group )
{
    if ( Bbitmap[group] == 'Y' )
    {
        return;
    }

    Bbitmap[group] = 'Y';
    readb ( 2, 2 * ( m_pGroupDescriptors[group]->bg_block_bitmap ), m_pBlockBitmaps[group] );
}

void Ext2Filesystem::ensureFreeInodeBitmapLoaded ( size_t group )
{
    if ( Ibitmap[group] == 'Y' )
    {
        return;
    }

    Ibitmap[group] = 'Y';
    //    m_pInodeBitmaps[group]=new (&m_alloc)char[sizeof(m_BlockSize)];
    readb ( 2, 2 * ( m_pGroupDescriptors[group]->bg_inode_bitmap ) , m_pInodeBitmaps[group] );
}

void Ext2Filesystem::ensureInodeTableLoaded ( size_t group )
{
    if ( Itable[group] == 'Y' )
    {
        return;
    }

    Itable[group] = 'Y';
    // Determine how many blocks to load to bring in the full inode table.
    uint32_t inodesPerGroup = m_pSuperblock->s_inodes_per_group;
    size_t nBlocks = ( inodesPerGroup * m_InodeSize ) / m_BlockSize;

    if ( ( inodesPerGroup * m_InodeSize ) / m_BlockSize )
    {
        nBlocks ++;
    }

    //m_pInodeTables[group]=new (&m_alloc)char[nBlocks*m_BlockSize];
    // Load each block in the inode table.
    PINF ( "loading Inode tables\n" );
    readb ( nBlocks * 2, 2 * ( m_pGroupDescriptors[group]->bg_inode_table ), m_pInodeTables[group] );
}

void Ext2Filesystem::increaseInodeRefcount ( uint32_t inode )
{
    Inode * pInode = getInode ( inode );

    if ( !pInode )
    {
        return;
    }

    uint32_t current_count = pInode->i_links_count;
    pInode->i_links_count = current_count + 1;
    writeInode ( inode );
}

bool Ext2Filesystem::decreaseInodeRefcount ( uint32_t inode )
{
    Inode * pInode = getInode ( inode );

    if ( !pInode )
    {
        return true;    // No inode found - but didn't decrement to zero.
    }

    uint32_t current_count = pInode->i_links_count;
    bool bRemove = current_count <= 1;

    if ( current_count )
    {
        pInode->i_links_count = current_count - 1;
    }

    writeInode ( inode );
    return bRemove;
}



