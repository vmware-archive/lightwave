package com.vmware.pscsetup.interop;

import com.vmware.identity.configure.DomainControllerNativeException;

public class DomainControllerInvalidPasswordException extends DomainControllerNativeException {

	private static final long serialVersionUID = 1890674391299348626L;

	public DomainControllerInvalidPasswordException(int errCode) {
		super(errCode);
	}

}
