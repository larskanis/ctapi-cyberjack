/***************************************************************************
    begin       : Tue Mar 24 2009
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


#ifndef LIBCYBERJACK_DRIVER_CTAPI_HPP
#define LIBCYBERJACK_DRIVER_CTAPI_HPP


#include "driver.hpp"



namespace Cyberjack {


  class DriverCtapi: public Driver {
  public:
    DriverCtapi();
    virtual ~DriverCtapi();

    virtual int open();
    virtual int enumReaders();
    virtual int close();
  };




} /* namespace */


#endif



