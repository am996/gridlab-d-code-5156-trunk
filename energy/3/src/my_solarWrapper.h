// This file generated by staff_codegen
// For more information please visit: http://code.google.com/p/staff/
// DO NOT EDIT

#ifndef _my_solarWrapper_h_
#define _my_solarWrapper_h_

#include <string>
#include <staff/component/ServiceWrapper.h>
#include "my_solar.h"

namespace staff
{
  class Component;
}


  class my_solarImpl;

  //! my_solar service wrapper
  class my_solarWrapper: public staff::ServiceWrapper
  {
  public:
    //! initializing constructor
    /*! \param  pComponent - service's component
    */
    my_solarWrapper(staff::Component* pComponent);

    //! destructor
    virtual ~my_solarWrapper();

    //! get service name
    /*! \return service name
    */
    virtual const std::string& GetName() const;

    //! get service description
    /*! \return service description
    */
    virtual const std::string& GetDescr() const;

    //! get service operations
    /*! \return service operations DataObject
    */
    virtual staff::DataObject GetOperations() const;

    //! get service description
    /*! \return service description
    */
    virtual staff::DataObject GetServiceDescription() const;

    //! invoke service operation
    /*! \param  rOperation - service operation
        \param  sID - service session id
        */
    virtual void Invoke(staff::Operation& rOperation, const std::string& sSessionId, const std::string& sInstanceId);

    //! get service's component
    /*! \return service's component
    */
    virtual const staff::Component* GetComponent() const;

    //! get service's component
    /*! \return service's component
    */
    virtual staff::Component* GetComponent();

    //! get pointer to service implementation
    /*! \param  sSessionId - service session id
        \param  sInstanceId - service instance id
        \return pointer to service implementation
        */
    virtual staff::PIService& GetImpl(const std::string& sSessionId, const std::string& sInstanceId);

    //! create new service impl
    /*! \return resulting service impl
      */
    virtual staff::PIService NewImpl();

    //! load service at startup
    /*! \return true, if service needed to be loaded at startup
      */
    virtual bool IsLoadAtStartup() const;

    //! get comma-delimeted list of dependecies
    /*! \return comma-delimeted list of dependecies
      */
    virtual std::string GetDependencies() const;

  private:
    staff::Component* m_pComponent;   //!< parent component
    static const std::string m_sName;  //!< service name
    static const std::string m_sDescr; //!< service description
  };



namespace staff
{
  class DataObject;
}

#endif // _my_solarWrapper_h_

