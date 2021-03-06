// This file generated by staff_codegen
// For more information please visit: http://code.google.com/p/staff/
// DO NOT EDIT

#include <staff/utils/SharedPtr.h>
#include <staff/utils/Log.h>
#include <staff/utils/fromstring.h>
#include <staff/utils/tostring.h>
#include <staff/utils/HexBinary.h>
#include <staff/utils/Base64Binary.h>
#include <staff/common/Attribute.h>
#include <staff/common/Exception.h>
#include <staff/common/DataObject.h>
#include <staff/common/Operation.h>
#include <staff/common/IService.h>
#include <staff/component/ServiceInstanceManager.h>
#include <staff/component/Component.h>
#include "my_microturbineImpl.h"
#include "my_microturbineWrapper.h"

namespace staff
{

///////////////////////////////////////////////////////////////////////////////////////////////////////
// typedef deserializators
}




my_microturbineWrapper::my_microturbineWrapper(staff::Component* pComponent):
  m_pComponent(pComponent)
{
}

my_microturbineWrapper::~my_microturbineWrapper()
{
}

void my_microturbineWrapper::Invoke(staff::Operation& rOperation, const std::string& sSessionId, const std::string& sInstanceId)
{
  const staff::DataObject& rRequest = rOperation.Request();
  const std::string& sOperationName = rOperation.GetName();

  if (sOperationName == "GetServiceDescription")
  {
    rOperation.SetResponse(GetServiceDescription());
  }
  else
  if (sOperationName == "CreateInstance")
  {
    staff::ServiceInstanceManager::Inst().CreateServiceInstance(sSessionId, m_sName,
                                                                rRequest.GetChildTextByLocalName("sInstanceId"));
  }
  else
  if (sOperationName == "FreeInstance")
  {
    staff::ServiceInstanceManager::Inst().FreeServiceInstance(sSessionId, m_sName,
                                                              rRequest.GetChildTextByLocalName("sInstanceId"));
  }
  else
  {
    staff::SharedPtr<my_microturbineImpl> tpServiceImpl = GetImpl(sSessionId, sInstanceId);
    if (sOperationName == "microturbine_init")
    {
      rOperation.Result().SetValue(tpServiceImpl->microturbine_init());
    }
    else
    if (sOperationName == "microturbine_presync")
    {
      int t0 = 0;
      int t1 = 0;
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("t0", t0), "Invalid value for element t0");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("t1", t1), "Invalid value for element t1");
      rOperation.Result().SetValue(tpServiceImpl->microturbine_presync(t0, t1));
    }
    else
    if (sOperationName == "microturbine_sync")
    {
      double CircuitA_V_Out_re = 0;
      double CircuitA_V_Out_im = 0;
      double CircuitB_V_Out_re = 0;
      double CircuitB_V_Out_im = 0;
      double CircuitC_V_Out_re = 0;
      double CircuitC_V_Out_im = 0;
      double LineA_V_Out_re = 0;
      double LineA_V_Out_im = 0;
      double LineB_V_Out_re = 0;
      double LineB_V_Out_im = 0;
      double LineC_V_Out_re = 0;
      double LineC_V_Out_im = 0;
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("CircuitA_V_Out_re", CircuitA_V_Out_re), "Invalid value for element CircuitA_V_Out_re");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("CircuitA_V_Out_im", CircuitA_V_Out_im), "Invalid value for element CircuitA_V_Out_im");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("CircuitB_V_Out_re", CircuitB_V_Out_re), "Invalid value for element CircuitB_V_Out_re");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("CircuitB_V_Out_im", CircuitB_V_Out_im), "Invalid value for element CircuitB_V_Out_im");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("CircuitC_V_Out_re", CircuitC_V_Out_re), "Invalid value for element CircuitC_V_Out_re");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("CircuitC_V_Out_im", CircuitC_V_Out_im), "Invalid value for element CircuitC_V_Out_im");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("LineA_V_Out_re", LineA_V_Out_re), "Invalid value for element LineA_V_Out_re");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("LineA_V_Out_im", LineA_V_Out_im), "Invalid value for element LineA_V_Out_im");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("LineB_V_Out_re", LineB_V_Out_re), "Invalid value for element LineB_V_Out_re");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("LineB_V_Out_im", LineB_V_Out_im), "Invalid value for element LineB_V_Out_im");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("LineC_V_Out_re", LineC_V_Out_re), "Invalid value for element LineC_V_Out_re");
      STAFF_ASSERT(rRequest.GetChildValueByLocalName("LineC_V_Out_im", LineC_V_Out_im), "Invalid value for element LineC_V_Out_im");
      rOperation.Result().SetValue(tpServiceImpl->microturbine_sync(CircuitA_V_Out_re, CircuitA_V_Out_im, CircuitB_V_Out_re, CircuitB_V_Out_im, CircuitC_V_Out_re, CircuitC_V_Out_im, LineA_V_Out_re, LineA_V_Out_im, LineB_V_Out_re, LineB_V_Out_im, LineC_V_Out_re, LineC_V_Out_im));
    }
    else
    {
      STAFF_THROW(staff::RemoteException, "Unknown Operation: " + rOperation.GetName());
    }

    rOperation.GetResponse().SetNamespaceUriGenPrefix("http://tempui.org/my_microturbine");
  }
}

