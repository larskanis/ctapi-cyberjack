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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#ifdef HAVE_LIBXML2



#include "mtemplate.hpp"

#include <ctype.h>
#include <strings.h>




namespace Cyberjack {



MTemplate::MTemplate()
{
}



MTemplate::~MTemplate(){
}



void MTemplate::readXml(xmlNodePtr n) {
  xmlNodePtr nn;
  xmlNodePtr nnn;
  const char *s;

  m_moduleNames.clear();

  s=(const char*) xmlGetProp(n, BAD_CAST "id");
  if (s && *s)
    m_id=std::string(s);

  nn=n->children;
  while(nn) {
    if (nn->type==XML_ELEMENT_NODE) {
      if (nn->name) {
	if (strcasecmp((const char*)nn->name, "activate_module")==0) {
	  s=(const char*) xmlGetProp(nn, BAD_CAST "fname");
	  if (s && *s)
	    m_moduleNames.push_back(std::string(s));
	}
	else if (strcasecmp((const char*)nn->name, "desc")==0) {
	  s=(const char*) xmlGetProp(nn, BAD_CAST "langid");
	  if (s && *s && strcasecmp(s, "de")==0) {
	    nnn=nn->children;
	    while(nnn) {
	      if (nnn->type==XML_TEXT_NODE) {
		s=(const char*)nnn->content;
		if (s && *s)
		  m_descr=std::string(s);
		break;
	      }
	      nnn=nnn->next;
	    }
	  }
	}
      }
    }

    nn=nn->next;
  }
}



} /* namespace */


#endif /* if xml2 */

