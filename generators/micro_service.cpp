/** $Id: micro_service.cpp 4738 2014-07-03 00:55:39Z dchassin $
	Copyright (C) 2008 Battelle Memorial Institute
	@file micro_service.cpp
	@defgroup micro_service
	@ingroup generators

 @{
 **/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <memory>
#include <staff/utils/Log.h>
#include <staff/common/logoperators.h>
#include <staff/common/Exception.h>
#include <staff/client/ServiceFactory.h>



#include "generators.h"
#include "micro_service.h"
#include "timestamp.h"
#include "my_microturbine.h"



#define HOUR 3600 * TS_SECOND
std::unique_ptr< ::my_microturbine > pmy_microturbine(::staff::ServiceFactory::Inst().GetService< ::my_microturbine >());

CLASS *micro_service::oclass = NULL;
micro_service *micro_service::defaults = NULL;
static PASSCONFIG passconfig = PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN;
static PASSCONFIG clockpass = PC_BOTTOMUP;

/* Class registration is only called once to register the class with the core */
micro_service::micro_service(MODULE *module)
{
	if (oclass==NULL)
	{
		oclass = gl_register_class(module,"micro_service",sizeof(micro_service),passconfig|PC_AUTOLOCK);
		if (oclass==NULL)
			throw "unable to register class micro_service";
		else
			oclass->trl = TRL_PROOF;

		if (gl_publish_variable(oclass,
			PT_enumeration,"generator_mode",PADDR(gen_mode_v),
				PT_KEYWORD,"UNKNOWN",(enumeration)UNKNOWN,
				PT_KEYWORD,"CONSTANT_V",(enumeration)CONSTANT_V,
				PT_KEYWORD,"CONSTANT_PQ",(enumeration)CONSTANT_PQ,
				PT_KEYWORD,"CONSTANT_PF",(enumeration)CONSTANT_PF,
				PT_KEYWORD,"SUPPLY_DRIVEN",(enumeration)SUPPLY_DRIVEN, //PV must operate in this mode

			PT_enumeration,"generator_status",PADDR(gen_status_v),
				PT_KEYWORD,"OFFLINE",(enumeration)OFFLINE,
				PT_KEYWORD,"ONLINE",(enumeration)ONLINE,	

			PT_enumeration,"power_type",PADDR(power_type_v),
				PT_KEYWORD,"AC",(enumeration)AC,
				PT_KEYWORD,"DC",(enumeration)DC,


			PT_double, "Rinternal", PADDR(Rinternal),
			PT_double, "Rload", PADDR(Rload),
			PT_double, "V_Max[V]", PADDR(V_Max), 
			PT_complex, "I_Max[A]", PADDR(I_Max),
			
			PT_double, "frequency[Hz]", PADDR(frequency),
			PT_double, "Max_Frequency[Hz]", PADDR(Max_Frequency),
			PT_double, "Min_Frequency[Hz]", PADDR(Min_Frequency),
			PT_double, "Fuel_Used[kVA]", PADDR(Fuel_Used),
			PT_double, "Heat_Out[kVA]", PADDR(Heat_Out),
			PT_double, "KV", PADDR(KV), //voltage constant
			PT_double, "Power_Angle", PADDR(Power_Angle),


			PT_double, "Max_P[kW]", PADDR(Max_P), //< maximum real power capacity in kW
			PT_double, "Min_P[kW]", PADDR(Min_P), //< minimus real power capacity in kW
	
			//PT_double, "Max_Q[kVAR]", PADDR(Max_Q), //< maximum reactive power capacity in kVar
			//PT_double, "Min_Q[kVAR]", PADDR(Min_Q), //< minimus reactive power capacity in kVar

			PT_complex, "phaseA_V_Out[kV]", PADDR(phaseA_V_Out), //voltage
			PT_complex, "phaseB_V_Out[kV]", PADDR(phaseB_V_Out),
			PT_complex, "phaseC_V_Out[kV]", PADDR(phaseC_V_Out),
	
	//complex V_Out;
	
			PT_complex, "phaseA_I_Out[A]", PADDR(phaseA_I_Out),    // current
			PT_complex, "phaseB_I_Out[A]", PADDR(phaseB_I_Out),
			PT_complex, "phaseC_I_Out[A]", PADDR(phaseC_I_Out),

	//complex I_Out;

			PT_complex, "power_A_Out", PADDR(power_A_Out),     //power
			PT_complex, "power_B_Out", PADDR(power_B_Out),
			PT_complex, "power_C_Out", PADDR(power_C_Out),

			PT_complex, "VA_Out", PADDR(VA_Out),

			PT_double, "pf_Out", PADDR(pf_Out),
			PT_complex, "E_A_Internal", PADDR(E_A_Internal),
			PT_complex, "E_B_Internal", PADDR(E_B_Internal),
			PT_complex, "E_C_Internal", PADDR(E_C_Internal),

			PT_double, "efficiency", PADDR(efficiency),
			//PT_int64, "generator_mode_choice", PADDR(generator_mode_choice),

			PT_double, "Rated_kVA[kVA]", PADDR(Rated_kVA),
			//PT_double, "Rated_kV[kV]", PADDR(Rated_kV),
		
			//PT_complex, "VA_Out[VA]", PADDR(VA_Out),

			//resistances and max P, Q

			PT_set, "phases", PADDR(phases),

				PT_KEYWORD, "A",(set)PHASE_A,
				PT_KEYWORD, "B",(set)PHASE_B,
				PT_KEYWORD, "C",(set)PHASE_C,
				PT_KEYWORD, "N",(set)PHASE_N,
				PT_KEYWORD, "S",(set)PHASE_S,
			NULL)<1) GL_THROW("unable to publish properties in %s",__FILE__);
		defaults = this;

	


		memset(this,0,sizeof(micro_service));
	}
}


