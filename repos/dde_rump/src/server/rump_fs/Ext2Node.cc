#include "Ext2Filesystem.h"
#include "Ext2Node.h"

Ext2Node::Ext2Node ( size_t inode_num, Inode * pInode, Ext2Filesystem * pFs ) :
    m_pInode ( pInode ), m_InodeNumber ( inode_num ), m_pExt2Fs ( pFs ), m_pBlocks ( 0 ),
    m_nBlocks ( 0 ), m_nSize ( pInode->i_size )
{
    // i_blocks == # of 512-byte blocks. Convert to FS block count.
    uint32_t blockCount = pInode->i_blocks;
    m_nBlocks = ( blockCount * 512 ) / m_pExt2Fs->m_BlockSize;
    m_pBlocks = new ( m_alloc ) uint32_t[m_nBlocks];
    Genode::memset ( m_pBlocks, ~0, sizeof ( uint32_t ) * m_nBlocks );

    for ( size_t i = 0; i < 12 && i < m_nBlocks; i++ )
    {
        m_pBlocks[i] = m_pInode->i_block[i];
    }
}

Ext2Node::~Ext2Node()
{
}

uint64_t Ext2Node::doRead ( uint64_t location, uint64_t size, size_t buffer )
{
    return size;
}

uint64_t Ext2Node::doWrite ( uint64_t location, uint64_t size, uintptr_t buffer )
{
    return size;
}


size_t Ext2Node::readBlock ( uint64_t location )
{
    // Sanity check.
    uint32_t nBlock = location / m_pExt2Fs->m_BlockSize;

    if ( nBlock > m_nBlocks )
    {
        return 0;
    }

    if ( location > m_nSize )
    {
        return 0;
    }

    ensureBlockLoaded ( nBlock );
    return reinterpret_cast<size_t> ( m_pExt2Fs->readBlock ( m_pBlocks[nBlock] ) );
}

void Ext2Node::writeBlock ( uint64_t location )
{
    // Sanity check.
    uint32_t nBlock = location / m_pExt2Fs->m_BlockSize;

    if ( nBlock > m_nBlocks )
    {
        return;
    }

    if ( location > m_nSize )
    {
        return;
    }

    // Update on disk.
    ensureBlockLoaded ( nBlock );
    return m_pExt2Fs->writeBlock ( m_pBlocks[nBlock], buff );
}

void Ext2Node::trackBlock ( uint32_t block )
{
    uint32_t * pTmp = new ( m_alloc ) uint32_t[m_nBlocks + 1];
    Genode::memcpy ( pTmp, m_pBlocks, m_nBlocks * sizeof ( uint32_t ) );
    m_alloc->free ( ( void * ) m_pBlocks, sizeof ( uint32_t ) * m_nBlocks );
    m_pBlocks = pTmp;
    m_pBlocks[m_nBlocks++] = block;
    // Inode i_blocks field is actually the count of 512-byte blocks.
    uint32_t i_blocks = ( m_nBlocks * m_pExt2Fs->m_BlockSize ) / 512;
    m_pInode->i_blocks = i_blocks;
    // Write updated inode.
    m_pExt2Fs->writeInode ( getInodeNumber() );
}

void Ext2Node::wipe()
{
    for ( size_t i = 0; i < m_nBlocks; i++ )
    {
        ensureBlockLoaded ( i );
        m_pExt2Fs->releaseBlock ( m_pBlocks[i] );
    }

    m_nSize = 0;
    m_nBlocks = 0;
    m_pInode->i_size = 0;
    m_pInode->i_blocks = 0;
    Genode::memset ( m_pInode->i_block, 0, sizeof ( uint32_t ) * 15 );
    // Write updated inode.
    m_pExt2Fs->writeInode ( getInodeNumber() );
}

bool Ext2Node::ensureLargeEnough ( size_t size )
{
    return true;
}

bool Ext2Node::ensureBlockLoaded ( size_t nBlock )
{
    if ( nBlock >= m_nBlocks )
    {
        PERR ( "EXT2: ensureBlockLoaded: error" );
    }

    if ( m_pBlocks[nBlock] == ~0U )
    {
        getBlockNumber ( nBlock );
    }

    return true;
}

bool Ext2Node::getBlockNumber ( size_t nBlock )
{
    // size_t nPerBlock = m_pExt2Fs->m_BlockSize/4;
    return true;
}

bool Ext2Node::addBlock ( uint32_t blockValue )
{
    //    size_t nEntriesPerBlock = m_pExt2Fs->m_BlockSize/4;

    // Calculate whether direct, indirect or tri-indirect addressing is needed.
    if ( m_nBlocks < 12 )
    {
        // Direct addressing is possible.
        m_pInode->i_block[m_nBlocks] = blockValue;
    }

    trackBlock ( blockValue );
    return true;
}

void Ext2Node::fileAttributeChanged ( size_t size )
{
    // Reconstruct the inode from the cached fields.
    uint32_t i_blocks = ( m_nBlocks * m_pExt2Fs->m_BlockSize ) / 512;
    m_pInode->i_blocks = i_blocks;
    m_pInode->i_size = size; /// \todo 4GB files.
    // Update our internal record of the file size accordingly.
    m_nSize = size;
    // Write updated inode.
    m_pExt2Fs->writeInode ( getInodeNumber() );
}

