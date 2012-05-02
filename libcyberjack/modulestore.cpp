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



#include "modulestore.hpp"

#include <libxml/tree.h>
#include <libxml/parser.h>


#include <strings.h>
#include <string.h>
#include <inttypes.h>

#include <sys/types.h>
#include <dirent.h>



namespace Cyberjack {



ModuleStore::ModuleStore(const char *s) {
  if (s && *s)
    m_path=std::string(s);
}



ModuleStore::~ModuleStore() {
}



void ModuleStore::readXmlModules(xmlNodePtr n) {
  xmlNodePtr nn;
  const char *s;

  s=(const char*) xmlGetProp(n, BAD_CAST "keyfile");
  if (s && *s)
    m_keyFileName=std::string(s);

  nn=n->children;
  while(nn) {
    if (nn->type==XML_ELEMENT_NODE) {
      if (nn->name && strcasecmp((const char*)nn->name, "module")==0) {
	Module m(m_path.c_str());

	m.readXml(nn);
	m_modules.push_back(m);
      }
    }

    nn=nn->next;
  }
}



void ModuleStore::readXmlTemplates(xmlNodePtr n) {
  xmlNodePtr nn;

  nn=n->children;
  while(nn) {
    if (nn->type==XML_ELEMENT_NODE) {
      if (nn->name && strcasecmp((const char*)nn->name, "template")==0) {
	MTemplate t;

	t.readXml(nn);
	m_templates.push_back(t);
      }
    }

    nn=nn->next;
  }
}



void ModuleStore::readXmlSetup(xmlNodePtr n) {
  xmlNodePtr nn;
  xmlNodePtr nnn;
  const char *s;

  s=(const char*) xmlGetProp(n, BAD_CAST "type");
  if (s && *s)
    m_readerType=std::string(s);

  nn=n->children;
  while(nn) {
    if (nn->type==XML_ELEMENT_NODE) {
      if (nn->name) {
	if (strcasecmp((const char*)nn->name, "templates")==0)
	  readXmlTemplates(nn);
	else if (strcasecmp((const char*)nn->name, "modules")==0)
	  readXmlModules(nn);
	else if (strcasecmp((const char*)nn->name, "version")==0) {
	  nnn=nn->children;
	  while(nnn) {
	    if (nnn->type==XML_TEXT_NODE) {
	      s=(const char*)nnn->content;
	      if (s && *s)
		m_version=std::string(s);
	      break;
	    }
	    nnn=nnn->next;
	  }
	}
	else if (strcasecmp((const char*)nn->name, "hw_string")==0) {
	  nnn=nn->children;
	  while(nnn) {
	    if (nnn->type==XML_TEXT_NODE) {
	      s=(const char*)nnn->content;
	      if (s && *s)
		m_hwString=std::string(s);
	      break;
	    }
	    nnn=nnn->next;
	  }
	}
      }
    }

    nn=nn->next;
  }
}



bool ModuleStore::readXmlFile() {
  xmlDocPtr doc;
  xmlNodePtr n;
  std::string fname;
  bool setupLoaded=false;

  fname=m_path+"/version.xml";

  doc=xmlParseFile(fname.c_str());
  if (doc==NULL) {
    fprintf(stderr,
	    "ERROR: Could not load xml file [%s]\n",
	    fname.c_str());
    xmlFreeDoc(doc);
    return false;
  }

  n=xmlDocGetRootElement(doc);
  if (n==NULL) {
    fprintf(stderr,
	    "ERROR: No root element in xml file [%s]\n",
	    fname.c_str());
    xmlFreeDoc(doc);
    return false;
  }

  if (n->name && strcasecmp((const char*)(n->name), "setup")==0) {
    readXmlSetup(n);
    setupLoaded=true;
  }
  xmlFreeDoc(doc);

  if (!setupLoaded) {
    fprintf(stderr,
	    "ERROR: No setup element in xml file [%s]\n",
	    fname.c_str());
    return false;
  }

  return true;
}



bool ModuleStore::readModuleStoresFrom(const char *path, std::list<ModuleStore> &slist) {
  DIR *d;
  int loaded=0;

  d=opendir(path);
  if (d==NULL) {
    fprintf(stderr, "No module store found at [%s]\n", path);
    return false;
  }
  else {
    struct dirent *de;

    while( (de=readdir(d)) ) {
      if (de->d_name[0]!='.') {
	std::string str;
	str=std::string(path);
	str+="/";
	str+=std::string(de->d_name);
	fprintf(stderr, "Trying [%s]\n", str.c_str());

	ModuleStore ms(str.c_str());
	if (ms.readXmlFile()) {
	  loaded++;
	  slist.push_back(ms);
	}
      }
    }
    closedir(d);
    if (loaded>0)
      return true;
    else {
      fprintf(stderr, "No module store found in [%s]\n", path);
      return false;
    }
  }
}



bool ModuleStore::readModuleStores(std::list<ModuleStore> &slist) {
  int okCounter=0;
  int errorCounter=0;
  bool rv;

  rv=readModuleStoresFrom("/usr/local/lib/cyberjack/firmware", slist);
  if (rv) okCounter++;
  else errorCounter++;
  rv=readModuleStoresFrom("/usr/lib/cyberjack/firmware", slist);
  if (rv) okCounter++;
  else errorCounter++;
  rv=readModuleStoresFrom("/lib/cyberjack/firmware", slist);
  if (rv) okCounter++;
  else errorCounter++;

  if (okCounter==0) {
    fprintf(stderr, "ERROR: Could not load any module store\n");
    return false;
  }
  return true;
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



bool ModuleStore::readKeyFile() {
  uint8_t buffer[64*1024];
  std::string fname;
  int rv;

  /* read binary */
  fname=m_path;
  fname+="/";
  fname+=m_keyFileName;
  rv=readFile(fname.c_str(), buffer, sizeof(buffer)-1);
  if (rv<1) {
    fprintf(stderr, "ERROR: Could not read keyfile [%s]\n", fname.c_str());
    return false;
  }
  m_keyFileData=std::string((const char*)buffer, rv);

  return true;
}







} /* namespace */


#endif /* if xml2 */

