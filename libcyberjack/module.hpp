/***************************************************************************
    begin       : Tue Apr 06 2010
    copyright   : (C) 2010 by Martin Preuss
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


#ifndef LIBCYBERJACK_MODULE_HPP
#define LIBCYBERJACK_MODULE_HPP


#include <libxml/tree.h>
#include <libxml/parser.h>


#include <string>



namespace Cyberjack {



  class Module {
  public:
    Module(const char *path);
    ~Module();

    const std::string &getReaderType() const { return m_readerType;};
    const std::string &getModuleName() const { return m_moduleName;};
    const std::string &getDescrAny() const { return m_descrAny;};
    const std::string &getDescrDe() const { return m_descrDe;};

    std::string getDescr() const;

    const std::string &getBinFileName() const { return m_binFileName;};
    const std::string &getSigFileName() const { return m_sigFileName;};

    const std::string &getBinFileData() const { return m_binFileData;};
    const std::string &getSigFileData() const { return m_sigFileData;};

    /** read the module description from the given XML node (pointing to a "<module>"
     * node).
     */
    void readXml(xmlNodePtr n);

    bool readModuleBinaries();

  protected:
    std::string m_path;
    std::string m_readerType;
    std::string m_moduleName;
    std::string m_descrDe;
    std::string m_descrAny;

    std::string m_binFileName;
    std::string m_sigFileName;

    std::string m_binFileData;
    std::string m_sigFileData;

    bool m_force;
  };




} /* namespace */


#endif



