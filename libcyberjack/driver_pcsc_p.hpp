/***************************************************************************
    begin       : Thu Mar 26 2009
    copyright   : (C) 2009 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef LIBCYBERJACK_DRIVER_PCSC_P_HPP
#define LIBCYBERJACK_DRIVER_PCSC_P_HPP

#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif

#include <PCSC/winscard.h>


/* Set structure elements aligment on bytes
 * http://gcc.gnu.org/onlinedocs/gcc/Structure_002dPacking-Pragmas.html */
#ifdef __APPLE__
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

typedef struct {
  uint8_t SAD;
  uint8_t DAD;
  uint16_t BufferLength;
  uint8_t buffer;
} MCTUniversal_t;

#ifdef __APPLE__
#pragma pack()
#else
#pragma pack(pop)
#endif


#ifdef __cplusplus
}
#endif

#endif

