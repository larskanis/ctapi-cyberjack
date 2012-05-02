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


#ifndef LIBCYBERJACK_MODULESTORE_HPP
#define LIBCYBERJACK_MODULESTORE_HPP


#include "module.hpp"
#include "mtemplate.hpp"

#include <string>
#include <list>



namespace Cyberjack {



  class ModuleStore {
  public:
    ModuleStore(const char *path);
    ~ModuleStore();

    const std::string &getPath() const { return m_path;};

    const std::string &getReaderType() const { return m_readerType;};
    const std::string &getKeyFileName() const { return m_keyFileName;};

    const std::string &getVersionString() const { return m_version;};
    const std::string &getHwString() const { return m_hwString;};

    const std::string &getKeyFileData() const { return m_keyFileData;};

    /** read the module description from the given XML node (pointing to a "<module>"
     * node).
     */
    bool readXmlFile();
    bool readKeyFile();

    static bool readModuleStores(std::list<ModuleStore> &slist);

    std::list<Module> &getModules() { return m_modules;};
    std::list<MTemplate> &getTemplates() { return m_templates;};


  protected:
    std::string m_path;
    std::string m_readerType;
    std::string m_keyFileName;
    std::string m_version;
    std::string m_hwString;

    std::list<Module> m_modules;
    std::list<MTemplate> m_templates;

    std::string m_keyFileData;

    void readXmlModules(xmlNodePtr n);
    void readXmlTemplates(xmlNodePtr n);
    void readXmlSetup(xmlNodePtr n);

    static bool readModuleStoresFrom(const char *path, std::list<ModuleStore> &slist);

  };




} /* namespace */


#endif



