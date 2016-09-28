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

package com.vmware.identity.heartbeat;

public class VmAfdHeartbeat implements AutoCloseable{
	   private PointerRef handle;
	   private final String serviceName;
	   private final int port;

	   public VmAfdHeartbeat(String serviceName, int port) {
	      if (serviceName == null) {
          throw new IllegalArgumentException(String.format(
	                  "Service Name cannot be NULL"));
	      }

	      this.serviceName = serviceName;
	      this.port = port;
	   }

	   public void startBeating() throws VmAfdGenericException{
		   if (handle == null)
		   {
			   int error = 0;
			   PointerRef pHandle = new PointerRef();

			   error = VmAfdHeartbeatAdapter.VmAfdStartHeartBeatW(serviceName, port, pHandle);

			   if (error != 0) {
				   throw new VmAfdGenericException(
		                           String.format("Error starting heartbeat for Service '%s' for port '%d'",
		                           serviceName,
		                           port),
		                           error
		                           );
			   }
			   handle = pHandle;
		   }
	   }

	   public void stopBeating(){
		   if (handle != null)
		   {
			    VmAfdHeartbeatAdapter.VmAfdStopHeartbeat(handle);
		   }
		   handle = null;
	   }

	   protected void finalize() throws Throwable {
		      try {
            stopBeating();
		      } finally {
		         super.finalize();
		      }
		   }

	   @Override
	   public void close() throws Exception {
		   this.stopBeating();
	   }
}
