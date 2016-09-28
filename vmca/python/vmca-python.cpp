
/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
#include <certclient.h>

using namespace boost::python;

static PyObject *VMCAExceptionType = NULL;

static void
translateVMCAException(vmca_exception const &e)
{
  assert(VMCAExceptionType != NULL);
  boost::python::object pythonExceptionInstance(e);
  PyErr_SetObject(VMCAExceptionType, pythonExceptionInstance.ptr());
}



BOOST_PYTHON_MODULE(vmca)
{
    
    enum_<VMCA_ENUM_CERT_RETURN_CODE>("VMCA_ENUM_CERT_RETURN_CODE")
    .value("VMCA_ENUM_SUCCESS", VMCA_ENUM_SUCCESS)
    .value("VMCA_ENUM_END", VMCA_ENUM_END)
    .value("VMCA_ENUM_ERROR", VMCA_ENUM_ERROR)
    .export_values()
    ;

    class_<client>("client", init<std::string>())
    .def("Login", &client::Login)
    .def("GetVersion", &client::GetVersion)
    .def("AddRootCertificate", &client::AddRootCertificate)
    .def("GetRootCACert", &client::GetRootCACert)
    .def("GetRequest", &client::GetRequest)
    .def("OpenEnumHandle", &client::OpenEnumHandle)
    .def("GetNextCertificate", &client::GetNextCertificate, return_value_policy<manage_new_object>())
    .def("CloseEnumHandle", &client::CloseEnumHandle)
    .def("GetCertificate" ,             &client::GetCertificate)
    .def("GetSelfSignedCertificate" ,   &client::GetSelfSignedCertificate)
    .def("GetCertificateFromCSR",       &client::GetCertificateFromCSR)
    .def("Revoke",                      &client::Revoke)
    .def("GetCRL",                      &client::GetCRL)
    .def("Logout",                      &client::Logout)
    ;

    class_<VMCAClient>("VMCAClient", init<std::string,std::string,std::string,std::string>())
    .def("GetVersion", &VMCAClient::GetVersion)
    .def("AddRootCertificate", &VMCAClient::AddRootCertificate)
    .def("GetRootCACert", &VMCAClient::GetRootCACert)
    .def("GetRequest", &VMCAClient::GetRequest)
    .def("OpenEnumHandle", &VMCAClient::OpenEnumHandle)
    .def("GetNextCertificate", &VMCAClient::GetNextCertificate, return_value_policy<manage_new_object>())
    .def("CloseEnumHandle", &VMCAClient::CloseEnumHandle)
    .def("GetCertificate" ,             &VMCAClient::GetCertificate)
    .def("GetSelfSignedCertificate" ,   &VMCAClient::GetSelfSignedCertificate)
    .def("GetCertificateFromCSR",       &VMCAClient::GetCertificateFromCSR)
    .def("Revoke",                      &VMCAClient::Revoke)
    .def("GetCRL",                      &VMCAClient::GetCRL)
    ;

    class_<request>("request")
    .def("CreateKeyPair" ,              &request::CreateKeyPair)
    .def("GetCSR" ,                     &request::GetCSR)
    .def_readwrite("name",              &request::Name)
    .def_readwrite("country",           &request::Country)
    .def_readwrite("locality",          &request::Locality)
    .def_readwrite("state",             &request::State)
    .def_readwrite("organization",      &request::Organization)
    .def_readwrite("orgunit",           &request::OrgUnit)
    .def_readwrite("dnsname",           &request::DNSName)
    .def_readwrite("uri",               &request::Uri)
    .def_readwrite("email",             &request::Email)
    .def_readwrite("ipaddress",         &request::IPAddress)
    .def_readwrite("keyusage",          &request::KeyUsage)
    ;


    class_<certificate>("certificate")
    .def_readwrite("pem",  &certificate::certString)
    .def("__str__", &certificate::print)
    .def("isCACert", &certificate::isCACert)
    ;

    class_<keypair>("keypair")
    .def_readwrite("privatekey", &keypair::privatekey)
    .def_readwrite("publickey", &keypair::publickey)
    ;

    class_<vmcacontext>("vmcacontext")
    .def_readonly("currIndex", &vmcacontext::currIndex)
    .def_readonly("enumStatus", &vmcacontext::enumStatus)
    ;

    class_<vmcacontext2>("vmcacontext2")
    .def_readonly("currIndex", &vmcacontext2::currIndex)
    .def_readonly("enumStatus", &vmcacontext2::enumStatus)
    ;

    class_<vmcacrl>("vmcacrl")
    .def_readwrite("filepath", &vmcacrl::filepath)
    .add_property("nextUpdate", &vmcacrl::GetNextUpdate)
    .add_property("lastUpdate", &vmcacrl::GetLastUpdate)
    .add_property("crlNumber", &vmcacrl::GetCrlNumber)
    ;

    class_<vmca_exception> vmca_exceptionClass("vmca_exception", init<int>());

    vmca_exceptionClass
    .add_property("message", &vmca_exception::getMessage)
    .add_property("errorcode", &vmca_exception::getErrorCode)
    .def("__str__", &vmca_exception::print)
    ;

    VMCAExceptionType = vmca_exceptionClass.ptr();

    boost::python::register_exception_translator<vmca_exception>
    (&translateVMCAException);
}

