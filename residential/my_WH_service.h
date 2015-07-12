
#include <stdarg.h>
#include "gridlabd.h"

#ifndef _my_WH_service_h
#define _my_WH_service_h

#include "residential.h"
class my_WH_service : public gld_object {
public:
	my_WH_service(MODULE *);
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

#endif //_my_WH_service_h