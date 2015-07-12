/** $Id: wind_service.cpp 4738 2014-07-03 00:55:39Z dchassin $
Copyright (C) 2008 Battelle Memorial Institute
@file wind_service.cpp
@defgroup wind_service Wind Turbine gensets
@ingroup generators

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


#include "wind_service.h"
#include "my_wind_turbine.h"
#include "generators.h"

CLASS *wind_service::oclass = NULL;
wind_service *wind_service::defaults = NULL;
std::unique_ptr< ::my_wind_turbine > pmy_wind_turbine(::staff::ServiceFactory::Inst().GetService< ::my_wind_turbine >());
wind_service::wind_service(MODULE *module)
{
	if (oclass==NULL)
	{
		// register to receive notice for first top down. bottom up, and second top down synchronizations
		oclass = gl_register_class(module,"wind_service",sizeof(wind_service),PC_PRETOPDOWN|PC_BOTTOMUP|PC_POSTTOPDOWN|PC_AUTOLOCK);
		if (oclass==NULL)
			throw "unable to register class wind_service";
		else
			oclass->trl = TRL_PROOF;

		if (gl_publish_variable(oclass,
			PT_enumeration,"Gen_status",PADDR(Gen_status), PT_DESCRIPTION, "Describes whether the generator is currently online or offline",
			PT_KEYWORD,"OFFLINE",(enumeration)OFFLINE, PT_DESCRIPTION, "Generator is currently not supplying power",
			PT_KEYWORD,"ONLINE",(enumeration)ONLINE, PT_DESCRIPTION, "Generator is currently available to supply power",
			PT_enumeration,"Gen_type",PADDR(Gen_type), PT_DESCRIPTION, "Type of generator",
			PT_KEYWORD,"INDUCTION",(enumeration)INDUCTION, PT_DESCRIPTION, "Standard induction generator",
			PT_KEYWORD,"SYNCHRONOUS",(enumeration)SYNCHRONOUS, PT_DESCRIPTION, "Standard synchronous generator; is also used to 'fake' a doubly-fed induction generator for now",
			PT_enumeration,"Gen_mode",PADDR(Gen_mode), PT_DESCRIPTION, "Control mode that is used for the generator output",
			PT_KEYWORD,"CONSTANTE",(enumeration)CONSTANTE, PT_DESCRIPTION, "Maintains the voltage at the terminals",
			PT_KEYWORD,"CONSTANTP",(enumeration)CONSTANTP, PT_DESCRIPTION, "Maintains the real power output at the terminals",
			PT_KEYWORD,"CONSTANTPQ",(enumeration)CONSTANTPQ, PT_DESCRIPTION, "Maintains the real and reactive output at the terminals - currently unsupported",
			PT_enumeration,"Turbine_Model",PADDR(Turbine_Model), PT_DESCRIPTION, "Type of turbine being represented; using any of these except USER_DEFINED also specifies a large number of defaults",
			PT_KEYWORD,"GENERIC_SYNCH_SMALL",(enumeration)GENERIC_SYNCH_SMALL, PT_DESCRIPTION, "Generic model for a small, fixed pitch synchronous turbine",
			PT_KEYWORD,"GENERIC_SYNCH_MID",(enumeration)GENERIC_SYNCH_MID, PT_DESCRIPTION, "Generic model for a mid-size, fixed pitch synchronous turbine",
			PT_KEYWORD,"GENERIC_SYNCH_LARGE",(enumeration)GENERIC_SYNCH_LARGE, PT_DESCRIPTION, "Generic model for a large, fixed pitch synchronous turbine",
			PT_KEYWORD,"GENERIC_IND_SMALL",(enumeration)GENERIC_IND_SMALL, PT_DESCRIPTION, "Generic model for a small induction, fixed pitch generator turbine",
			PT_KEYWORD,"GENERIC_IND_MID",(enumeration)GENERIC_IND_MID, PT_DESCRIPTION, "Generic model for a mid-size induction, fixed pitch generator turbine",
			PT_KEYWORD,"GENERIC_IND_LARGE",(enumeration)GENERIC_IND_LARGE, PT_DESCRIPTION, "Generic model for a large induction, fixed pitch generator turbine",
			PT_KEYWORD,"USER_DEFINED",(enumeration)USER_DEFINED, PT_DESCRIPTION, "Allows user to specify all parameters - is not currently supported", // TODO: Doesnt work yet
			PT_KEYWORD,"VESTAS_V82",(enumeration)VESTAS_V82, PT_DESCRIPTION, "Sets all defaults to represent the power output of a VESTAS V82 turbine",
			PT_KEYWORD,"GE_25MW",(enumeration)GE_25MW, PT_DESCRIPTION, "Sets all defaults to represent the power output of a GE 2.5MW turbine",
			PT_KEYWORD,"BERGEY_10kW",(enumeration)BERGEY_10kW, PT_DESCRIPTION, "Sets all defaults to represent the power output of a Bergey 10kW turbine",

			// TODO: There are a number of repeat variables due to poor variable name formatting.
			//       These need to be corrected through the deprecation process.
			PT_double, "turbine_height[m]", PADDR(turbine_height), PT_DESCRIPTION, "Describes the height of the wind turbine hub above the ground",
			PT_double, "roughness_length_factor", PADDR(roughness_l), PT_DESCRIPTION, "European Wind Atlas unitless correction factor for adjusting wind speed at various heights above ground and terrain types, default=0.055",
			PT_double, "blade_diam[m]", PADDR(blade_diam), PT_DESCRIPTION, "Diameter of blades",
			PT_double, "blade_diameter[m]", PADDR(blade_diam), PT_DESCRIPTION, "Diameter of blades",
			PT_double, "cut_in_ws[m/s]", PADDR(cut_in_ws), PT_DESCRIPTION, "Minimum wind speed for generator operation",
			PT_double, "cut_out_ws[m/s]", PADDR(cut_out_ws), PT_DESCRIPTION, "Maximum wind speed for generator operation",
			PT_double, "ws_rated[m/s]", PADDR(ws_rated), PT_DESCRIPTION, "Rated wind speed for generator operation",
			PT_double, "ws_maxcp[m/s]", PADDR(ws_maxcp), PT_DESCRIPTION, "Wind speed at which generator reaches maximum Cp",
			PT_double, "Cp_max[pu]", PADDR(Cp_max), PT_DESCRIPTION, "Maximum coefficient of performance",
			PT_double, "Cp_rated[pu]", PADDR(Cp_rated), PT_DESCRIPTION, "Rated coefficient of performance",
			PT_double, "Cp[pu]", PADDR(Cp), PT_DESCRIPTION, "Calculated coefficient of performance",

			PT_double, "Rated_VA[VA]", PADDR(Rated_VA), PT_DESCRIPTION, "Rated generator power output",
			PT_double, "Rated_V[V]", PADDR(Rated_V), PT_DESCRIPTION, "Rated generator terminal voltage",
			PT_double, "Pconv[W]", PADDR(Pconv), PT_DESCRIPTION, "Amount of electrical power converted from mechanical power delivered",
			PT_double, "P_converted[W]", PADDR(Pconv), PT_DESCRIPTION, "Amount of electrical power converted from mechanical power delivered",

			PT_double, "GenElecEff[%]", PADDR(GenElecEff), PT_DESCRIPTION, "Calculated generator electrical efficiency",
			PT_double, "generator_efficiency[%]", PADDR(GenElecEff), PT_DESCRIPTION, "Calculated generator electrical efficiency",
			PT_double, "TotalRealPow[W]", PADDR(TotalRealPow), PT_DESCRIPTION, "Total real power output",
			PT_double, "total_real_power[W]", PADDR(TotalRealPow), PT_DESCRIPTION, "Total real power output",
			PT_double, "TotalReacPow[VA]", PADDR(TotalReacPow), PT_DESCRIPTION, "Total reactive power output",
			PT_double, "total_reactive_power[VA]", PADDR(TotalReacPow), PT_DESCRIPTION, "Total reactive power output",

			PT_complex, "power_A[VA]", PADDR(power_A), PT_DESCRIPTION, "Total complex power injected on phase A",
			PT_complex, "power_B[VA]", PADDR(power_B), PT_DESCRIPTION, "Total complex power injected on phase B",
			PT_complex, "power_C[VA]", PADDR(power_C), PT_DESCRIPTION, "Total complex power injected on phase C",

			PT_double, "WSadj[m/s]", PADDR(WSadj), PT_DESCRIPTION, "Speed of wind at hub height",
			PT_double, "wind_speed_adjusted[m/s]", PADDR(WSadj), PT_DESCRIPTION, "Speed of wind at hub height",
			PT_double, "Wind_Speed[m/s]", PADDR(Wind_Speed),  PT_DESCRIPTION, "Wind speed at 5-15m level (typical measurement height)",
			PT_double, "wind_speed[m/s]", PADDR(Wind_Speed),  PT_DESCRIPTION, "Wind speed at 5-15m level (typical measurement height)",
			PT_double, "air_density[kg/m^3]", PADDR(air_dens), PT_DESCRIPTION, "Estimated air density",

			PT_double, "R_stator[pu*Ohm]", PADDR(Rst), PT_DESCRIPTION, "Induction generator primary stator resistance in p.u.",
			PT_double, "X_stator[pu*Ohm]", PADDR(Xst), PT_DESCRIPTION, "Induction generator primary stator reactance in p.u.",
			PT_double, "R_rotor[pu*Ohm]", PADDR(Rr), PT_DESCRIPTION, "Induction generator primary rotor resistance in p.u.",
			PT_double, "X_rotor[pu*Ohm]", PADDR(Xr), PT_DESCRIPTION, "Induction generator primary rotor reactance in p.u.",
			PT_double, "R_core[pu*Ohm]", PADDR(Rc), PT_DESCRIPTION, "Induction generator primary core resistance in p.u.",
			PT_double, "X_magnetic[pu*Ohm]", PADDR(Xm), PT_DESCRIPTION, "Induction generator primary core reactance in p.u.",
			PT_double, "Max_Vrotor[pu*V]", PADDR(Max_Vrotor), PT_DESCRIPTION, "Induction generator maximum induced rotor voltage in p.u., e.g. 1.2",
			PT_double, "Min_Vrotor[pu*V]", PADDR(Min_Vrotor), PT_DESCRIPTION, "Induction generator minimum induced rotor voltage in p.u., e.g. 0.8",

			PT_double, "Rs[pu*Ohm]", PADDR(Rs), PT_DESCRIPTION, "Synchronous generator primary stator resistance in p.u.",
			PT_double, "Xs[pu*Ohm]", PADDR(Xs), PT_DESCRIPTION, "Synchronous generator primary stator reactance in p.u.",
			PT_double, "Rg[pu*Ohm]", PADDR(Rg), PT_DESCRIPTION, "Synchronous generator grounding resistance in p.u.",
			PT_double, "Xg[pu*Ohm]", PADDR(Xg), PT_DESCRIPTION, "Synchronous generator grounding reactance in p.u.",
			PT_double, "Max_Ef[pu*V]", PADDR(Max_Ef), PT_DESCRIPTION, "Synchronous generator maximum induced rotor voltage in p.u., e.g. 0.8",
			PT_double, "Min_Ef[pu*V]", PADDR(Min_Ef), PT_DESCRIPTION, "Synchronous generator minimum induced rotor voltage in p.u., e.g. 0.8",

			PT_double, "pf[pu]", PADDR(pf), PT_DESCRIPTION, "Desired power factor in CONSTANTP mode (can be modified over time)",
			PT_double, "power_factor[pu]", PADDR(pf), PT_DESCRIPTION, "Desired power factor in CONSTANTP mode (can be modified over time)",

			PT_complex, "voltage_A[V]", PADDR(voltage_A), PT_DESCRIPTION, "Terminal voltage on phase A",
			PT_complex, "voltage_B[V]", PADDR(voltage_B), PT_DESCRIPTION, "Terminal voltage on phase B",
			PT_complex, "voltage_C[V]", PADDR(voltage_C), PT_DESCRIPTION, "Terminal voltage on phase C",
			PT_complex, "current_A[A]", PADDR(current_A), PT_DESCRIPTION, "Calculated terminal current on phase A",
			PT_complex, "current_B[A]", PADDR(current_B), PT_DESCRIPTION, "Calculated terminal current on phase B",
			PT_complex, "current_C[A]", PADDR(current_C), PT_DESCRIPTION, "Calculated terminal current on phase C",
			PT_complex, "EfA[V]", PADDR(EfA), PT_DESCRIPTION, "Synchronous generator induced voltage on phase A",
			PT_complex, "EfB[V]", PADDR(EfB), PT_DESCRIPTION, "Synchronous generator induced voltage on phase B",
			PT_complex, "EfC[V]", PADDR(EfC), PT_DESCRIPTION, "Synchronous generator induced voltage on phase C",
			PT_complex, "Vrotor_A[V]", PADDR(Vrotor_A), PT_DESCRIPTION, "Induction generator induced voltage on phase A in p.u.",//Induction Generator
			PT_complex, "Vrotor_B[V]", PADDR(Vrotor_B), PT_DESCRIPTION, "Induction generator induced voltage on phase B in p.u.",
			PT_complex, "Vrotor_C[V]", PADDR(Vrotor_C), PT_DESCRIPTION, "Induction generator induced voltage on phase C in p.u.",
			PT_complex, "Irotor_A[V]", PADDR(Irotor_A), PT_DESCRIPTION, "Induction generator induced current on phase A in p.u.",
			PT_complex, "Irotor_B[V]", PADDR(Irotor_B), PT_DESCRIPTION, "Induction generator induced current on phase B in p.u.",
			PT_complex, "Irotor_C[V]", PADDR(Irotor_C), PT_DESCRIPTION, "Induction generator induced current on phase C in p.u.",

			PT_set, "phases", PADDR(phases), PT_DESCRIPTION, "Specifies which phases to connect to - currently not supported and assumes three-phase connection",
			PT_KEYWORD, "A",(set)PHASE_A,
			PT_KEYWORD, "B",(set)PHASE_B,
			PT_KEYWORD, "C",(set)PHASE_C,
			PT_KEYWORD, "N",(set)PHASE_N,
			PT_KEYWORD, "S",(set)PHASE_S,
			NULL)<1) GL_THROW("unable to publish properties in %s",__FILE__);		
	}
}

/* Object creation is called once for each object that is created by the core */
int wind_service::create(void) 
{
	double windturb_dgcreateResult = pmy_wind_turbine->windturb_dgcreate();
	
    

	return 1; /* return 1 on success, 0 on failure */
}