/* Object creation is called once for each object that is created by the core */
int micro_service::create(void) 
{
	memcpy(this,defaults,sizeof(*this));
	/* TODO: set the context-free initial value of properties */
	return 1; /* return 1 on success, 0 on failure */
}


double micro_service::determine_power_angle (complex power_out){ //could also pass in pf_Out as a parameter if needed
	Power_Angle = acos((double) (power_out.Re() / power_out.Mag()));
	return Power_Angle;

}

complex micro_service::determine_source_voltage(complex voltage_out, double r_internal, double r_load){
	//placeholder, probably need to replace with iterative function
	complex Vs = complex(((r_internal + r_load)/(r_load)) * voltage_out.Re(), ((r_internal + r_load)/(r_load)) * voltage_out.Im());
	return Vs;

}

double micro_service::determine_frequency(complex power_out){
	//double Kx = 3 * Rinternal / 3.14159;
	//used linear approximation which does better job than textbook equation...
	//assumes power in W
	double f = power_out.Mag() * 1.5 + 55000;\
	f = f/60;
	return f;
}

double micro_service::calculate_loss(double frequency_out){
	//placeholder	
	return 0;
}

double micro_service::determine_heat(complex power_out, double heat_loss){
	//placeholder;
	Heat_Out = power_out.Mag() * heat_loss;
	return Heat_Out;
}






/* Object initialization is called once after all object have been created */
int micro_service::init(OBJECT *parent)
{



    price = pmy_microturbine->microturbine_init();


	
	struct {
		complex **var;
		char *varname;
	} map[] = {
		// local object name,	meter object name
		{&pCircuit_V_A,			"phaseA_V_In"}, 
		{&pCircuit_V_B,			"phaseB_V_In"}, 
		{&pCircuit_V_C,			"phaseC_V_In"}, 
		{&pLine_I_A,			"phaseA_I_In"}, 
		{&pLine_I_B,			"phaseB_I_In"}, 
		{&pLine_I_C,			"phaseC_I_In"}, 
		/// @todo use triplex property mapping instead of assuming memory order for meter variables (residential, low priority) (ticket #139)
	};
	 

	

	static complex default_line_voltage[1], default_line_current[1];
	int i;

	// find parent meter, if not defined, use a default meter (using static variable 'default_meter')
	if (parent!=NULL && strcmp(parent->oclass->name,"meter")==0)
	{
		for (i=0; i<sizeof(map)/sizeof(map[0]); i++)
			*(map[i].var) = get_complex(parent,map[i].varname);

		gl_verbose("micro_service init: mapped METER objects to internal variables");
	}
	else if (parent!=NULL && strcmp(parent->oclass->name,"rectifier")==0){
		gl_verbose("micro_service init: parent WAS found, is an rectifier!");
			// construct circuit variable map to meter
			/// @todo use triplex property mapping instead of assuming memory order for meter variables (residential, low priority) (ticket #139)

		for (i=0; i<sizeof(map)/sizeof(map[0]); i++){
			*(map[i].var) = get_complex(parent,map[i].varname);
		}
		gl_verbose("micro_service init: mapped RECTIFIER objects to internal variables");
	}
	else{
		
			// construct circuit variable map to meter
		/// @todo use triplex property mapping instead of assuming memory order for meter variables (residential, low priority) (ticket #139)
		gl_verbose("micro_service init: mapped meter objects to internal variables");

		OBJECT *obj = OBJECTHDR(this);
		gl_verbose("micro_service init: no parent meter defined, parent is not a meter");
		gl_warning("micro_service:%d %s", obj->id, parent==NULL?"has no parent meter defined":"parent is not a meter");

		// attach meter variables to each circuit in the default_meter
			*(map[0].var) = &default_line_voltage[0];
			*(map[1].var) = &default_line_current[0];

		// provide initial values for voltages
			default_line_voltage[0] = complex(V_Max.Re()/sqrt(3.0),0);
			default_line_current[0] = complex(I_Max.Re());
			//default_line123_voltage[1] = complex(V_Max/sqrt(3.0)*cos(2*PI/3),V_Max/sqrt(3.0)*sin(2*PI/3));
			//default_line123_voltage[2] = complex(V_Max/sqrt(3.0)*cos(-2*PI/3),V_Max/sqrt(3.0)*sin(-2*PI/3));

	}

	gl_verbose("micro_service init: finished connecting with meter");




	/* TODO: set the context-dependent initial value of properties */
	//double ZB, SB, EB;
	//complex tst;
		if (gen_mode_v==UNKNOWN)
	{
		OBJECT *obj = OBJECTHDR(this);
		throw("Generator control mode is not specified");
	}
		if (gen_status_v==0)
	{
		//OBJECT *obj = OBJECTHDR(this);
		throw("Generator is out of service!");
	}else
		{
			return 1;

		}
	return 1; /* return 1 on success, 0 on failure */
}





