// This file generated by staff_codegen
// For more information please visit: http://code.google.com/p/staff/
// Service Implementation

#ifndef _my_solarImpl_h_
#define _my_solarImpl_h_

#include "my_solar.h"


  //! implementation of my_solar service
  class my_solarImpl: public my_solar
  {
  public:
    my_solarImpl();
    virtual ~my_solarImpl();
    virtual void OnCreate();
    virtual void OnDestroy();
    virtual double solarcreate();
    virtual int solarinit(int gen_mode_v, int gen_status_v, int panel_type_v);
    virtual double solarsync(int t0, int t1);
  };


#endif // _my_solarImpl_h_