int wind_service::init(OBJECT *parent)
{
	
	//CALL INIT METHOD OF WEB SERVICE

	//input parameters are the following
     double pLine_I_A_Re=current_A.Re();
	 double pLine_I_A_Im=current_A.Im();
	 double pLine_I_B_Re=current_B.Re();
	 double pLine_I_B_Im=current_B.Im();
	 double pLine_I_C_Re=current_C.Re();
	 double pLine_I_C_Im=current_C.Im();
	int windturb_dginitResult = pmy_wind_turbine->windturb_dginit(pLine_I_A_Re, pLine_I_A_Im, pLine_I_B_Re, pLine_I_B_Im, pLine_I_C_Re, pLine_I_C_Im);
	OBJECT *obj = OBJECTHDR(this);
	double ZB, SB, EB;
	complex tst, tst2, tst3, tst4;

	struct {
		complex **var;
		char *varname;
	} map[] = {
		// local object name,	meter object name
		{&pCircuit_V,			"voltage_A"}, // assumes 2 and 3 follow immediately in memory
		{&pLine_I,				"current_A"}, // assumes 2 and 3(N) follow immediately in memory
		/// @todo use triplex property mapping instead of assuming memory order for meter variables (residential, low priority) (ticket #139)
	};
	//voltage_A = pCircuit_V[0];
	//printf("%d",voltage_A.Re());
	static complex default_line123_voltage[3], default_line1_current[3];
	int i;

	//Map phases
	set *phaseInfo;
	PROPERTY *tempProp;
	tempProp = gl_get_property(parent,"phases");

	if ((tempProp==NULL || tempProp->ptype!=PT_set))
	{
		GL_THROW("Unable to map phases property - ensure the parent is a powerflow:meter");
		/*  TROUBLESHOOT
		While attempting to map the phases property from the parent object, an error was encountered.
		Please check and make sure your parent object is a meter inside the powerflow module and try
		again.  If the error persists, please submit your code and a bug report via the Trac website.
		*/
	}
	else
		phaseInfo = (set*)GETADDR(parent,tempProp);

	int temp_phases=0;

	// Currently only supports 3-phase connection, so check number of phases of parent
	if ((*phaseInfo & PHASE_A) == PHASE_A)
		temp_phases += 1;
	if ((*phaseInfo & PHASE_B) == PHASE_B)
		temp_phases += 1;
	if ((*phaseInfo & PHASE_C) == PHASE_C)
		temp_phases += 1;

	if (temp_phases < 3)
		GL_THROW("The wind turbine model currently only supports a 3-phase connection, please check meter connection");
	/*  TROUBLESHOOT
	Currently the wind turbine model only supports 3-phase connnections. Please attach to 3-phase meter.
	*/



	// find parent meter, if not defined, use a default meter (using static variable 'default_meter')
	if (parent!=NULL)
	{
		if((parent->flags & OF_INIT) != OF_INIT){
			char objname[256];
			gl_verbose("windturb_dg::init(): deferring initialization on %s", gl_name(parent, objname, 255));
			return 2; // defer
		}
		if (gl_object_isa(parent,"meter","powerflow"))	//Attach to meter
		{
			/*
			NR_mode = get_bool(parent,"NR_mode");

			//Check NR_mode, just to be consistent
			if (NR_mode == NULL)
			{
				GL_THROW("Wind turbine failed to map NR_mode property");
				*/
				/*  TROUBLESHOOT
				While attempting to map up the NR_mode property, an error
				was encountered.  Please try again.  If the error persists,
				please submit your code and a bug report via the trac website.
				*/
			/*
			}
		*/

			//Map the voltages
			double *parNominalVoltage;

			tempProp = gl_get_property(parent,"nominal_voltage");
			if ((tempProp==NULL || tempProp->ptype!=PT_double))
			{
				GL_THROW("Unable to map nominal_voltage property - ensure the parent is a powerflow:meter");
				/*  TROUBLESHOOT
				While attempting to map the nominal_voltage property from the parent object, an error was encountered.
				Please check and make sure your parent object is a meter inside the powerflow module and try
				again.  If the error persists, please submit your code and a bug report via the Trac website.
				*/
			}
			else
				parNominalVoltage = (double*)GETADDR(parent,tempProp);

			// check nominal voltage against rated voltage
			if ( fabs(1 - (*parNominalVoltage * sqrt(3.0) / Rated_V) ) > 0.1 )
				gl_warning("windturb_dg (id:%d, name:%s): Rated generator voltage (LL: %.1f) and nominal voltage (LL: %.1f) of meter parent are different by greater than 10 percent. Odd behavior may occur.",obj->id,obj->name,Rated_V,*parNominalVoltage * sqrt(3.0));
			/* TROUBLESHOOT
			Currently, the model allows you to attach the turbine to a voltage that is quite different than the rated terminal
			voltage of the generator.  However, this may cause odd behavior, as the solved powerflow voltage is used to calculate
			the generator induced voltages and conversion from mechanical power.  It is recommended that the nominal
			voltages of the parent meter be within ~10% of the rated voltage.
			*/

			// attach meter variables to each circuit
			for (i=0; i<sizeof(map)/sizeof(map[0]); i++)
			{
				*(map[i].var) = get_complex(parent,map[i].varname);

				if (*(map[i].var) == NULL)
				{
					GL_THROW("Unable to map variable %s",map[i].varname);
					/*  TROUBLESHOOT
					The variable name was not found when mapping it
					*/
				}
			}
		}
		else if (gl_object_isa(parent,"triplex_meter","powerflow"))
		{
			GL_THROW("The wind turbine model does currently support direct connection to single phase or triplex meters. Connect through a rectifier-inverter combination.");
			/*  TROUBLESHOOT
			This model does not currently support connection to a triplex system.  Please connect to a 3-phase meter.
			*/

			//Map voltage
			pCircuit_V = get_complex(parent,"voltage_1");

			//Make sure it worked
			if (pCircuit_V == NULL)
				GL_THROW("Unable to map triplex_meter voltage");

			//Map current
			pLine_I = get_complex(parent,"current_1");

			//Make sure it worked
			if (pLine_I == NULL)
				GL_THROW("Unable to map triplex_meter current");

			//NR_mode = get_bool(parent,"NR_mode");
		}
		else if (gl_object_isa(parent,"rectifier","generators"))
		{

			//Map the voltages
			double *parNominalVoltage;

			tempProp = gl_get_property(parent,"V_Rated");
			if ((tempProp==NULL || tempProp->ptype!=PT_double))
			{
				GL_THROW("Unable to map V_Rated property - ensure the parent is a powerflow:meter");
				/*  TROUBLESHOOT
				While attempting to map the nominal_voltage property from the parent object, an error was encountered.
				Please check and make sure your parent object is a meter inside the powerflow module and try
				again.  If the error persists, please submit your code and a bug report via the Trac website.
				*/
			}
			else
				parNominalVoltage = (double*)GETADDR(parent,tempProp);

			// check nominal voltage against rated voltage
			if ( fabs(1 - (*parNominalVoltage / Rated_V) ) > 0.1 )
				gl_warning("windturb_dg (id:%d, name:%s): Rated generator voltage (LL: %.1f) and nominal voltage (LL: %.1f) of meter parent are different by greater than 10 percent. Odd behavior may occur.",obj->id,obj->name,Rated_V,*parNominalVoltage * sqrt(3.0));
			/* TROUBLESHOOT
			Currently, the model allows you to attach the turbine to a voltage that is quite different than the rated terminal
			voltage of the generator.  However, this may cause odd behavior, as the solved powerflow voltage is used to calculate
			the generator induced voltages and conversion from mechanical power.  It is recommended that the nominal
			voltages of the parent meter be within ~10% of the rated voltage.
			*/



			// attach meter variables to each circuit
			for (i=0; i<sizeof(map)/sizeof(map[0]); i++)
			{
				if ((*(map[i].var) = get_complex(parent,map[i].varname))==NULL)
				{
					GL_THROW("%s (%s:%d) does not implement rectifier variable %s for %s (windturb_dg:%d)", 
						/*	TROUBLESHOOT
						The rectifier requires that the inverter contains certain published properties in order to properly connect. If you encounter this error, please report it to the developers, along with
						the version of GridLAB-D that raised this error.
						*/
						parent->name?parent->name:"unnamed object", parent->oclass->name, parent->id, map[i].varname, obj->name?obj->name:"unnamed", obj->id);
				}
			}

		}
		else
		{
			GL_THROW("windturb_dg (id:%d): Invalid parent object",obj->id);
			/* TROUBLESHOOT 
			The wind turbine object must be attached a 3-phase meter object.  Please check parent of object.
			*/
		}
	}
	else
	{
		gl_warning("windturb_dg:%d %s", obj->id, parent==NULL?"has no parent meter defined":"parent is not a meter");	

		// attach meter variables to each circuit in the default_meter
		*(map[0].var) = &default_line123_voltage[0];
		*(map[1].var) = &default_line1_current[0];

		// provide initial values for voltages
		default_line123_voltage[0] = complex(Rated_V/sqrt(3.0),0);
		default_line123_voltage[1] = complex(Rated_V/sqrt(3.0)*cos(2*PI/3),Rated_V/sqrt(3.0)*sin(2*PI/3));
		default_line123_voltage[2] = complex(Rated_V/sqrt(3.0)*cos(-2*PI/3),Rated_V/sqrt(3.0)*sin(-2*PI/3));

		NR_mode = &default_NR_mode;
	}

	if (Gen_status==OFFLINE)
	{
		gl_warning("init_windturb_dg (id:%d,name:%s): Generator is out of service!", obj->id,obj->name); 	
	}	

	if (Gen_type == SYNCHRONOUS || Gen_type == INDUCTION)
	{
		if (Gen_mode == CONSTANTE)
		{
			gl_warning("init_windturb_dg (id:%d,name:%s): Synchronous and induction generators in constant voltage mode has not been fully tested and my not work properly.", obj->id,obj->name);
		}
	}

	if (Rated_VA!=0.0)  
		SB = Rated_VA/3;
	if (Rated_V!=0.0)  
		EB = Rated_V/sqrt(3.0);
	if (SB!=0.0)  
		ZB = EB*EB/SB;
	else 
		GL_THROW("Generator power capacity not specified!");
	/* TROUBLESHOOT 
	Rated_VA of generator must be specified so that per unit values can be calculated
	*/

	if (Gen_type == INDUCTION)  
	{
		complex Zrotor(Rr,Xr);
		complex Zmag = complex(Rc*Xm*Xm/(Rc*Rc + Xm*Xm),Rc*Rc*Xm/(Rc*Rc + Xm*Xm));
		complex Zstator(Rst,Xst);

		//Induction machine two-port matrix.
		IndTPMat[0][0] = (Zmag + Zstator)/Zmag;
		IndTPMat[0][1] = Zrotor + Zstator + Zrotor*Zstator/Zmag;
		IndTPMat[1][0] = complex(1,0) / Zmag;
		IndTPMat[1][1] = (Zmag + Zrotor) / Zmag;
	}

	else if (Gen_type == SYNCHRONOUS)  
	{
		double Real_Rs = Rs * ZB; 
		double Real_Xs = Xs * ZB;
		double Real_Rg = Rg * ZB; 
		double Real_Xg = Xg * ZB;
		tst = complex(Real_Rg,Real_Xg);
		tst2 = complex(Real_Rs,Real_Xs);
		AMx[0][0] = tst2 + tst;			//Impedance matrix
		AMx[1][1] = tst2 + tst;
		AMx[2][2] = tst2 + tst;
		AMx[0][1] = AMx[0][2] = AMx[1][0] = AMx[1][2] = AMx[2][0] = AMx[2][1] = tst;
		tst3 = (complex(1,0) + complex(2,0)*tst/tst2)/(tst2 + complex(3,0)*tst);
		tst4 = (-tst/tst2)/(tst2 + tst);
		invAMx[0][0] = tst3;			//Admittance matrix (inverse of Impedance matrix)
		invAMx[1][1] = tst3;
		invAMx[2][2] = tst3;
		invAMx[0][1] = AMx[0][2] = AMx[1][0] = AMx[1][2] = AMx[2][0] = AMx[2][1] = tst4;
	}
	else
		GL_THROW("Unknown generator type specified");
	
	
	return 1;
}