complex *micro_service::get_complex(OBJECT *obj, char *name)
{
	PROPERTY *p = gl_get_property(obj,name);
	if (p==NULL || p->ptype!=PT_complex)
		return NULL;
	return (complex*)GETADDR(obj,p);
}




/* Presync is called when the clock needs to advance on the first top-down pass */
TIMESTAMP micro_service::presync(TIMESTAMP t0, TIMESTAMP t1)
{

	TIMESTAMP t2 = TS_NEVER;
	Heat_Out = Fuel_Used = frequency = 0.0;

	int microturbine_presyncResult = pmy_microturbine->microturbine_presync(t0, t1);
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

/* Sync is called when the clock needs to advance on the bottom-up pass */
TIMESTAMP micro_service::sync(TIMESTAMP t0, TIMESTAMP t1) 
{
	//gather V_Out for each phase
	//gather I_Out for each phase
	//gather VA_Out for each phase
	//gather Q_Out
	//gather S_Out
	//gather Pf_Out


	//CALL SYNC METHOD OF THE WEB SERVICE
	//return value in VA_Out.Re()
	//input parameters 
	
    double CircuitA_V_Out_re = 2;
    double CircuitA_V_Out_im = 2;
    double CircuitB_V_Out_re = 2;
    double CircuitB_V_Out_im = 2;
    double CircuitC_V_Out_re = 2;
    double CircuitC_V_Out_im = 2;
    double LineA_V_Out_re = 2;
    double LineA_V_Out_im = 2;
    double LineB_V_Out_re = 2;
    double LineB_V_Out_im = 2;
    double LineC_V_Out_re = 2;
    double LineC_V_Out_im = 2;
	printf("ads");
	double microturbine_syncResult = pmy_microturbine->microturbine_sync(CircuitA_V_Out_re, CircuitA_V_Out_im, CircuitB_V_Out_re, CircuitB_V_Out_im, CircuitC_V_Out_re, CircuitC_V_Out_im, LineA_V_Out_re, LineA_V_Out_im, LineB_V_Out_re, LineB_V_Out_im, LineC_V_Out_re, LineC_V_Out_im);

	

return TS_NEVER;
}

/* Postsync is called when the clock needs to advance on the second top-down pass */
TIMESTAMP micro_service::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;

	Heat_Out = Fuel_Used = frequency = 0.0;

	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE
//////////////////////////////////////////////////////////////////////////

EXPORT int create_micro_service(OBJECT **obj, OBJECT *parent) 
{
	try 
	{
		*obj = gl_create_object(micro_service::oclass);
		if (*obj!=NULL)
		{
			micro_service *my = OBJECTDATA(*obj,micro_service);
			gl_set_parent(*obj,parent);
			return my->create();
		}
		else
			return 0;
	} 
	CREATE_CATCHALL(micro_service);
}

EXPORT int init_micro_service(OBJECT *obj, OBJECT *parent) 
{
	try 
	{
		if (obj!=NULL)
			return OBJECTDATA(obj,micro_service)->init(parent);
		else
			return 0;
	}
	INIT_CATCHALL(micro_service);
}

EXPORT TIMESTAMP sync_micro_service(OBJECT *obj, TIMESTAMP t1, PASSCONFIG pass)
{
	TIMESTAMP t2 = TS_NEVER;
	micro_service *my = OBJECTDATA(obj,micro_service);
	try
	{
		switch (pass) {
		case PC_PRETOPDOWN:
			t2 = my->presync(obj->clock,t1);
			break;
		case PC_BOTTOMUP:
			t2 = my->sync(obj->clock,t1);
			break;
		case PC_POSTTOPDOWN:
			t2 = my->postsync(obj->clock,t1);
			break;
		default:
			GL_THROW("invalid pass request (%d)", pass);
			break;
		}
		if (pass==clockpass)
			obj->clock = t1;		
	}
	SYNC_CATCHALL(micro_service);
	return t2;
}
