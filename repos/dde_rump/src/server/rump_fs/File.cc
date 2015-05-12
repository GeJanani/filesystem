#include "Filesystem.h"
#include "File.h"
File::File() :
    m_Name ( "" ), m_Inode ( 0 ), m_pFilesystem ( 0 ), m_Size ( 0 ),
    m_pParent ( 0 )
{
}

File::File ( char * name,
             size_t inode, Filesystem * pFs, size_t size, File * pParent ) :
    m_Name ( name ), m_Inode ( inode ), m_pFilesystem ( pFs ),
    m_Size ( size ), m_pParent ( pParent )
{
}

File::~File()
{
}

uint64_t File::read ( uint64_t location, uint64_t size, size_t buffer, bool bCanBlock )
{
    return 0;
}

uint64_t File::write ( uint64_t location, uint64_t size, size_t buffer, bool bCanBlock )
{
    return 0;
}



char * File::getName()
{
    return m_Name;
}

void File::getName ( char * s )
{
    s = m_Name;
}

size_t File::getSize()
{
    return m_Size;
}

void File::setSize ( size_t sz )
{
    m_Size = sz;
}

void File::truncate()
{
}

char * File::getFullPath ( bool bWithLabel )
{
    char * temp;
    return temp;
}


