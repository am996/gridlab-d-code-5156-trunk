#ifndef _WH_as_service_h
#define _WH_as_service_h

#include <stdarg.h>
#include "gridlabd.h"


#include "residential.h"
class WH_as_service : public gld_object {
public:
	WH_as_service(MODULE *);
	int create(void);
	int init(OBJECT *parent);
	int isa(char *classname);
	TIMESTAMP presync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP sync(TIMESTAMP t0, TIMESTAMP t1);
	TIMESTAMP postsync(TIMESTAMP t0, TIMESTAMP t1);
	static CLASS *oclass;
public:
	double a;

};

#endif //_WH_as_service_h