package com.vmware.pscsetup.interop;

public class DomainControllerInvalidPasswordException extends DomainControllerNativeException {

	private static final long serialVersionUID = 1890674391299348626L;

	public DomainControllerInvalidPasswordException(int errCode) {
		super(errCode);
	}

}