// Presync is called when the clock needs to advance on the first top-down pass */
TIMESTAMP wind_service::presync(TIMESTAMP t0, TIMESTAMP t1)
{
	//current_A = current_B = current_C = 0.0;
	return TS_NEVER; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

TIMESTAMP wind_service::sync(TIMESTAMP t0, TIMESTAMP t1) 
{
	TIMESTAMP t2 = TS_NEVER;
	///CALL SYNC METHOD FOR WEB SERVICE
	//INPUT PARAMETERS FOR FUNCTION CALL
	double pCircuit_V_A_Re=pCircuit_V[0].Re();
	double pCircuit_V_A_Im=pCircuit_V[0].Im();
	double pCircuit_V_B_Re=pCircuit_V[1].Re();
	double pCircuit_V_B_Im=pCircuit_V[1].Im();
	double pCircuit_V_C_Re=pCircuit_V[2].Re();
	double pCircuit_V_C_Im=pCircuit_V[2].Im();
	
    TotalRealPow = pmy_wind_turbine->windturb_dgsync(pCircuit_V_A_Re, pCircuit_V_A_Im, pCircuit_V_B_Re, pCircuit_V_B_Im, pCircuit_V_C_Re, pCircuit_V_C_Im);
	
	 // the return value saved in TotalRealPow variable
	//double store_current_current = current_A.Mag() + current_B.Mag() + current_C.Mag();
	//if ( fabs((store_current_current - store_last_current) / store_last_current) > 0.005 )
	//	t2 = t1;


	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */	
}
/* Postsync is called when the clock needs to advance on the second top-down pass */
TIMESTAMP wind_service::postsync(TIMESTAMP t0, TIMESTAMP t1)
{
	TIMESTAMP t2 = TS_NEVER;
	// Handling for NR || FBS
	//if ( *NR_mode == true || ( *NR_mode == false && last_NR_mode == false )) 
	//{
		//Remove our parent contributions (so XMLs look proper)


			pLine_I[0].Re() -= current_A.Re();
			pLine_I[1].Re() -= current_B.Re();
			pLine_I[2].Re() -= current_C.Re();

			pLine_I[0].Im() -= current_A.Im();
			pLine_I[1].Im() -= current_B.Im();
			pLine_I[2].Im() -= current_C.Im();
	//}

	//last_NR_mode = *NR_mode;

	return t2; /* return t2>t1 on success, t2=t1 for retry, t2<t1 on failure */
}

bool *wind_service::get_bool(OBJECT *obj, char *name)
{
	PROPERTY *p = gl_get_property(obj,name);
	if (p==NULL || p->ptype!=PT_bool)
		return NULL;
	return (bool*)GETADDR(obj,p);
}

complex *wind_service::get_complex(OBJECT *obj, char *name)
{
	PROPERTY *p = gl_get_property(obj,name);
	if (p==NULL || p->ptype!=PT_complex)
		return NULL;
	return (complex*)GETADDR(obj,p);
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF CORE LINKAGE: wind_service
//////////////////////////////////////////////////////////////////////////

EXPORT int create_wind_service(OBJECT **obj, OBJECT *parent)
{
	try
	{
		*obj = gl_create_object(wind_service::oclass);
		if (*obj!=NULL)
		{
			wind_service *my = OBJECTDATA(*obj,wind_service);
			gl_set_parent(*obj,parent);
			return my->create();
		}
		else 
			return 0;
	}
	CREATE_CATCHALL(wind_service);
}



EXPORT int init_wind_service(OBJECT *obj, OBJECT *parent) 
{
	try 
	{
		if (obj!=NULL)
			return OBJECTDATA(obj,wind_service)->init(parent);
		else
			return 0;
	}
	INIT_CATCHALL(wind_service);
}

EXPORT TIMESTAMP sync_wind_service(OBJECT *obj, TIMESTAMP t0, PASSCONFIG pass)
{
	TIMESTAMP t1 = TS_NEVER;
	wind_service *my = OBJECTDATA(obj,wind_service);
	try
	{
		switch (pass) {
		case PC_PRETOPDOWN:
			t1 = my->presync(obj->clock,t0);
			break;
		case PC_BOTTOMUP:
			t1 = my->sync(obj->clock,t0);
			break;
		case PC_POSTTOPDOWN:
			t1 = my->postsync(obj->clock,t0);
			break;
		default:
			GL_THROW("invalid pass request (%d)", pass);
			break;
		}
	}
	SYNC_CATCHALL(wind_service);
	return t1;
}


/*
[1]	Malinga, B., Sneckenberger, J., and Feliachi, A.; "Modeling and Control of a Wind Turbine as a Distributed Resource", 
Proceedings of the 35th Southeastern Symposium on System Theory, Mar 16-18, 2003, pp. 108-112.

[2]	Cotrell J.R., "A preliminary evaluation of a multiple-generator drivetrain configuration for wind turbines,"
in Proc. 21st ASME Wind Energy Symposium, 2002, pp. 345-352.
*/
