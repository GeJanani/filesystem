#include "Filesystem.h"
#include "File.h"
#include "Directory.h"
#include <os/config.h>
#include <os/path.h>
#include <util/string.h>

Filesystem::Filesystem()
{
}

File * Filesystem::find ( char * path, File * pStartNode )
{
    if ( !pStartNode )
    {
        pStartNode = getRoot();
    }

    File * a = findNode ( pStartNode, path );
    return a;
}
/*
bool Filesystem::createFile(String path, uint32_t mask, File *pStartNode)
{
   return true;
}
*/
bool Filesystem::createDirectory ( char * path, File * pStartNode )
{
    PINF ( "create file or directory\n" );
    char * temp = new ( m_alloc ) char[Genode::strlen ( path ) + 1];
    Genode::memcpy ( temp, path, Genode::strlen ( path ) + 1 );

    if ( !pStartNode )
    {
        pStartNode = getRoot();
    }

    File * pFile = findNode ( pStartNode, temp );
    Genode::memcpy ( temp, path, Genode::strlen ( path ) + 1 );

    if ( pFile )
    {
        return false;
    }

    char * filename = new ( m_alloc ) char[ ( Genode::strlen ( path ) ) + 1];
    File * pParent = findParent ( temp, pStartNode, filename );

    if ( !pParent )
    {
        PERR ( "Parent DoesNotExist" );
        return false;
    }

    // Now make the directory.
    createDirectory ( pParent, filename );
    return true;
}

bool Filesystem::remove ( char * path, File * pStartNode )
{
    if ( !pStartNode )
    {
        pStartNode = getRoot();
    }

    char * temp = new ( m_alloc ) char[Genode::strlen ( path ) + 1];
    Genode::memcpy ( temp, path, Genode::strlen ( path ) + 1 );
    File * pFile = findNode ( pStartNode, temp );

    if ( !pFile )
    {
        PERR ( "file doesnt exist" );
        return false;
    }

    char * filename = new ( m_alloc ) char[Genode::strlen ( path ) + 1];
    File * pParent = findParent ( path, pStartNode, filename );

    // Check the parent existed.
    if ( !pParent )
    {
        return false;
    }

    Directory * pDParent = Directory::fromFile ( pParent );

    if ( !pDParent )
    {
        return false;
    }

    bool bRemoved = remove ( pParent, pFile );

    if ( bRemoved )
    {
        pDParent->m_Cache.remove ( filename );
    }

    return bRemoved;
}

File * Filesystem::findNode ( File * pNode, char * path )
{
    if ( Genode::strlen ( path ) == 0 )
    {
        return pNode;
    }

    if ( path[0] == '/' )
    {
        pNode = getRoot();
        path++;
    }

    // Grab the next filename component.
    size_t i = 0;

    //size_t nExtra = 0;
    while ( path[i] != '/' && path[i] != '\0' )
    {
        i++;
    }

    char * restOfPath = new ( m_alloc ) char[ ( Genode::strlen ( path ) ) + 1];
    Genode::memset ( restOfPath, 0, ( Genode::strlen ( path ) ) + 1 );

    if ( path[i] != '\0'	)
    {
        char * temp = &path[i + 1];
        Genode::strncpy ( restOfPath, temp, Genode::strlen ( temp ) );
        restOfPath[Genode::strlen ( temp )] = '\0';

        if ( path[i] == '/' )
        {
            path[i] = '\0';
        }
    }

    // At this point 'path' contains the token to search for. 'restOfPath' contains the path for the next recursion (or nil).

    // If 'path' is zero-lengthed, ignore and recurse.
    if ( Genode::strlen ( path ) == 0 )
    {
        return findNode ( pNode, restOfPath );
    }

    // Next, if the current node isn't a directory, die.
    if ( !pNode->isDirectory() )
    {
        PERR ( "Not a DIRECTORY " );
        return 0;
    }

    if ( !Genode::strcmp ( path, "." ) || ( !Genode::strcmp ( path, ".." ) && pNode->m_pParent == 0 ) )
    {
        return findNode ( pNode, restOfPath );
    }

    else if ( !Genode::strcmp ( path, ".." ) )
    {
        return findNode ( pNode->m_pParent, restOfPath );
    }

    Directory * pDir = Directory::fromFile ( pNode );

    if ( !pDir )
    {
        // Throw some error...
        return 0;
    }

    // Cache lookup.
    File * pFile;
    PDBG ( "Are directory contents cached ::  %d \n", pDir->m_bCachePopulated );

    if ( !pDir->m_bCachePopulated )
    {
        PDBG ( "Caching the directory contents :: \n" );
        // Directory contents not cached - cache them now.
        pDir->cacheDirectoryContents();
    }

    PDBG ( "Looking in cache\n" );
    pFile = pDir->m_Cache.lookup ( path );

    if ( pFile )
    {
        return findNode ( pFile, restOfPath );
    }

    else
    {
        PERR ( "\nCache lookup failed, does not exist.\n" );
        return 0;
    }
}

File * Filesystem::findParent ( char * path, File * pStartNode, char * filename )
{
    // Work forwards to the end of the path string, attempting to find the last '/'.
    ssize_t lastSlash = -1;

    for ( size_t i = 0; i < Genode::strlen ( path ); i++ )
        if ( path[i] == '/' )
        {
            lastSlash = i;
        }

    // Now, if there were no slashes, the parent node is pStartNode.
    if ( lastSlash == -1 )
    {
        Genode::memcpy ( filename, path, Genode::strlen ( path ) + 1 );
        return pStartNode;
    }

    else
    {
        // Else split the filename off from the rest of the path and follow it.
        Genode::memcpy ( filename, &path[lastSlash + 1], Genode::strlen ( path ) );
        // Remove the trailing '/' from path;
        path[lastSlash] = '\0';
        return findNode ( pStartNode, path );
    }
}
