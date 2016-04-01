package com.vmware.pscsetup.interop;

import com.vmware.identity.configure.DomainControllerNativeException;

public class DomainControllerAccessDeniedException extends DomainControllerNativeException{

	private static final long serialVersionUID = 7296694862269963267L;

	public DomainControllerAccessDeniedException(int errCode) {
		super(errCode);
	}

}
