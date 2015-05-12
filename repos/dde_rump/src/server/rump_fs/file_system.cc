/*
 * \brief  Rump initialization
 * \author Sebastian Sumpf
 * \date   2014-01-17
 */

/*
 * Copyright (C) 2014 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include "file_system.h"

#include <base/thread.h>
#include <os/config.h>

#include <timer_session/connection.h>
#include <util/string.h>
#include <util/hard_context.h>
#include <base/allocator_avl.h>
#include <base/printf.h>
#include <block_session/connection.h>

//===========================/


char buff[1025];


static bool _supports_symlinks;
using namespace Genode;
//=================================



static bool const verbose = false;

static Genode::Allocator_avl _block_alloc ( Genode::env()->heap() );
static size_t _blk_size = 0;
static Block::sector_t _blk_cnt  = 0;
static Block::Session::Tx::Source * _source;
static Block::Connection * _block_connection;





static bool check_symlinks()
{
    return false;
}


/*static bool check_read_only()
{
	//if (!Genode::strcmp(_fs_type, RUMP_MOUNT_CD9660))
		//return true;

	return false;
}
*/




void File_system::init ( Server::Entrypoint & ep )
{
    PDBG ( "disk_initialize(drv=%u) called.", 0 );
    _block_connection = new ( Genode::env()->heap() ) Block::Connection ( &_block_alloc );
    //  Block::Connection _block_connection(&_block_alloc);
    _source = _block_connection->tx();
    Block::Session::Operations  ops;
    _block_connection->info ( &_blk_cnt, &_blk_size, &ops );

    /* check for read- and write-capability */
    if ( !ops.supported ( Block::Packet_descriptor::READ ) )
    {
        PERR ( "Block device not readable!" );
        //destroy(env()->heap(), _block_connection);
    }

    if ( !ops.supported ( Block::Packet_descriptor::WRITE ) )
    {
        PINF ( "Block device not writeable!" );
    }

    PDBG ( "We have %llu blocks with a size of %zu bytes",
           _blk_cnt, _blk_size );
    /* check support for symlinks */
    _supports_symlinks = check_symlinks();
    //new (Genode::env()->heap()) Sync(ep);
}

void  readb ( int count, int sector, char * buff )
{
    int drv = 0;
    PDBG ( "disk_read(drv=%u, buff=%p, sector=%u, count=%u) called.",
           drv, buff, ( unsigned int ) sector, count );

    for ( int i = 0; i < count; i = i + 2 )
    {
        /* allocate packet-descriptor for reading */
        Block::Packet_descriptor p ( _source->alloc_packet ( 2 * _blk_size ),
                                     Block::Packet_descriptor::READ, sector + i, 2 );
        _source->submit_packet ( p );
        p = _source->get_acked_packet();

        if ( !p.succeeded() )
        {
            PERR ( "Could not read block(s)" );
            _source->release_packet ( p );
            return ;
        }

        memcpy ( ( void * ) ( buff + 512 * i ), _source->packet_content ( p ), 1024 );
        _source->release_packet ( p );
    }

    PDBG ( "Block Read successful\n" );
}
void writeb ( int count, int sector, char * buff )
{
    PDBG ( "writing back data to block device\n" );
    /* allocate packet-descriptor for writing */
    Block::Packet_descriptor p ( _source->alloc_packet ( count * _blk_size ),
                                 Block::Packet_descriptor::WRITE, sector, count );
    memcpy ( _source->packet_content ( p ), ( void * ) buff, count * _blk_size );
    _source->submit_packet ( p );
    p = _source->get_acked_packet();

    if ( !p.succeeded() )
    {
        PERR ( "Could not read block(s)" );
    }

    _source->release_packet ( p );
}

bool File_system::supports_symlinks()
{
    return _supports_symlinks;
}