const std::string& my_microturbineWrapper::GetName() const
{
  return m_sName;
}

const std::string& my_microturbineWrapper::GetDescr() const
{
  return m_sDescr;
}

const staff::Component* my_microturbineWrapper::GetComponent() const
{
  return m_pComponent;
}

staff::Component* my_microturbineWrapper::GetComponent()
{
  return m_pComponent;
}

staff::PIService& my_microturbineWrapper::GetImpl(const std::string& sSessionId, const std::string& sInstanceId)
{
  return staff::ServiceInstanceManager::Inst().GetServiceInstance(sSessionId, m_sName, sInstanceId);
}

staff::PIService my_microturbineWrapper::NewImpl()
{
  return new my_microturbineImpl;
}

bool my_microturbineWrapper::IsLoadAtStartup() const
{
  return false;
}

std::string my_microturbineWrapper::GetDependencies() const
{
  return "";
}

staff::DataObject my_microturbineWrapper::GetOperations() const
{
  staff::DataObject tOperations("Operations");

  {// Operation: double microturbine_init()
    staff::DataObject tOpmicroturbine_init = tOperations.CreateChild("Operation");
    tOpmicroturbine_init.CreateChild("Name", "microturbine_init");
    tOpmicroturbine_init.CreateChild("RestMethod", "");
    tOpmicroturbine_init.CreateChild("RestLocation", "");
  }
  {// Operation: int microturbine_presync(int t0, int t1)
    staff::DataObject tOpmicroturbine_presync = tOperations.CreateChild("Operation");
    tOpmicroturbine_presync.CreateChild("Name", "microturbine_presync");
    tOpmicroturbine_presync.CreateChild("RestMethod", "");
    tOpmicroturbine_presync.CreateChild("RestLocation", "");
  }
  {// Operation: double microturbine_sync(double CircuitA_V_Out_re, double CircuitA_V_Out_im, double CircuitB_V_Out_re, double CircuitB_V_Out_im, double CircuitC_V_Out_re, double CircuitC_V_Out_im, double LineA_V_Out_re, double LineA_V_Out_im, double LineB_V_Out_re, double LineB_V_Out_im, double LineC_V_Out_re, double LineC_V_Out_im)
    staff::DataObject tOpmicroturbine_sync = tOperations.CreateChild("Operation");
    tOpmicroturbine_sync.CreateChild("Name", "microturbine_sync");
    tOpmicroturbine_sync.CreateChild("RestMethod", "");
    tOpmicroturbine_sync.CreateChild("RestLocation", "");
  }

  return tOperations;
}

staff::DataObject my_microturbineWrapper::GetServiceDescription() const
{
  staff::DataObject tServiceDescription;

  tServiceDescription.Create("ServiceDescription");
  tServiceDescription.DeclareDefaultNamespace("http://tempui.org/staff/service-description");

  tServiceDescription.CreateChild("Name", m_sName);
  tServiceDescription.CreateChild("Description", m_sDescr);

  tServiceDescription.AppendChild(GetOperations());

  return tServiceDescription;
}

const std::string my_microturbineWrapper::m_sName = "my_microturbine";
const std::string my_microturbineWrapper::m_sDescr = "my_microturbine service";



