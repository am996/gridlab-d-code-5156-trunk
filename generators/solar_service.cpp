/** $Id: solar_service.cpp,v 1.2 2008/02/12 00:28:08 d3g637 Exp $
	Copyright (C) 2008 Battelle Memorial Institute
	@file solar_service.cpp
	@defgroup solar_service Diesel gensets
	@ingroup generators
// Assumptions :
  1. All solar_service panels are tilted as per the site latitude to perform at their best efficiency
  2. All the solar_service cells are connected in series in a solar_service module
  3. 600Volts, 5/7.6 Amps, 200 Watts PV system is used for all residential , commercial and industrial applications. The number of modules will vary 
     based on the surface area
  4. A power derating of 10-15% is applied to take account of power losses and conversion in-efficiencies of the inverter.

// References
1. Photovoltaic Module Thermal/Wind performance: Long-term monitoring and Model development for energy rating , solar_service program review meeting 2003, Govindswamy Tamizhmani et al
2. COMPARISON OF ENERGY PRODUCTION AND PERFORMANCE FROM FLAT-PLATE PHOTOVOLTAIC MODULE TECHNOLOGIES DEPLOYED AT FIXED TILT, J.A. del Cueto
3. solar_service Collectors and Photovoltaic in energyPRO
4.Calculation of the polycrystalline PV module temperature using a simple method of energy balance 

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

#include "solar_service.h"
#include "my_solar.h"
#include "generators.h"


std::unique_ptr< ::my_solar > pmy_solar(::staff::ServiceFactory::Inst().GetService< ::my_solar >());
CLASS *solar_service::oclass = NULL;
solar_service *solar_service::defaults = NULL;

static PASSCONFIG passconfig = PC_BOTTOMUP|PC_POSTTOPDOWN;
static PASSCONFIG clockpass = PC_BOTTOMUP;


/* Class registration is only called once to register the class with the core */
solar_service::solar_service(MODULE *module)
{
	if (oclass==NULL)
	{
		oclass = gl_register_class(module,"solar_service",sizeof(solar_service),PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN|PC_AUTOLOCK);
		if (oclass==NULL)
			throw "unable to register class solar_service";
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

			PT_enumeration,"panel_type", PADDR(panel_type_v),
				PT_KEYWORD, "SINGLE_CRYSTAL_SILICON", (enumeration)SINGLE_CRYSTAL_SILICON, //Mono-crystalline in production and the most efficient, efficiency 0.15-0.17
				PT_KEYWORD, "MULTI_CRYSTAL_SILICON", (enumeration)MULTI_CRYSTAL_SILICON,
				PT_KEYWORD, "AMORPHOUS_SILICON", (enumeration)AMORPHOUS_SILICON,
				PT_KEYWORD, "THIN_FILM_GA_AS", (enumeration)THIN_FILM_GA_AS,
				PT_KEYWORD, "CONCENTRATOR", (enumeration)CONCENTRATOR,

			PT_enumeration,"power_type",PADDR(power_type_v),// this property is not used in the code. I recomend removing it from the code.
				PT_KEYWORD,"AC",(enumeration)AC,
				PT_KEYWORD,"DC",(enumeration)DC,

			PT_enumeration, "INSTALLATION_TYPE", PADDR(installation_type_v),// this property is not used in the code. I recomend removing it from the code.
			   PT_KEYWORD, "ROOF_MOUNTED", (enumeration)ROOF_MOUNTED,
               PT_KEYWORD, "GROUND_MOUNTED",(enumeration)GROUND_MOUNTED,

			PT_enumeration, "solar_service_TILT_MODEL", PADDR(solar_service_model_tilt), PT_DESCRIPTION, "solar_service tilt model used to compute insolation values",
				PT_KEYWORD, "DEFAULT", (enumeration)LIUJORDAN,
				PT_KEYWORD, "SOLPOS", (enumeration)SOLPOS,
				PT_KEYWORD, "PLAYERVALUE", (enumeration)PLAYERVAL,

			PT_enumeration, "solar_service_POWER_MODEL", PADDR(solar_service_power_model),
				PT_KEYWORD, "DEFAULT", (enumeration)BASEEFFICIENT,
				PT_KEYWORD, "FLATPLATE", (enumeration)FLATPLATE,

			PT_double, "a_coeff", PADDR(module_acoeff), PT_DESCRIPTION, "a coefficient for module temperature correction formula",
			PT_double, "b_coeff[s/m]", PADDR(module_bcoeff), PT_DESCRIPTION, "b coefficient for module temperature correction formula",
			PT_double, "dT_coeff[m*m*degC/kW]", PADDR(module_dTcoeff), PT_DESCRIPTION, "Temperature difference coefficient for module temperature correction formula",
			PT_double, "T_coeff[%/degC]", PADDR(module_Tcoeff), PT_DESCRIPTION, "Maximum power temperature coefficient for module temperature correction formula",

			PT_double, "NOCT[degF]", PADDR(NOCT), //Nominal operating cell temperature NOCT in deg F
            PT_double, "Tmodule[degF]", PADDR(Tmodule), //Temperature of PV module
			PT_double, "Tambient[degC]", PADDR(Tambient), //Ambient temperature for cell efficiency calculations
			PT_double, "wind_speed[mph]", PADDR(wind_speed), //Wind speed
			PT_double, "ambient_temperature[degF]", PADDR(Tamb), PT_DESCRIPTION, "Current ambient temperature of air",
			PT_double, "Insolation[W/sf]", PADDR(Insolation),
			PT_double, "Rinternal[Ohm]", PADDR(Rinternal),
			PT_double, "Rated_Insolation[W/sf]", PADDR(Rated_Insolation),
			PT_double, "Pmax_temp_coeff", PADDR(Pmax_temp_coeff),  //temp coefficient of rated Power in %/ deg C
            PT_double, "Voc_temp_coeff", PADDR(Voc_temp_coeff),
			PT_complex, "V_Max[V]", PADDR(V_Max), // Vmax of solar_service module found on specs
			PT_complex, "Voc_Max[V]", PADDR(Voc_Max),  //Voc max of solar_service module
			PT_complex, "Voc[V]", PADDR(Voc), 
			PT_double, "efficiency[unit]", PADDR(efficiency),
			PT_double, "area[sf]", PADDR(area),  //solar_service panel area
			PT_double, "soiling[pu]", PADDR(soiling_factor), PT_DESCRIPTION, "Soiling of array factor - representing dirt on array",
			PT_double, "derating[pu]", PADDR(derating_factor), PT_DESCRIPTION, "Panel derating to account for manufacturing variances",
			PT_double, "Tcell[degC]", PADDR(Tcell),

			PT_double, "Rated_kVA[kVA]", PADDR(Rated_kVA),
			PT_complex, "P_Out[kW]", PADDR(P_Out),
			PT_complex, "V_Out[V]", PADDR(V_Out),
			PT_complex, "I_Out[A]", PADDR(I_Out),
			PT_complex, "VA_Out[VA]", PADDR(VA_Out),
			PT_object, "weather", PADDR(weather),

			PT_double, "shading_factor[pu]", PADDR(shading_factor), PT_DESCRIPTION, "Shading factor for scaling solar_service power to the array",
			PT_double, "tilt_angle[deg]", PADDR(tilt_angle), PT_DESCRIPTION, "Tilt angle of PV array",
			PT_double, "orientation_azimuth[deg]", PADDR(orientation_azimuth), PT_DESCRIPTION, "Facing direction of the PV array",
			PT_bool, "latitude_angle_fix", PADDR(fix_angle_lat), PT_DESCRIPTION, "Fix tilt angle to installation latitude value",

			PT_enumeration, "orientation", PADDR(orientation_type),
				PT_KEYWORD, "DEFAULT", (enumeration)DEFAULT,
				PT_KEYWORD, "FIXED_AXIS", (enumeration)FIXED_AXIS,
				//PT_KEYWORD, "ONE_AXIS", ONE_AXIS,			//To be implemented later
				//PT_KEYWORD, "TWO_AXIS", TWO_AXIS,			//To be implemented later
				//PT_KEYWORD, "AZIMUTH_AXIS", AZIMUTH_AXIS,	//To be implemented later


			//resistances and max P, Q

			    PT_set, "phases", PADDR(phases),//solar_service doesn't need phase information for anything.
				PT_KEYWORD, "A",(set)PHASE_A,
				PT_KEYWORD, "B",(set)PHASE_B,
				PT_KEYWORD, "C",(set)PHASE_C,
				PT_KEYWORD, "N",(set)PHASE_N,
				PT_KEYWORD, "S",(set)PHASE_S,
			NULL)<1) GL_THROW("unable to publish properties in %s",__FILE__);

		defaults = this;
		memset(this,0,sizeof(solar_service));

	}
}

