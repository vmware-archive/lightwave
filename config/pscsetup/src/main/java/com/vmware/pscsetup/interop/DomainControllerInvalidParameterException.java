package com.vmware.pscsetup.interop;

public class DomainControllerInvalidParameterException extends DomainControllerNativeException {

	private static final long serialVersionUID = 7443396024801194306L;

	public DomainControllerInvalidParameterException(int errCode) {
		super(errCode);
	}

}
