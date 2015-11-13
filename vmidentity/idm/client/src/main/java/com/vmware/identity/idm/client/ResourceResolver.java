/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.idm.client;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;
 
import org.w3c.dom.ls.LSInput;
import org.w3c.dom.ls.LSResourceResolver;

/** VMware Identity Service
*
* ResourceResovlver class for resolve schemas packaged as resource 
*
* @author:  scott chai <schai@vmware.com>
*
*/

public class ResourceResolver implements LSResourceResolver {
	@Override
	public LSInput resolveResource(
			String type,
			String namespaceURI,
			String publicId,
			String systemId,
			String baseURI) {
		
		InputStream resourceAsStream = this.getClass().getResourceAsStream(systemId);
		return new LSInputImpl(publicId, systemId, resourceAsStream);
	}
	 
	protected class LSInputImpl implements LSInput {
	 
		private String publicId;
		 
		private String systemId;
		 
		private BufferedInputStream inputStream;
		 
		public LSInputImpl(String publicId, String sysId, InputStream input) {
		this.publicId = publicId;
		this.systemId = sysId;
		this.inputStream = new BufferedInputStream(input);
		}
		
		public String getPublicId() {
		return publicId;
		}
		 
		public void setPublicId(String publicId) {
		this.publicId = publicId;
		}
		 
		@Override
		public String getBaseURI() {
		return null;
		}
		 
		@Override
		public InputStream getByteStream() {
		return null;
		}
		 
		@Override
		public boolean getCertifiedText() {
		return false;
		}
		 
		@Override
		public Reader getCharacterStream() {
		return null;
		}
		 
		@Override
		public String getEncoding() {
		return null;
		}
		 
		@Override
		public String getStringData() {
			
			synchronized (inputStream) {
				try {
					byte[] input = new byte[inputStream.available()];
					inputStream.read(input);
					String contents = new String(input);
					return contents;
				}
				catch (IOException e) {
					e.printStackTrace();
					System.out.println("Exception " + e);
					return null;
				}
			}
		}
		 
		@Override
		public void setBaseURI(String baseURI) {
		}
		 
		@Override
		public void setByteStream(InputStream byteStream) {
		}
		 
		@Override
		public void setCertifiedText(boolean certifiedText) {
		}
		 
		@Override
		public void setCharacterStream(Reader characterStream) {
		}
		 
		@Override
		public void setEncoding(String encoding) {
		}
		 
		@Override
		public void setStringData(String stringData) {
		}
		 
		public String getSystemId() {
		return systemId;
		}
		 
		public void setSystemId(String systemId) {
		this.systemId = systemId;
		}
		 
		public BufferedInputStream getInputStream() {
		return inputStream;
		}
		 
		public void setInputStream(BufferedInputStream inputStream) {
		this.inputStream = inputStream;
		}
		 
		 
	} //end of LSInputImpl
 
}
