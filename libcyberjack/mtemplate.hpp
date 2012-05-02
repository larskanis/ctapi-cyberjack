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


#ifndef LIBCYBERJACK_MTEMPLATE_HPP
#define LIBCYBERJACK_MTEMPLATE_HPP


#include <libxml/tree.h>
#include <libxml/parser.h>


#include <string>
#include <list>



namespace Cyberjack {



  class MTemplate {
  public:
    MTemplate();
    ~MTemplate();

    const std::string &getDescr() const { return m_descr;};

    const std::list<std::string> &getModuleNames() const { return m_moduleNames;};
    const std::string &getTemplateName() const { return m_id;};

    /** read the module description from the given XML node (pointing to a "<module>"
     * node).
     */
    void readXml(xmlNodePtr n);

  protected:
    std::string m_id;
    std::string m_descr;
    std::list<std::string> m_moduleNames;

  };




} /* namespace */


#endif



