#include "Ext2Filesystem.h"
#include "Ext2Directory.h"
#include "Ext2File.h"


Ext2Directory::Ext2Directory ( char * name, size_t inode_num, Inode * inode,
                               Ext2Filesystem * pFs, File * pParent ) :
    Directory ( name, inode_num,
                static_cast<Filesystem *> ( pFs ),
                inode->i_size, /// \todo Deal with >4GB files here.
                pParent ),
    Ext2Node ( inode_num, inode, pFs )
{
    //uint32_t mode = inode->i_mode;
}

Ext2Directory::~Ext2Directory()
{
}

bool Ext2Directory::addEntry ( char * filename, File * pFile, size_t type )
{
    char * buffer;
    // Calculate the size of our Dir* entry.
    size_t length = 4 + /* 32-bit inode number */
                    2 + /* 16-bit record length */
                    1 + /* 8-bit name length */
                    1 + /* 8-bit file type */
                    Genode::strlen ( filename ); /* Don't leave space for NULL-terminator, not needed. */
    bool bFound = false;
    uint32_t i;
    Dir * pDir;

    for ( i = 0; i < m_nBlocks; i++ )
    {
        ensureBlockLoaded ( i );
        buffer = ( m_pExt2Fs->readBlock ( m_pBlocks[i] ) );
        pDir = reinterpret_cast<Dir *> ( buffer );

        while ( reinterpret_cast<size_t> ( pDir ) < reinterpret_cast<size_t> ( buffer ) + m_pExt2Fs->m_BlockSize )
        {
            // What's the minimum length of this directory entry?
            size_t thisReclen = 4 + 2 + 1 + 1 + pDir->d_namelen;

            // Align to 4-byte boundary.
            if ( thisReclen % 4 )
            {
                thisReclen += 4 - ( thisReclen % 4 );
            }

            // Valid directory entry?
            if ( pDir->d_inode > 0 )
            {
                // Is there enough space to add this dirent?
                /// \todo Ensure 4-byte alignment.
                if ( pDir->d_reclen - thisReclen >= length )
                {
                    bFound = true;
                    // Save the current reclen.
                    uint16_t oldReclen = pDir->d_reclen;
                    // Adjust the current record's reclen field to the minimum.
                    pDir->d_reclen = thisReclen;
                    // Move to the new directory entry location.
                    pDir = reinterpret_cast<Dir *> ( reinterpret_cast<size_t> ( pDir ) + thisReclen );
                    // New record length.
                    uint16_t newReclen = oldReclen - thisReclen;
                    // Set the new record length.
                    pDir->d_reclen = newReclen;
                    break;
                }
            }

            else if ( pDir->d_reclen == 0 )
            {
                // No more entries to follow.
                break;
            }

            else if ( pDir->d_reclen - thisReclen >= length )
            {
                // We can use this unused entry - we fit into it.
                // The record length does not need to be adjusted.
                bFound = true;
                break;
            }

            // Next.
            pDir = reinterpret_cast<Dir *> ( reinterpret_cast<size_t> ( pDir ) + pDir->d_reclen );
        }

        if ( bFound )
        {
            break;
        }
    }

    if ( !bFound )
    {
        // Need to make a new block.
        uint32_t block = m_pExt2Fs->findFreeBlock ( getInodeNumber() );

        if ( block == 0 )
        {
            // We had a problem.
            return false;
        }

        if ( !addBlock ( block ) )
        {
            return false;
        }

        i = m_nBlocks - 1;
        m_Size = m_nBlocks * m_pExt2Fs->m_BlockSize;
        fileAttributeChanged();
        /// \todo Previous directory entry might need its reclen updated to
        ///       point to this new entry (as directory entries cannot cross
        ///       block boundaries).
        ensureBlockLoaded ( i );
        buffer = m_pExt2Fs->readBlock ( m_pBlocks[i] );
        Genode::memset ( reinterpret_cast<void *> ( buffer ), 0, m_pExt2Fs->m_BlockSize );
        pDir = reinterpret_cast<Dir *> ( buffer );
        pDir->d_reclen = m_pExt2Fs->m_BlockSize;
    }

    // Set the directory contents.
    uint32_t entryInode = pFile->getInode();
    pDir->d_inode = entryInode;
    m_pExt2Fs->increaseInodeRefcount ( entryInode );

    if ( 1 )
    {
        // File type in directory entry.
        switch ( type )
        {
            case EXT2_S_IFREG:
                pDir->d_file_type = EXT2_FILE;
                break;

            case EXT2_S_IFDIR:
                pDir->d_file_type = EXT2_DIRECTORY;
                break;

            default:
                PERR ( "Unrecognised filetype." );
                pDir->d_file_type = EXT2_UNKNOWN;
        }
    }

    else
    {
        // No file type in directory entries.
        pDir->d_file_type = 0;
    }

    pDir->d_namelen = Genode::strlen ( filename );
    Genode::memcpy ( pDir->d_name, static_cast<const char *> ( filename ), Genode::strlen ( filename ) );
    // We're all good - add the directory to our cache.
    m_Cache.insert ( filename, pFile );
    // Trigger write back to disk.
    m_pExt2Fs->writeBlock ( m_pBlocks[i], ( char * ) buffer );
    m_Size = m_nSize;
    return true;
}

