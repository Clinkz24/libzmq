/*
    Copyright (c) 2007-2009 FastMQ Inc.

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    Lesser GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../bindings/c/zmq.h"

#include "downstream.hpp"
#include "err.hpp"
#include "pipe.hpp"

zmq::downstream_t::downstream_t (class app_thread_t *parent_) :
    socket_base_t (parent_),
    current (0)
{
    options.requires_in = false;
    options.requires_out = true;
}

zmq::downstream_t::~downstream_t ()
{
}

void zmq::downstream_t::xattach_pipes (class reader_t *inpipe_,
    class writer_t *outpipe_)
{
    zmq_assert (!inpipe_ && outpipe_);
    pipes.push_back (outpipe_);
}

void zmq::downstream_t::xdetach_inpipe (class reader_t *pipe_)
{
    //  There are no inpipes, so this function shouldn't be called at all.
    zmq_assert (false);
}

void zmq::downstream_t::xdetach_outpipe (class writer_t *pipe_)
{
    zmq_assert (pipe_);
    pipes.erase (pipes.index (pipe_));
}

void zmq::downstream_t::xkill (class reader_t *pipe_)
{
    //  There are no inpipes, so this function shouldn't be called at all.
    zmq_assert (false);
}

void zmq::downstream_t::xrevive (class reader_t *pipe_)
{
    //  There are no inpipes, so this function shouldn't be called at all.
    zmq_assert (false);
}

int zmq::downstream_t::xsetsockopt (int option_, const void *optval_,
    size_t optvallen_)
{
    //  No special option for this socket type.
    errno = EINVAL;
    return -1;
}

int zmq::downstream_t::xsend (zmq_msg_t *msg_, int flags_)
{
    //  If there are no pipes we cannot send the message.
    if (pipes.empty ()) {
        errno = EAGAIN;
        return -1;
    }

    //  Move to the next pipe (load-balancing).
    current++;
    if (current >= pipes.size ())
        current = 0;

    //  TODO: Implement this once queue limits are in-place.
    zmq_assert (pipes [current]->check_write (zmq_msg_size (msg_)));

    //  Push message to the selected pipe.
    pipes [current]->write (msg_);
    pipes [current]->flush ();

    //  Detach the message from the data buffer.
    int rc = zmq_msg_init (msg_);
    zmq_assert (rc == 0);

    return 0;
}

int zmq::downstream_t::xflush ()
{
    //  TODO: Maybe there's a point in flushing messages downstream.
    //  It may be useful in the case where number of messages in a single
    //  transaction is much greater than the number of attached pipes.
    errno = ENOTSUP;
    return -1;

}

int zmq::downstream_t::xrecv (zmq_msg_t *msg_, int flags_)
{
    errno = ENOTSUP;
    return -1;
}

bool zmq::downstream_t::xhas_in ()
{
    return false;
}

bool zmq::downstream_t::xhas_out ()
{
    //  TODO: Modify this code once pipe limits are in place.
    return true;
}


