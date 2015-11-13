
/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */



#include <boost/python.hpp>

#include <boost/python/return_opaque_pointer.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/enum.hpp>

#include <string>
#include <stdio.h>
#include <vmafd.h>
#include <afdclient.h>
using namespace boost::python;

PyObject *VMAFDExceptionType = NULL;

void translateVMAFDException(vmafd_exception const &e)
{
  assert(VMAFDExceptionType != NULL);
  boost::python::object pythonExceptionInstance(e);
  PyErr_SetObject(VMAFDExceptionType, pythonExceptionInstance.ptr());
}

BOOST_PYTHON_OPAQUE_SPECIALIZED_TYPE_ID(opaque_)

BOOST_PYTHON_MODULE(vmafd)
{
    class_<client>("client", init<std::string>())
        .def(init<>())
        .def("GetStatus",                &client::GetStatus)
        .def("GetDomainName",            &client::GetDomainName)
        .def("SetDomainName",            &client::SetDomainName)
        .def("GetDomainState",           &client::GetDomainState)
        .def("GetLDU",                   &client::GetLDU)
        .def("SetLDU",                   &client::SetLDU)
        .def("SetRHTTPProxyPort",        &client::SetRHTTPProxyPort)
        .def("SetDCPort",                &client::SetDCPort)
        .def("GetCMLocation",            &client::GetCMLocation)
        .def("GetLSLocation",            &client::GetLSLocation)
        .def("GetDCName" ,               &client::GetDCName)
        .def("SetDCName" ,               &client::SetDCName)
        .def("GetPNID",                  &client::GetPNID)
        .def("SetPNID",                  &client::SetPNID)
        .def("GetCAPath",                &client::GetCAPath)
        .def("SetCAPath",                &client::SetCAPath)
        .def("GetMachineName",           &client::GetMachineName)
        .def("GetMachinePassword",       &client::GetMachinePassword)
        .def("GetMachineCert",           &client::GetMachineCert)
        .def("GetMachinePrivateKey",     &client::GetMachinePrivateKey)
        .def("SetMachineCert",           &client::SetMachineCert)
        .def("SetMachineCertWithString", &client::SetMachineCertWithString)
        .def("GetSiteGUID",              &client::GetSiteGUID)
        .def("GetSiteName",              &client::GetSiteName)
        .def("GetMachineID",             &client::GetMachineID)
        .def("SetMachineID",             &client::SetMachineID)
        .def("JoinDomain",               &client::JoinDomain)
        .def("CreateCertStore",          &client::CreateCertStore,
                                             return_value_policy<return_opaque_pointer>())
        .def("OpenCertStore",            &client::OpenCertStore,
                                             return_value_policy<return_opaque_pointer>())
        .def("CloseCertStore",           &client::CloseCertStore)
        .def("AddCert",                  &client::AddCert)
        .def("AddTrustedRoot",           &client::AddTrustedRoot)
        .def("GetCertByAlias",           &client::GetCertByAlias)
        .def("GetPrivateKeyByAlias",     &client::GetPrivateKeyByAlias)
        .def("GetEntryCount",            &client::GetEntryCount)
        .def("BeginEnumAliases",         &client::BeginEnumAliases,
                                             return_value_policy<return_opaque_pointer>())
        .def("EnumAliases",              &client::EnumAliases)
        .def("EndEnumAliases",           &client::EndEnumAliases)
        .def("DeleteCert",               &client::DeleteCert)
        .def("DeleteCertStore",          &client::DeleteCertStore)
        .def("EnableClientAffinity",     &client::EnableClientAffinity)
        .def("DisableClientAffinity",    &client::DisableClientAffinity)
        .def("GetAffinitizedDC",         &client::GetAffinitizedDC)
        .def("EnumDCEntries",            &client::EnumDCEntries)
        .def("GetCdcState",              &client::GetCdcState)
    ;

    class_<vmafd_exception> vmafd_exceptionClass("vmafd_exception", boost::python::init<int>());

    vmafd_exceptionClass.add_property("message", &vmafd_exception::getMessage)
    .add_property("errorcode", &vmafd_exception::getErrorCode);

    VMAFDExceptionType = vmafd_exceptionClass.ptr();

    boost::python::register_exception_translator<vmafd_exception>
    (&translateVMAFDException);
}
