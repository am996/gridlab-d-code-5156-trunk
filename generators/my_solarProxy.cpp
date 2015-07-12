// This file generated by staff_codegen
// For more information please visit: http://code.google.com/p/staff/
// DO NOT EDIT

#include <memory>
#include <staff/utils/SharedPtr.h>
#include <staff/utils/Log.h>
#include <staff/utils/fromstring.h>
#include <staff/utils/tostring.h>
#include <staff/utils/HexBinary.h>
#include <staff/utils/Base64Binary.h>
#include <staff/common/DataObject.h>
#include <staff/common/Attribute.h>
#include <staff/common/Operation.h>
#include <staff/common/Exception.h>
#include <staff/client/ServiceFactory.h>
#include <staff/client/IProxyAllocator.h>
#include <staff/client/ICallback.h>
#include <staff/client/Options.h>
#include "my_solarProxy.h"

namespace staff
{

///////////////////////////////////////////////////////////////////////////////////////////////////////
// typedef deserializators
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// classes implementation



// proxy allocator
class my_solarProxyAllocator: public staff::IProxyAllocator
{
public:
  my_solarProxyAllocator()
  {
    try
    {
      staff::ServiceFactory::Inst().RegisterProxyAllocator(typeid(my_solar).name(), *this);
    }
    STAFF_CATCH_ALL_DESCR("Failed to register proxy allocator my_solar");
  }

  virtual staff::IService* AllocateProxy(const std::string& sServiceUri,
                                         const std::string& sSessionId,
                                         const std::string& sInstanceId)
  {
    std::auto_ptr<my_solarProxy> tpProxy(new my_solarProxy);
    tpProxy->Init(sServiceUri, sSessionId, sInstanceId);
    return tpProxy.release();
  }