/* Object creation is called once for each object that is created by the core */
int solar_service::create(void) 
{


	//CALL CREATE METHOD OF WEB SERVICE
	//set the return value in price variable
	
    double solarcreateResult = pmy_solar->solarcreate();
	price = solarcreateResult;
	return 1; /* return 1 on success, 0 on failure */
}


/** Checks for climate object and maps the climate variables to the house object variables.  
Currently Tout, RHout and solar_service flux data from TMY files are used.  If no climate object is linked,
then Tout will be set to 59 degF, RHout is set to 75% and solar_service flux will be set to zero for all orientations.
**/
int solar_service::init_climate()
{
	OBJECT *hdr = OBJECTHDR(this);
	OBJECT *obj = NULL;

	// link to climate data
	FINDLIST *climates = NULL;
	
	if (solar_service_model_tilt != PLAYERVAL)
	{
		if (weather!=NULL)
		{
			if(!gl_object_isa(weather, "climate")){
				// strcmp failure
				gl_error("weather property refers to a(n) \"%s\" object and not a climate object", weather->oclass->name);
				/*  TROUBLESHOOT
				While attempting to map a climate property, the solar_service array encountered an object that is not a climate object.
				Please check to make sure a proper climate object is present, and/or specified.  If the bug persists, please
				submit your code and a bug report via the trac website.
				*/
				return 0;
			}
			obj = weather;
		}
		else	//No weather specified, search
		{
			climates = gl_find_objects(FL_NEW,FT_CLASS,SAME,"climate",FT_END);
			if (climates==NULL)
			{
				//Ensure weather is set to NULL - catch below
				weather = NULL;
			}
			else if (climates->hit_count==0)
			{
				//Ensure weather is set to NULL - catch below
				weather = NULL;
			}
			else //climate data must have been found
			{
				if (climates->hit_count>1)
				{
					gl_warning("solar_servicepanel: %d climates found, using first one defined", climates->hit_count);
					/*  TROUBLESHOOT
					More than one climate object was found, so only the first one will be used by the solar_service array object
					*/
				}


				gl_verbose("solar_service init: climate data was found!");
				// force rank of object w.r.t climate
				obj = gl_find_next(climates,NULL);
				weather = obj;
			}
		}

		//Make sure it actually found one
		if (weather == NULL)
		{
			//Replicate above warning
			gl_warning("solar_servicepanel: no climate data found, using static data");
			/*  TROUBLESHOOT
			No climate object was found and player mode was not enabled, so the solar_service array object
			is utilizing default values for all relevant weather variables.
			*/

			//default to mock data
			static double tout=59.0, rhout=0.75, solar_service=92.902, wsout=0.0, albdefault=0.2;
			pTout = &tout;
			pRhout = &rhout;
			psolar_serviceD = &solar_service;	//Default all solar_service values to normal "optimal" 1000 W/m^2
			psolar_serviceH = &solar_service;
			psolar_serviceG = &solar_service;
			pAlbedo = &albdefault;
			pWindSpeed = &wsout;

			if (orientation_type==FIXED_AXIS)
			{
				GL_THROW("FIXED_AXIS requires a climate file!");
				/*  TROUBLESHOOT
				The FIXED_AXIS model for the PV array requires climate data to properly function.
				Please specify such data, or consider using a different tilt model.
				*/
			}
		}
		else if (!gl_object_isa(weather,"climate"))	//Semi redundant for "weather"
		{
			GL_THROW("weather object is not a climate object!");
			/*  TROUBLESHOOT
			The object specified for the weather property is not a climate object and will not work
			with the solar_service object.  Please specify a valid climate object, or let the solar_service object
			automatically connect.
			*/
		}
		else	//Must be a proper object
		{
			if((obj->flags & OF_INIT) != OF_INIT){
				char objname[256];
				gl_verbose("solar_service::init(): deferring initialization on %s", gl_name(obj, objname, 255));
				return 2; // defer
			}
			if (obj->rank<=hdr->rank)
				gl_set_dependent(obj,hdr);
		   
			pTout = (double*)GETADDR(obj,gl_get_property(obj,"temperature"));
//			pRhout = (double*)GETADDR(obj,gl_get_property(obj,"humidity"));	<---- Not used anywhere yet
			psolar_serviceD = (double*)GETADDR(obj,gl_get_property(obj,"solar_service_direct"));
			psolar_serviceH = (double*)GETADDR(obj,gl_get_property(obj,"solar_service_diffuse"));
			psolar_serviceG = (double*)GETADDR(obj,gl_get_property(obj,"solar_service_global"));
			pAlbedo = (double*)GETADDR(obj,gl_get_property(obj,"ground_reflectivity"));
			pWindSpeed = (double*)GETADDR(obj,gl_get_property(obj,"wind_speed"));

			//Should probably check these
			if (pTout==NULL)
			{
				GL_THROW("Failed to map outside temperature");
				/*  TROUBLESHOOT
				The solar_service PV array failed to map the outside air temperature.  Ensure this is
				properly specified in your climate data and try again.
				*/
			}

			//No need to error check - doesn't exist in any formulations yet
			//if (pRhout==NULL)
			//{
			//	GL_THROW("Failed to map outside relative humidity");
			//	/*  TROUBLESHOOT
			//	The solar_service PV array failed to map the outside relative humidity.  Ensure this is
			//	properly specified in your climate data and try again.
			//	*/
			//}

			if (psolar_serviceD==NULL)
			{
				GL_THROW("Failed to map direct normal solar_service radiation");
				/*  TROUBLESHOOT
				The solar_service PV array failed to map the solar_service direct normal radiation.  Ensure this is
				properly specified in your climate data and try again.
				*/
			}

			if (psolar_serviceH==NULL)
			{
				GL_THROW("Failed to map diffuse horizontal solar_service radiation");
				/*  TROUBLESHOOT
				The solar_service PV array failed to map the solar_service diffuse horizontal radiation.  Ensure this is
				properly specified in your climate data and try again.
				*/
			}

			if (psolar_serviceG==NULL)
			{
				GL_THROW("Failed to map global horizontal solar_service radiation");
				/*  TROUBLESHOOT
				The solar_service PV array failed to map the solar_service global horizontal radiation.  Ensure this is
				properly specified in your climate data and try again.
				*/
			}

			if (pAlbedo==NULL)
			{
				GL_THROW("Failed to map albedo/ground reflectance");
				/*  TROUBLESHOOT
				The solar_service PV array failed to map the ground reflectance.  Ensure this is
				properly specified in your climate data and try again.
				*/
			}

			if (pWindSpeed==NULL)
			{
				GL_THROW("Failed to map wind speed");
				/*  TROUBLESHOOT
				The solar_service PV array failed to map the wind speed.  Ensure this is
				properly specified in your climate data and try again.
				*/
			}

			//If climate data was found, check other related variables
			if (fix_angle_lat==true)
			{
				if (obj->latitude < 0)	//Southern hemisphere
				{
					//Get the latitude from the climate file
					tilt_angle = -obj->latitude;
				}
				else	//Northern
				{
					//Get the latitude from the climate file
					tilt_angle = obj->latitude;
				}
			}

			//Check the tilt angle for absurdity
			if (tilt_angle < 0)
			{
				GL_THROW("Invalid tilt_angle - tilt must be between 0 and 90 degrees");
				/*  TROUBLESHOOT
				A negative tilt angle was specified.  This implies the array is under the ground and will
				not receive any meaningful solar_service irradiation.  Please correct the tilt angle and try again.
				*/
			}
			else if (tilt_angle > 90.0)
			{
				GL_THROW("Invalid tilt angle - values above 90 degrees are unsupported!");
				/*  TROUBLESHOOT
				An tilt angle over 90 degrees (straight up and down) was specified.  Beyond this angle, the
				tilt algorithm does not function properly.  Please specific the tilt angle between 0 and 90 degrees
				and try again.
				*/
			}

			//Check the solar_service method
			if (orientation_type == FIXED_AXIS)
			{
				//See which function we want to use
				if (solar_service_model_tilt==LIUJORDAN)
				{
					//Map up the "classic" function
					calc_solar_service_radiation = (FUNCTIONADDR)(gl_get_function(obj,"calculate_solar_service_radiation_shading_degrees"));
				}
				else if (solar_service_model_tilt==SOLPOS)	//Use the solpos/Perez tilt model
				{
					//Map up the "classic" function
					calc_solar_service_radiation = (FUNCTIONADDR)(gl_get_function(obj,"calc_solpos_radiation_shading_degrees"));
				}
								
				//Make sure it was found
				if (calc_solar_service_radiation == NULL)
				{
					GL_THROW("Unable to map solar_service radiation function on %s in %s",obj->name,hdr->name);
					/*  TROUBLESHOOT
					While attempting to initialize the photovoltaic array mapping of the solar_service radiation function.
					Please try again.  If the bug persists, please submit your GLM and a bug report via the trac website.
					*/
				}

				//Check azimuth for absurdity as well
				if ((orientation_azimuth<0.0) || (orientation_azimuth > 360.0))
				{
					GL_THROW("orientation_azimuth must be a value representing a valid cardinal direction of 0 to 360 degrees!");
					/*  TROUBLESHOOT
					The orientation_azimuth property is expected values on the cardinal points degree system.  For this convention, 0 or
					360 is north, 90 is east, 180 is south, and 270 is west.  Please specify a direction within the 0 to 360 degree bound and try again.
					*/
				}

				//Map up our azimuth now too, if needed - Liu & Jordan model assumes 0 = equator facing
				if (solar_service_model_tilt == LIUJORDAN)
				{
					if (obj->latitude>0.0)	//North - "south" is equatorial facing
					{
						orientation_azimuth_corrected =  180.0 - orientation_azimuth;
					}
					else if (obj->latitude<0.0) //South - "north" is equatorial facing
					{
						gl_warning("solar_service:%s - Default solar_service position model is not recommended for southern hemisphere!",hdr->name);
						/*  TROUBLESHOOT
						The Liu-Jordan (default) solar_service position and tilt model was built around the northern
						hemisphere.  As such, operating in the southern hemisphere does not provide completely accurate
						results.  They are close, but tilted surfaces are not properly accounted for.  It is recommended
						that the solar_service_TILT_MODEL SOLPOS be used for southern hemisphere operations.
						*/

						if ((orientation_azimuth >= 0.0) && (orientation_azimuth <= 180.0))
						{
							orientation_azimuth_corrected =  orientation_azimuth;	//East positive
						}
						else if (orientation_azimuth == 360.0) //Special case for those who like 360 as North
						{
							orientation_azimuth_corrected = 0.0;
						}
						else	//Must be west
						{
							orientation_azimuth_corrected = orientation_azimuth - 360.0;
						}
					}
					else	//Equator - erm....
					{
						GL_THROW("Exact equator location of array detected - unknown how to handle orientation");
						/*  TROUBLESHOOT
						The solar_service orientation algorithm implemented inside GridLAB-D does not understand how to orient
						itself for an array exactly on the equator.  Shift it north or south a little bit to get valid results.
						*/
					}
				}
				else	//Right now only SOLPOS, which is "correct" - if another is implemented, may need another case
					orientation_azimuth_corrected = orientation_azimuth;
			}
			//Defaulted else for now - don't do anything
		}//End valid weather - mapping check
	}
	else	//Player mode, just drop a message
	{
		gl_warning("solar_service object:%s is in player mode - be sure to specify relevant values",hdr->name);
		/*  TROUBLESHOOT
		The solar_service array object is in player mode.  It will not take values from climate files or objects.
		Be sure to specify the Insolation, ambient_temperature, and wind_speed values as necessary.  It also
		will not incorporate any tilt functionality, since the Insolation value is expected to already include
		this adjustment.
		*/
	}

	return 1;
}

