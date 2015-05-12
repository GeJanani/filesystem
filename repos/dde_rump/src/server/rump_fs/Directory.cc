#include "Directory.h"
#include "Filesystem.h"

Directory::Directory() :
    File(), m_Cache(), m_bCachePopulated ( false )
{
}

Directory::Directory ( char * name, size_t inode, Filesystem * pFs, size_t size, File * pParent ) :
    File ( name, inode, pFs, size, pParent ),
    m_Cache ( 0 ), m_bCachePopulated ( false )
{
}

Directory::~Directory()
{
}

void Directory::cacheDirectoryContents()
{
}
