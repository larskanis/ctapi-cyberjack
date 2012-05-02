/***************************************************************************
 * project      : Cyberjack Diagnoses Tool
    begin       : Fri Jan 26 2007
    copyright   : (C) 2007 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *             This file is licensed under the GPL version 2.              *
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CHECKSUITE_H
#define CHECKSUITE_H


class CheckSuite;


#include <list>
#include <inttypes.h>

#include "checkmodule.h"


#define CHECKSUITE_SPECIAL_ERRORCODE (-126)

#define CHECKSUITE_FLAGS_HAVE_PCSCD   0x00000001
#define CHECKSUITE_FLAGS_HAVE_OPENCT  0x00000002
#define CHECKSUITE_FLAGS_HAVE_LCC1    0x00000004
#define CHECKSUITE_FLAGS_HAVE_LCC2    0x00000008
#define CHECKSUITE_FLAGS_HAVE_LCC3    0x00000010
#define CHECKSUITE_FLAGS_HAVE_LCC4    0x00000020



class CheckSuite {
private:
  typedef int8_t (*CT_INITNAME_FN)(uint16_t ctn, const char *devName);
  typedef int8_t (*CT_DATA_FN)(uint16_t ctn,
			       uint8_t *dad,
			       uint8_t *sad,
			       uint16_t cmd_len,
			       const uint8_t *cmd,
			       uint16_t *response_len,
			       uint8_t *response);
  typedef int8_t (*CT_CLOSE_FN)(uint16_t ctn);

private:
  void *_driverHandle;
  CT_INITNAME_FN _ctInitNameFn;
  CT_DATA_FN _ctDataFn;
  CT_CLOSE_FN _ctCloseFn;

  uint32_t _flags;
  std::string _distName;
  std::string _distVersion;

  std::list<CheckModule*> _moduleList;

protected:
  virtual bool beginCheck(const char *title,
			  int doneChecks,
                          int totalChecks);

  virtual bool endCheck(const char *title,
			int doneChecks,
			int totalChecks,
			bool result);

public:
  CheckSuite();
  virtual ~CheckSuite();

  bool performChecks(std::string &xmlString,
		     std::string &reportString,
		     std::string &hintString);

  void addStandardModules(bool withReaderCheck);

  void setDriverHandle(void *p);
  void *getDriverHandle();

  void addCheckModule(CheckModule *m);

  uint32_t getFlags() const { return _flags;};
  void setFlags(uint32_t f);
  void addFlags(uint32_t f);
  void delFlags(uint32_t f);

  void setDist(const std::string &dname, const std::string &dver);
  const std::string &getDistName() const { return _distName;};
  const std::string &getDistVersion() const { return _distVersion;};

  int8_t ctInitName(uint16_t ctn, const char *devName);

  int8_t ctData(uint16_t ctn,
		uint8_t *dad,
		uint8_t *sad,
		uint16_t cmd_len,
		const uint8_t *cmd,
		uint16_t *response_len,
		uint8_t *response);

  int8_t ctClose(uint16_t ctn);

};



#endif



