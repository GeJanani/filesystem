#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include "File.h"
class Filesystem {
    public:
        Filesystem();
        /** Destructor */
        virtual ~Filesystem() {}

        /** Populates this filesystem with data from the given Disk device.
         * \return true on success, false on failure. */
        virtual bool initialise() = 0;
        static Genode::Allocator * m_alloc;

        /** Attempt to find a file or directory in this filesystem.
            \param path The path to the file, in UTF-8 format, without filesystem identifier
            (e.g. not "root:/file", but "/file").
            \param pStartNode The node to start parsing 'path' from - defaults to / but
            is expected to contain the current working directory.
            \return The file if one was found, or 0 otherwise or if there was an error.
        */
        virtual File * find ( char * path, File * pStartNode = 0 );

        /** Returns the root filesystem node. */
        virtual File * getRoot() = 0;

        /** Creates a file on the filesystem - fails if the file's parent directory does not exist. */
        bool createFile ( char * path, uint32_t mask, File * pStartNode = 0 );

        /** Creates a directory on the filesystem. Fails if the dir's parent directory does not exist. */
        bool createDirectory ( char * path, File * pStartNode = 0 );


        /** Removes a file, directory or symlink.
            \note Will fail if it is a directory and is not empty. The failure mode
            is unspecified. */
        bool remove ( char * path, File * pStartNode = 0 );

    protected:
        /** createFile calls this after it has parsed the string path. */
        virtual bool createFile ( File * parent, char * filename, uint32_t mask ) = 0;
        /** createDirectory calls this after it has parsed the string path. */
        virtual bool createDirectory ( File * parent, char * filename ) = 0;
        /** remove() calls this after it has parsed the string path. */
        virtual bool remove ( File * parent, File * file ) = 0;
        /** is this entire filesystem read-only?  */
    private:

        /** Internal function to find a node - Returns 0 on failure or the node.
            \param pNode The node to start parsing 'path' from.
            \param path  The path from pNode to the destination node. */
        File * findNode ( File * pNode, char * path );

        /** Internal function to find a node's parent directory.
            \param path The path from pStartNode to the original file.
            \param pStartNode The node to start parsing 'path' from.
            \param[out] filename The child file's name. */
        File * findParent ( char * path, File * pStartNode, char * filename );

};


#endif