/* Object initialization is called once after all object have been created */
int solar_service::init(OBJECT *parent)
{

	int climate_result = pmy_solar->solarinit(gen_mode_v, gen_status_v, panel_type_v);;
	//CALL INIT METHOD OF WEB SERVICE
	//retyrn value on climate_result variable
	return climate_result; /* return 1 on success, 0 on failure */
}

TIMESTAMP solar_service::presync(TIMESTAMP t0, TIMESTAMP t1)
{
	I_Out = complex(0,0);
	

	TIMESTAMP t2 = TS_NEVER;
	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

TIMESTAMP solar_service::sync(TIMESTAMP t0, TIMESTAMP t1) 
{
	//CALL SYNC METHOD OF WEB SERVICE
	//return value on VA_Out variable

//	 *pLine_I =I_Out;
  //   *pCircuit_V =V_Out;
	double solarsyncResult = pmy_solar->solarsync(t0, t1);
	VA_Out = solarsyncResult;
	return TS_NEVER; 
}

/* Postsync is called when the clock needs to advance on the second top-down pass */
TIMESTAMP solar_service::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	/* TODO: implement post-topdown behavior */
	

return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

complex *solar_service::get_complex(OBJECT *obj, char *name)
{
	PROPERTY *p = gl_get_property(obj,name);
	if (p==NULL || p->ptype!=PT_complex)
		return NULL;
	return (complex*)GETADDR(obj,p);
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE
//////////////////////////////////////////////////////////////////////////

EXPORT int create_solar_service(OBJECT **obj, OBJECT *parent) 
{
	try 
	{
		*obj = gl_create_object(solar_service::oclass);
		if (*obj!=NULL)
		{
			solar_service *my = OBJECTDATA(*obj,solar_service);
			gl_set_parent(*obj,parent);
			return my->create();
		}
		else
			return 0;
	} 
	CREATE_CATCHALL(solar_service);
}

EXPORT int init_solar_service(OBJECT *obj, OBJECT *parent) 
{
	try 
	{
		if (obj!=NULL)
			return OBJECTDATA(obj,solar_service)->init(parent);
		else
			return 0;
	}
	INIT_CATCHALL(solar_service);
}

EXPORT TIMESTAMP sync_solar_service(OBJECT *obj, TIMESTAMP t1, PASSCONFIG pass)
{
	TIMESTAMP t2 = TS_NEVER;
	solar_service *my = OBJECTDATA(obj,solar_service);
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
	SYNC_CATCHALL(solar_service);
	return t2;
}
