/* * 
 *  This file is part of Feng
 * 
 * Copyright (C) 2007 by LScube team <team@streaming.polito.it> 
 * See AUTHORS for more details 
 *  
 * Feng is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * Feng is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License
 * along with Feng; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *  
 * */

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <fenice/demuxer.h>
#include <fenice/mediaparser.h>
#include <fenice/mediaparser_module.h>
#include <fenice/utils.h>
#include <fenice/fnc_log.h>
#include <fenice/debug.h>

static const MediaParserInfo info = {
    "MPA",
    MP_audio
};

static int mpa_init(MediaProperties *properties, void **private_data) 
{
    INIT_PROPS

    return 0;
}

static int mpa_parse(void *track, uint8_t *data, long len, uint8_t *extradata, 
          long extradata_len)
{
    Track *tr = (Track *)track;
    uint32_t mtu = DEFAULT_MTU; //FIXME get it from SETUP
    int32_t offset, rem;
    uint8_t dst[mtu];
    rem = len;

    if (mtu >= len + 4) {
        memset (dst, 0, 4);
        memcpy (dst + 4, data, len);
        if (bp_write(tr->buffer, 0, tr->properties->mtime, 0, 0,
                              dst, len + 4)) {
                fnc_log(FNC_LOG_ERR, "Cannot write bufferpool");
                return ERR_ALLOC;
        }
        fnc_log(FNC_LOG_VERBOSE, "[mp3] no frags");
    } else {
        do {
            offset = len - rem;
            if (offset & 0xffff0000) return ERR_ALLOC;
            memcpy (dst + 4, data + offset, min(mtu - 4, rem));
            offset = htonl(offset & 0xffff);
            memcpy (dst, &offset, 4);

            if (bp_write(tr->buffer, 0, tr->properties->mtime, 0, 0,
                                  dst, min(mtu, rem + 4))) {
                fnc_log(FNC_LOG_ERR, "Cannot write bufferpool");
                return ERR_ALLOC;
            }
            rem -= mtu - 4;
            fnc_log(FNC_LOG_VERBOSE, "[mp3] frags");
        } while (rem >= 0);
    }
    fnc_log(FNC_LOG_VERBOSE, "[mp3]Frame completed");
    return ERR_NOERROR;
}


static int mpa_uninit(void *private_data)
{
    return 0;
}

FNC_LIB_MEDIAPARSER(mpa);
