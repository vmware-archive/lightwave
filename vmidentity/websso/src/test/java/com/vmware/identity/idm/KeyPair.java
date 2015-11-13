/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
package com.vmware.identity.idm;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.util.List;

/**
 * A tuple consisting of a public and private (optionally) key
 *
 */
public class KeyPair {
	private PrivateKey privateKey;
	private List<Certificate> certificateChain;
	
	/**
	 * Getter for a private key
	 * @return
	 */
	public PrivateKey getPrivateKey() {
		return privateKey;
	}
	
	/**
	 * Setter for a private key
	 * @param privateKey
	 */
	public void setPrivateKey(PrivateKey privateKey) {
		this.privateKey = privateKey;
	}
	
	/**
	 * Getter for a certificate chain
	 * @return
	 */
	public List<Certificate> getCertificateChain() {
		return certificateChain;
	}
	
	/**
	 * Setter for a certificate chain
	 * @param certificateChain
	 */
	public void setCertificateChain(List<Certificate> certificateChain) {
		this.certificateChain = certificateChain;
	}

}
