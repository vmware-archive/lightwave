package com.vmware.pscsetup.interop;

import com.vmware.identity.configure.DomainControllerNativeException;

public class DomainControllerInvalidHostnameException extends DomainControllerNativeException {

	private static final long serialVersionUID = -4425593677902862166L;

	public DomainControllerInvalidHostnameException(int errCode) {
		super(errCode);
	}

}