  virtual staff::IService* AllocateProxy(const std::string& sBaseUri,
                                         const std::string& sServiceName,
                                         const std::string& sSessionId,
                                         const std::string& sInstanceId)
  {
    std::auto_ptr<my_solarProxy> tpProxy(new my_solarProxy);
    tpProxy->Init(sBaseUri + (sServiceName.empty() ? "my_solar" : sServiceName),
                  sSessionId, sInstanceId);
    return tpProxy.release();
  }
};

my_solarProxyAllocator tmy_solarProxyAllocatorInitializer;

// asynch proxies


// service proxy
my_solarProxy::my_solarProxy()
{
}

my_solarProxy::~my_solarProxy()
{
  try
  {
    Deinit();
  }
  STAFF_CATCH_ALL;
}

void my_solarProxy::Init(const std::string& sServiceUri, const std::string& sSessionId, const std::string& sInstanceId)
{
  staff::IService::Init("my_solar", sSessionId, sInstanceId);
  m_sServiceUri = !sServiceUri.empty() ? sServiceUri : "http://10.96.12.45:9090/axis2/services/my_solar";
  m_tClient.Init(m_sServiceUri);
  staff::Options& rOptions = m_tClient.GetOptions();
  rOptions.SetDefaultNamespace("http://tempui.org/", "");
  rOptions.SetSoapVersion(staff::Options::Soap11);

  rOptions.SetSessionId(sSessionId);

  if (!sInstanceId.empty())
  {
    staff::Operation tOperation("CreateInstance");
    tOperation.Request().CreateChild("sInstanceId").SetText(sInstanceId);
    tOperation.SetResponse(m_tClient.Invoke(tOperation.Request()));
    if (m_tClient.GetLastResponseHasFault())
    {
      STAFF_ASSERT_SOAPFAULT(!tOperation.IsFault(), tOperation.GetFaultCode(),
                             tOperation.GetFaultString(), tOperation.GetFaultDetail()); // soap fault
      STAFF_THROW(::staff::RemoteException, "Failed to invoke service: " +
                  tOperation.GetResponse().ToString()); // other fault
    }
    rOptions.SetInstanceId(sInstanceId);
  }
}

void my_solarProxy::Deinit()
{
  if (!staff::IService::GetInstanceId().empty())
  {
    staff::Operation tOperation("FreeInstance");
    tOperation.Request().CreateChild("sInstanceId").SetText(staff::IService::GetInstanceId());
    tOperation.SetResponse(m_tClient.Invoke(tOperation.Request()));
    if (m_tClient.GetLastResponseHasFault())
    {
      STAFF_ASSERT_SOAPFAULT(!tOperation.IsFault(), tOperation.GetFaultCode(),
                             tOperation.GetFaultString(), tOperation.GetFaultDetail()); // soap fault
      STAFF_THROW(::staff::RemoteException, "Failed to invoke service: " +
                  tOperation.GetResponse().ToString()); // other fault
    }
  }
}

staff::ServiceClient* my_solarProxy::GetClient()
{
  return &m_tClient;
}


double my_solarProxy::solarcreate()
{
  staff::Operation tOperation("solarcreate", "solarcreateResult");
  staff::DataObject& rdoRequest = tOperation.Request();
  rdoRequest.SetNamespaceUriGenPrefix("http://tempui.org/");
  // synchronous call
  tOperation.SetResponse(m_tClient.Invoke(rdoRequest));
  if (m_tClient.GetLastResponseHasFault())
  {
    STAFF_ASSERT_SOAPFAULT(!tOperation.IsFault(), tOperation.GetFaultCode(),
                           tOperation.GetFaultString(), tOperation.GetFaultDetail()); // soap fault
    STAFF_THROW(::staff::RemoteException, "Failed to invoke service: " + tOperation.GetResponse().ToString()); // other fault
  }

  const staff::DataObject& rdoResult = tOperation.ResultOpt();
  double tReturn = 0;
  rdoResult.GetValue(tReturn);
  return tReturn;
}

int my_solarProxy::solarinit(int gen_mode_v, int gen_status_v, int panel_type_v)
{
  staff::Operation tOperation("solarinit", "solarinitResult");
  staff::DataObject& rdoRequest = tOperation.Request();
  rdoRequest.SetNamespaceUriGenPrefix("http://tempui.org/");
  rdoRequest.CreateChild("gen_mode_v", gen_mode_v);
  rdoRequest.CreateChild("gen_status_v", gen_status_v);
  rdoRequest.CreateChild("panel_type_v", panel_type_v);

  // synchronous call
  tOperation.SetResponse(m_tClient.Invoke(rdoRequest));
  if (m_tClient.GetLastResponseHasFault())
  {
    STAFF_ASSERT_SOAPFAULT(!tOperation.IsFault(), tOperation.GetFaultCode(),
                           tOperation.GetFaultString(), tOperation.GetFaultDetail()); // soap fault
    STAFF_THROW(::staff::RemoteException, "Failed to invoke service: " + tOperation.GetResponse().ToString()); // other fault
  }

  const staff::DataObject& rdoResult = tOperation.ResultOpt();
  int tReturn = 0;
  rdoResult.GetValue(tReturn);
  return tReturn;
}

double my_solarProxy::solarsync(int t0, int t1)
{
  staff::Operation tOperation("solarsync", "solarsyncResult");
  staff::DataObject& rdoRequest = tOperation.Request();
  rdoRequest.SetNamespaceUriGenPrefix("http://tempui.org/");
  rdoRequest.CreateChild("t0", t0);
  rdoRequest.CreateChild("t1", t1);

  // synchronous call
  tOperation.SetResponse(m_tClient.Invoke(rdoRequest));
  if (m_tClient.GetLastResponseHasFault())
  {
    STAFF_ASSERT_SOAPFAULT(!tOperation.IsFault(), tOperation.GetFaultCode(),
                           tOperation.GetFaultString(), tOperation.GetFaultDetail()); // soap fault
    STAFF_THROW(::staff::RemoteException, "Failed to invoke service: " + tOperation.GetResponse().ToString()); // other fault
  }

  const staff::DataObject& rdoResult = tOperation.ResultOpt();
  double tReturn = 0;
  rdoResult.GetValue(tReturn);
  return tReturn;
}

