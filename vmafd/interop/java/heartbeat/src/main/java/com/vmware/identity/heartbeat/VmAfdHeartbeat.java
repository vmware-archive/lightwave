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
