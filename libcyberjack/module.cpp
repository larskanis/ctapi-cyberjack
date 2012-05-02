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



#include "module.hpp"

#include <ctype.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>



namespace Cyberjack {



Module::Module(const char *path)
:m_path(std::string(path))
,m_force(false)
{
}



Module::~Module(){
}



std::string Module::getDescr() const {
  if (!m_descrDe.empty())
    return m_descrDe;
  else if (!m_descrAny.empty())
    return m_descrAny;
  else
    return "Unbenanntes Modul";
}



void Module::readXml(xmlNodePtr n) {
  xmlNodePtr nn;
  xmlNodePtr nnn;
  const char *s;

  s=(const char*) xmlGetProp(n, BAD_CAST "type");
  if (s && *s)
    m_readerType=std::string(s);

  s=(const char*) xmlGetProp(n, BAD_CAST "fname");
  if (s && *s)
    m_moduleName=std::string(s);

  s=(const char*) xmlGetProp(n, BAD_CAST "force");
  if (s && *s && strcasecmp(s, "1")==0)
    m_force=true;

  nn=n->children;
  while(nn) {
    if (nn->type==XML_ELEMENT_NODE) {
      if (nn->name) {
	if (strcasecmp((const char*)nn->name, "binary")==0) {
	  nnn=nn->children;
	  while(nnn) {
	    if (nnn->type==XML_TEXT_NODE) {
	      s=(const char*)nnn->content;
	      if (s && *s)
		m_binFileName=std::string(s);
              break;
	    }
	    nnn=nnn->next;
	  }
	}
	else if (strcasecmp((const char*)nn->name, "signature")==0) {
	  nnn=nn->children;
	  while(nnn) {
	    if (nnn->type==XML_TEXT_NODE) {
	      s=(const char*)nnn->content;
	      if (s && *s)
		m_sigFileName=std::string(s);
	      break;
	    }
	    nnn=nnn->next;
	  }
	}
	else if (strcasecmp((const char*)nn->name, "desc")==0) {
	  s=(const char*) xmlGetProp(nn, BAD_CAST "langid");
	  if (s && *s) {
	    if (strcasecmp(s, "de")==0) {
	      nnn=nn->children;
	      while(nnn) {
		if (nnn->type==XML_TEXT_NODE) {
		  s=(const char*)nnn->content;
		  if (s && *s)
		    m_descrDe=std::string(s);
		  break;
		}
		nnn=nnn->next;
	      }
	    }
            else if (strcasecmp(s, "*")==0) {
	      nnn=nn->children;
	      while(nnn) {
		if (nnn->type==XML_TEXT_NODE) {
		  s=(const char*)nnn->content;
		  if (s && *s)
		    m_descrAny=std::string(s);
		  break;
		}
		nnn=nnn->next;
	      }
	    }
	  }
	}
      }
    }

    nn=nn->next;
  }
}



static int readFile(const char *fname, uint8_t *buffer, uint32_t size) {
  FILE *f;
  uint8_t *p;
  int len;

  f=fopen(fname, "r");
  if (f==NULL)
    return -1;

  p=buffer;
  len=0;
  while(!feof(f)) {
    int rv;
    int l;

    l=size;
    if (l<1) {
      fprintf(stderr, "ERROR: Buffer too small\n");
      return -1;
    }
    if (l>1024)
      l=1024;
    rv=fread(p, 1, l, f);
    if (rv==0)
      break;
    p+=rv;
    len+=rv;
    size-=rv;
  }
  fclose(f);
  return len;
}



bool Module::readModuleBinaries() {
  uint8_t buffer[64*1024];
  std::string fname;
  int rv;

  /* read binary */
  fname=m_path;
  fname+="/";
  fname+=m_binFileName;
  rv=readFile(fname.c_str(), buffer, sizeof(buffer)-1);
  if (rv<1) {
    fprintf(stderr, "ERROR: Could not read binary [%s]\n", fname.c_str());
    return false;
  }
  m_binFileData=std::string((const char*)buffer, rv);

  /* read signature file */
  fname=m_path;
  fname+="/";
  fname+=m_sigFileName;
  rv=readFile(fname.c_str(), buffer, sizeof(buffer)-1);
  if (rv<1) {
    fprintf(stderr, "ERROR: Could not signature binary [%s]\n", fname.c_str());
    return false;
  }
  m_sigFileData=std::string((const char*)buffer, rv);

  return true;
}




} /* namespace */


#endif /* if XML2 */


