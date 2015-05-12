#ifndef FILE_H
#define FILE_H
#define FILE_UR 0001
#define FILE_UW 0002
#define FILE_UX 0004
#define FILE_GR 0010
#define FILE_GW 0020
#define FILE_GX 0040
#define FILE_OR 0100
#define FILE_OW 0200
#define FILE_OX 0400
extern "C" {
#include <sys/cdefs.h>
#include <sys/types.h>
}

#include <os/config.h>
#include <os/path.h>
#include <util/string.h>

class Filesystem;
/** A File is a regular file - it is also the superclass of Directory, Symlink
    and Pipe. */
class File {
        friend class Filesystem;

    public:
        /** Constructor, creates an invalid file. */
        File();

    public:
        /** Constructor, should be called only by a Filesystem. */
        File ( char * name, size_t inode, class Filesystem * pFs, size_t size, File * pParent );
        /** Destructor - doesn't do anything. */
        virtual ~File();

        /** Reads from the file.
         *  \param[in] buffer Buffer to write the read data into. Can be null, in
         *      which case the data can be found by calling getPhysicalPage.
         *  \param[in] bCanBlock Whether or not the File can block when reading
         */
        virtual uint64_t read ( uint64_t location, uint64_t size, size_t buffer, bool bCanBlock = true );
        /** Writes to the file.
         *  \param[in] bCanBlock Whether or not the File can block when reading
         */
        virtual uint64_t write ( uint64_t location, uint64_t size, size_t buffer, bool bCanBlock = true );



        /** Returns the name of the file. */
        char * getName();
        void getName ( char * s );
        // File names cannot be changed.

        /** Obtains the full path of the File. */
        virtual char * getFullPath ( bool bWithLabel = true );

        /** Delete all data from the file. */
        virtual void truncate();

        size_t getSize();
        void setSize ( size_t sz );


        /** Returns true if the File is actually a directory. */
        virtual bool isDirectory() {
            return false;
        }

        size_t getInode() {
            return m_Inode;
        }
        virtual void setInode ( size_t inode ) {
            m_Inode = inode;
        }

        Filesystem * getFilesystem() {
            return m_pFilesystem;
        }
        void setFilesystem ( Filesystem * pFs ) {
            m_pFilesystem = pFs;
        }

        virtual void fileAttributeChanged() {}






        File * getParent() {
            return m_pParent;
        }

        /** Does this File object support the given integer-based command? */
        virtual bool supports ( const int command ) {
            return false;
        }

        /** Handle a command. */
        virtual int command ( const int command, void * buffer ) {
            return 0;
        }

        /** Function to retrieve the block size returned by readBlock.
            \note This must be constant throughout the life of the file. */
        virtual size_t getBlockSize() const {
            return 1024;
        }

    protected:

        /** Internal function to retrieve an aligned 512byte section of the file. */
        //size was ptr
        virtual size_t readBlock ( uint64_t location ) {
            return 0;
        }
        /**
         * Internal function to write a block retrieved with readBlock back to
         * the file. The address of the block is provided for convenience.
         */
        virtual void writeBlock ( uint64_t location, size_t addr ) {
        }

        /** Internal function to extend a file to be at least the given size. */
        virtual void extend ( size_t newSize ) {
            if ( m_Size < newSize ) {
                m_Size = newSize;
            }
        }




        char * m_Name;

        size_t m_Inode;

        class Filesystem * m_pFilesystem;
        size_t m_Size;

        File * m_pParent;


};

#endif
