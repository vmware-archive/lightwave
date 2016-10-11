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

package com.vmware.certificate.interop;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.JUnitCore;

public class TestEndToEnd {

	public static void main(String[] args) throws Exception {                    
	       JUnitCore.main(
	         "com.vmware.certificate.interop.TestEndToEnd"); 
	    /*  
		TestEndToEnd t = new TestEndToEnd();
		t.test();
		*/
	}
	
	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public final void test() throws Exception {
//		Client vmcaClient = new Client("localhost");
//		Request r = vmcaClient.getRequest();
//		r.setName("JavaTest");
//		r.setCountry("US");
//		r.setOrganization("VMware");
//		r.setOrgunit("Engineering");
//		r.setState("California");
//		r.setLocality("Palo Alto");
//		r.setIpaddress("127.0.0.1");
//		r.setEmail("anu.engineer@gmail.com");
//		r.setUri("host.machine.com");
//		
//		KeyPair keys = r.CreateKeyPair(1024);
//		Assert.assertNotNull(keys.getPrivatekey());
//		Assert.assertNotNull(keys.getPublickey());
//		
//		System.out.println(keys.getPrivatekey());
//		
//		java.util.Date NotBefore = new java.util.Date ();
//		java.util.Date NotAfter = new java.util.Date();
//		NotAfter.setYear(NotAfter.getYear() + 1);
//		
//		Certificate c =  r.GetCertificate(keys, NotBefore, NotAfter);
//		Assert.assertNotNull(c.getPEM());
//		
//		System.out.println(c.getPEM());
//		
//		Certificate s = r.GetSelfSignedCertificate(keys, NotBefore, NotAfter);
//		Assert.assertNotNull(s.getPEM());
//		
//		System.out.println(s.getPEM());
	}

}
