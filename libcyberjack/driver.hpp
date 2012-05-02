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


#ifndef LIBCYBERJACK_DRIVER_HPP
#define LIBCYBERJACK_DRIVER_HPP


#include <inttypes.h>

#include <list>
#include <string>

#include "cyberjack_l.h"


namespace Cyberjack {

  class Driver;


  enum {
    ErrorCode_Ok=0,
    ErrorCode_Generic=-1,
    ErrorCode_Invalid=-2,
    ErrorCode_NoDevice=-3,
    ErrorCode_NotSupported=-4,
    ErrorCode_IO=-5,
    ErrorCode_NoService=-6
  };

  enum {
    Object_None=0,
    Object_Reader,
    Object_Card0,
    Object_Card1,
    Object_Card2,
    Object_Card3
  };

  enum {
    BusType_None=0,
    BusType_UsbRaw,
    BusType_UsbTty,
    BusType_Serial,
    BusType_Pcsc,
  };


  class Reader {
  public:

    virtual ~Reader();

    Driver *getDriver() const { return m_driver;};

    /** @name Lowlevel Methods
     *
     */
    /*@{*/
    virtual int connect(int object);
    virtual int disconnect();
    virtual int connectedObject();

    virtual int sendApdu(uint8_t *dad,
                         uint8_t *sad,
                         uint16_t cmd_len,
                         const uint8_t *cmd,
                         uint16_t *response_len,
                         uint8_t *response);

    virtual int test(std::string &result);
    /*@}*/


    /** @name Reader Information
     *
     * Some of the methods described here need @ref gatherInfo() to be called
     * before.
     */
    /*@{*/
    const std::string &getName() const { return m_name;};
    const std::string &getProductString() const { return m_productString;};
    const std::string &getSerial() const { return m_serial;};
    int getBusType() const { return m_busType;};
    uint32_t getVendorId() const { return m_vendorId;};
    uint32_t getProductId() const { return m_productId;};

    /**
     * This information is only available after @ref gatherInfo has been called.
     */
    const std::list<cj_ModuleInfo> &getModuleInfos() const { return m_moduleInfos;};

    /**
     * This information is only available after @ref gatherInfo has been called.
     */
    const cj_ReaderInfo &getReaderInfo() const { return m_readerInfo;};

    /**
     * Every time @ref Driver::enumReaders() is called an internal update counter
     * in the driver is incremented. New readers enumerated by that method always get
     * the update counter value "0". Devices which have already been available in previous
     * @ref Driver::enumReaders() calls are assigned the driver's current update counter
     * value. So in order to determine whether a reader is new, old or away is to compare
     * the value returned here to that returned by @ref Driver::getUpdateCounter():
     * <ul>
     *  <li>reader's update counter is 0: device has just been enumerated</li>
     *  <li>reader's update counter < driver's update counter: device is gone</li>
     *  <li>reader's update counter == driver's update counter: device is still available<li>
     * </ul>
     */
    uint32_t getUpdateCounter() const { return m_updateCounter;};

    /**
     * This method should only be called by a Driver implementation.
     */
    void setUpdateCounter(uint32_t i) { m_updateCounter=i;};
    /*@}*/


    /** @name Common Highlevel Methods
     *
     * Methods in this group send special APDU's to get extended information about
     * a reader or to modify its setup.
     */
    /*@{*/

    /**
     * Retrieves reader- and module information from the device and stores them
     * internally.
     */
    int gatherInfo(bool doConnect);

    /**
     * Send the given module data to the driver. This is the first step when
     * flashing a module/kernel to the reader.
     * Usually the data comes from a file ending in ".bin".
     */
    int sendModuleToFlash(const uint8_t *pdata, uint32_t ldata);

    /**
     * Send the given signature data to the driver. This is the second step when
     * flashing a module/kernel to the reader.
     * Usually the data comes from a file ending in ".sgn".
     */
    int sendSignatureToFlash(const uint8_t *pdata, uint32_t ldata);

    /**
     * Flash the data previously send using @ref sendModuleToFlash and
     * @ref sendSignatureToFlash.
     */
    int flash();

    /**
     * Delete all currently installed modules from the reader.
     */
    int deleteAllModules(bool doConnect);

    /**
     * Send new keys to the reader. Such keys are used to verify the integrity
     * of the kernel and other modules.
     */
    int updateKeys(const uint8_t *pdata, uint32_t ldata);
    /*@}*/

    void setVendorId(uint32_t id) { m_vendorId=id;};
    void setProductId(uint32_t id) { m_productId=id;};

    void setProductString(const std::string &s) { m_productString=s;};
    void setSerialNumber(const std::string &s) { m_serial=s;};

  protected:
    Reader(Driver *driver,
           const std::string &name,
           const std::string &productString,
           const std::string &serial,
           int busType,
           uint32_t vendorId,
           uint32_t productId);

    int _getReaderInfo(cj_ReaderInfo *readerInfo);
    int _getNumberOfModules();
    int _getModuleInfo(int idx, cj_ModuleInfo *modInfo);

    int _sendData(uint8_t ins, const uint8_t *pdata, uint32_t ldata);

    Driver *m_driver;
    std::string m_name;
    std::string m_productString;
    std::string m_serial;
    int m_busType;
    uint32_t m_vendorId;
    uint32_t m_productId;

    uint32_t m_updateCounter;

    std::list<cj_ModuleInfo> m_moduleInfos;
    cj_ReaderInfo m_readerInfo;

  };



  class Driver {
  public:
    virtual ~Driver();

    virtual int open();
    virtual int enumReaders();
    virtual int close();

    std::list<Reader*> &getReaders() { return m_readerList;};
    uint32_t getUpdateCounter() const { return m_updateCounter;};

    void removeOldReaders();
    void clearReaderList();

  protected:
    Driver();

    void setUpdateCounter(uint32_t i) { m_updateCounter=i;};

    uint32_t m_updateCounter;
    std::list<Reader*> m_readerList;
  };




} /* namespace */


#endif



