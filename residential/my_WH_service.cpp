#include "my_WH_service.h"


#include <memory>
#include <staff/utils/Log.h>
#include <staff/common/logoperators.h>
#include <staff/common/Exception.h>
#include <staff/client/ServiceFactory.h>
#include "my_wheater.h"

using namespace std;

//#include "soapwebServiceSoap12BindingProxy.h"
//#include "StdAfx.h"
 std::auto_ptr< ::my_wheater > pWHeater(::staff::ServiceFactory::Inst().GetService< ::my_wheater >());

  //  STAFF_ASSERT(pWHeater.get(), "Cannot get client for service WHeater!");


CLASS* my_WH_service::oclass = NULL;

my_WH_service::my_WH_service(MODULE *module)
{
	if (oclass==NULL)
	{
		oclass = gl_register_class(module,"my_WH_service",sizeof(my_WH_service),PC_BOTTOMUP|PC_AUTOLOCK);
		if (oclass==NULL)
			GL_THROW("unable to register object class implemented by %s", __FILE__);

		if (gl_publish_variable(oclass,
				PT_double,"a",PADDR(a),PT_DESCRIPTION, "Total ",
				NULL)<1) 
			GL_THROW("unable to publish properties in %s",__FILE__);
		
		memset(this,0,sizeof(my_WH_service));
	}
}

int my_WH_service::create()
{

	
	return SUCCESS;
}

int my_WH_service::init(OBJECT *parent)
{
	a=9;
	return SUCCESS;
}

int my_WH_service::isa(char *classname)
{
	return strcmp(classname,"my_WH_service")==0;
}

TIMESTAMP my_WH_service::sync(TIMESTAMP t0, TIMESTAMP t1)
{
 double heating_element_capacity = 0;
 double temperature = 0;
 double tank_setpoint = 0;
 double tank_UA = 0;
 double water_demand = 0;
 double tank_volume = 0;
 double thermostat_deadband = 0;
 int WHsyncResult = pWHeater->WHsync(t0, t1, heating_element_capacity, temperature, tank_setpoint, tank_UA, water_demand, tank_volume, thermostat_deadband);
 
 return TS_NEVER;
}
/*
TIMESTAMP my_WH_service::sync(TIMESTAMP t0, TIMESTAMP t1)
{

	int WHsyncResult = pWHeater->WHsync(1, 1, 4, 110, 120, 3.3, 0.7, 20, 4);
	printf("      %d ", WHsyncResult);
	
		return TS_NEVER;
}*/

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE
//////////////////////////////////////////////////////////////////////////

EXPORT int create_my_WH_service(OBJECT **obj, OBJECT *parent)
{
	try
	{
		*obj = gl_create_object(my_WH_service::oclass);
		if (*obj!=NULL)
		{
			my_WH_service *my = OBJECTDATA(*obj,my_WH_service);
			gl_set_parent(*obj,parent);
			return my->create();
		}
	}
	catch (const char *msg)
	{
		gl_error("create_my_WH_service: %s", msg);
		return 0;
	}
	return 1;
}

EXPORT int init_my_WH_service(OBJECT *obj, OBJECT *parent)
{
	try
	{
		if (obj!=NULL){
			return OBJECTDATA(obj,my_WH_service)->init(parent);
		}
	}
	catch (const char *msg)
	{
		char name[64];
		gl_error("init_my_WH_service(obj=%s): %s", gl_name(obj,name,sizeof(name)), msg);
		return 0;
	}
	return 1;
}

EXPORT int isa_my_WH_service(OBJECT *obj, char *classname)
{
	if(obj != 0 && classname != 0){
		return OBJECTDATA(obj,my_WH_service)->isa(classname);
	} else {
		return 0;
	}
}

EXPORT TIMESTAMP sync_my_WH_service(OBJECT *obj, TIMESTAMP t1, PASSCONFIG pass)
{
	TIMESTAMP t2 = TS_NEVER;
	my_WH_service *my = OBJECTDATA(obj,my_WH_service);
	switch (pass) {
	case PC_BOTTOMUP:
		t2 = my->sync(obj->clock,t1);
		obj->clock = t1;
		break;
	default:
		break;
	}
	return t2;
}