bool Ext2Directory::removeEntry ( char * filename, Ext2Node * pFile )
{
    size_t fileInode = pFile->getInodeNumber();
    bool bFound = false;
    uint32_t i;
    Dir * pDir, *pLastDir = 0;

    for ( i = 0; i < m_nBlocks; i++ )
    {
        ensureBlockLoaded ( i );
        char * buffer = m_pExt2Fs->readBlock ( m_pBlocks[i] );
        pDir = reinterpret_cast<Dir *> ( buffer );
        pLastDir = 0;

        while ( reinterpret_cast<size_t> ( pDir ) < reinterpret_cast<size_t> ( buffer ) + m_pExt2Fs->m_BlockSize )
        {
            if ( pDir->d_inode == fileInode )
            {
                if ( pDir->d_namelen == Genode::strlen ( filename ) )
                {
                    if ( !Genode::strcmp ( pDir->d_name, static_cast<const char *> ( filename ) ) )
                    {
                        // Wipe out the directory entry.
                        uint16_t old_reclen = pDir->d_reclen;
                        Genode::memset ( pDir, 0, old_reclen );

                        /// \todo  this is not quite enough. The previous
                        ///       entry needs to be updated to skip past this
                        ///       now-empty entry. If this was the first entry,
                        ///       a blank record must be created to point to
                        ///       either the next entry or the end of the block.

                        if ( pLastDir )
                        {
                            uint16_t new_reclen = old_reclen + pLastDir->d_reclen;
                            pLastDir->d_reclen = new_reclen;
                        }

                        pDir->d_reclen = old_reclen;
                        m_pExt2Fs->writeBlock ( m_pBlocks[i], buffer );
                        bFound = true;
                        break;
                    }
                }
            }

            else if ( !pDir->d_reclen )
            {
                // No more entries.
                break;
            }

            pLastDir = pDir;
            pDir = reinterpret_cast<Dir *> ( reinterpret_cast<uintptr_t> ( pDir ) + ( pDir->d_reclen ) );
        }

        if ( bFound )
        {
            break;
        }
    }

    m_Size = m_nSize;

    if ( bFound )
    {
        if ( m_pExt2Fs->releaseInode ( fileInode ) )
        {
            // Remove all blocks for the file, inode has hit zero refcount.
            pFile->wipe();
        }

        this->listEntries();
        return true;
    }

    else
    {
        return false;
    }

    this->listEntries();
    return true;
}

