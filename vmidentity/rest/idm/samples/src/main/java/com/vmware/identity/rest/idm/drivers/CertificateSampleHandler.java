/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
 *
 */
package com.vmware.identity.rest.idm.drivers;

import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.util.List;

import org.apache.log4j.Logger;
import org.json.JSONException;
import org.json.JSONObject;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.attributes.CertificateScope;
import com.vmware.identity.rest.idm.samples.CertificateSample;

/**
 * Class for handling calling samples for CertificateHandler from command line.
 * 
 * @author abapat
 *
 */
public class CertificateSampleHandler extends SampleHandler {
	private CertificateSample sample;

	/**
	 * Initializes CertificateSample and logger.
	 */
	public CertificateSampleHandler() {
		log = Logger.getLogger(getClass().getName());
		try {
			sample = new CertificateSample();
		} catch (KeyManagementException | NoSuchAlgorithmException | KeyStoreException | ClientException | IOException e) {
			log.fatal("Error occured when initializing CertificateSample", e);
		}
	}

	@Override
	public String getType() {
		return "certificate";
	}

	@Override
	public void callSample(String operation, String json) {
		String payload = parsePayload(json);
		try {
			if (operation.equalsIgnoreCase("gettenantcertificates") || operation.equalsIgnoreCase("getidpcertificates")) {
				List<CertificateChainDTO> list;
				if (operation.equalsIgnoreCase("gettenantcertificates")) {
					log.info("Getting Tenant Certificates" + tenant);
					list = sample.getCertificateChain(tenant, CertificateScope.TENANT);
				} else {
					log.info("Getting IDP Certificates: " + tenant);
					list = sample.getCertificateChain(tenant, CertificateScope.EXTERNAL_IDP);
				}
				StringBuilder sb = new StringBuilder();
				int count = 1;
				for (CertificateChainDTO chain : list) {
					sb.append("-----BEGIN CHAIN " + count + "-----\n");
					for (CertificateDTO c : chain.getCertificates()) {
						sb.append(c.toPrettyString() + "\n");
					}
					sb.append("-----END CHAIN " + count++ + "-----\n");
				}
				log.info(sb.toString());
			} else if (operation.equalsIgnoreCase("getprivatekey")) {
				log.info("Getting Private Key: " + tenant);
				PrivateKeyDTO key = sample.getPrivateKey(tenant);
				log.info(key.toPrettyString());
			} else if (operation.equalsIgnoreCase("delete")) {
				JSONObject JSON = new JSONObject(payload);
				log.info("Deleting Certificate: " + payload);
				sample.delete(tenant, JSON.getString("fingerprint"));
			} else {
				log.fatal("Invalid command: " + operation);
			}
		} catch (JSONException e) {
			log.fatal("Error when parsing payload", e);
		} catch (Exception e) {
			log.fatal("Error when calling sample", e);
		}

	}
}
