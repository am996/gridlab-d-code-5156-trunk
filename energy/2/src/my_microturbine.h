// This file generated by staff_codegen
// For more information please visit: http://code.google.com/p/staff/

#ifndef _MY_MICROTURBINE_H_
#define _MY_MICROTURBINE_H_

#include <staff/common/IService.h>


  //! my_microturbine service
  class my_microturbine: public staff::IService
  {
  public:
    // add your opperations here. Example:
    // virtual void MyOperation() = 0;
	virtual double microturbine_init() = 0;
	virtual int microturbine_presync(int t0, int t1) = 0;
	virtual double microturbine_sync(double CircuitA_V_Out_re, double CircuitA_V_Out_im, double CircuitB_V_Out_re, double CircuitB_V_Out_im, double CircuitC_V_Out_re, double CircuitC_V_Out_im, double LineA_V_Out_re, double LineA_V_Out_im, double LineB_V_Out_re, double LineB_V_Out_im, double LineC_V_Out_re, double LineC_V_Out_im) = 0;

  };


#endif // _MY_MICROTURBINE_H_