void Ext2Directory::cacheDirectoryContents()
{
    uint32_t i;
    Dir * pDir;

    for ( i = 0; i < m_nBlocks; i++ )
    {
        ensureBlockLoaded ( i );
        char * buffer = m_pExt2Fs->readBlock ( m_pBlocks[i] );
        pDir = reinterpret_cast<Dir *> ( buffer );

        while ( reinterpret_cast<size_t> ( pDir ) < reinterpret_cast<size_t> ( buffer + m_pExt2Fs->m_BlockSize ) )
        {
            Dir * pNextDir = reinterpret_cast<Dir *> ( reinterpret_cast<size_t> ( pDir ) + pDir->d_reclen );

            if ( pDir->d_inode == 0 )
            {
                if ( pDir == pNextDir )
                {
                    // No further iteration possible (null entry).
                    break;
                }

                // Oops, not a valid entry (possibly deleted file). Skip.
                pDir = pNextDir;
                continue;
            }

            size_t namelen = pDir->d_namelen + 1;
            // Can we get the file type from the directory entry?
            size_t fileType = EXT2_UNKNOWN;

            if ( 1 )
            {
                // Directory entry holds file type.
                fileType = pDir->d_file_type;
            }

            else
            {
                // Inode holds file type.
                Inode * inode = m_pExt2Fs->getInode ( pDir->d_inode );
                size_t inode_ftype = inode->i_mode & 0xF000;

                switch ( inode_ftype )
                {
                    case EXT2_S_IFREG:
                        fileType = EXT2_FILE;
                        break;

                    case EXT2_S_IFDIR:
                        fileType = EXT2_DIRECTORY;
                        break;

                    default:
                        break;
                }

                namelen |= pDir->d_file_type << 8;
            }

            // Grab filename from the entry.
            char * filename = new ( m_alloc ) char[namelen];
            Genode::memcpy ( filename, pDir->d_name, namelen - 1 );
            filename[namelen - 1] = '\0';
            uint32_t inode = pDir->d_inode;
            File * pFile = 0;

            switch ( fileType )
            {
                case EXT2_FILE:
                    pFile = new ( m_alloc ) Ext2File ( filename, inode, m_pExt2Fs->getInode ( inode ), m_pExt2Fs, this );
                    break;

                case EXT2_DIRECTORY:
                    pFile = new ( m_alloc ) Ext2Directory ( filename, inode, m_pExt2Fs->getInode ( inode ), m_pExt2Fs, this );
                    break;
            }

            // Add to cache.
            m_Cache.insert ( filename, pFile );
            // Next.
            pDir = pNextDir;
        }
    }

    m_bCachePopulated = true;
}


bool Ext2Directory::listEntries()
{
    // Find this file in the directory.
    // size_t fileInode = pFile->getInodeNumber();
    PDBG ( " Directories in current directory \n" );
    bool bFound = false;
    uint32_t i;
    Dir * pDir; // *pLastDir = 0;

    for ( i = 0; i < m_nBlocks; i++ )
    {
        ensureBlockLoaded ( i );
        char * buffer = m_pExt2Fs->readBlock ( m_pBlocks[i] );
        pDir = reinterpret_cast<Dir *> ( buffer );

        // pLastDir = 0;
        while ( reinterpret_cast<size_t> ( pDir ) < reinterpret_cast<size_t> ( buffer ) + m_pExt2Fs->m_BlockSize )
        {
            PINF ( "%s\n", pDir->d_name );

            if ( pDir->d_reclen == 0 )
            {
                // No more entries to follow.
                break;
            }

            // Next.
            pDir = reinterpret_cast<Dir *> ( reinterpret_cast<size_t> ( pDir ) + pDir->d_reclen );
        }

        if ( bFound )
        {
            break;
        }
    }

    return true;
}
void Ext2Directory::fileAttributeChanged()
{
    static_cast<Ext2Node *> ( this )->fileAttributeChanged ( m_Size );
}

