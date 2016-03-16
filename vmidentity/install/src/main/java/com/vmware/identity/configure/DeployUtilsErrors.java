package com.vmware.identity.configure;

public enum DeployUtilsErrors {

	ERROR_ACCESS_DENIED(5),
	ERROR_INVALID_PARAMETER(87),
	ERROR_INVALID_NETNAME(1214),
	ERROR_PASSWORD_RESTRICTION(1325),
	LW_ERROR_PASSWORD_RESTRICTION(40127);
	private int errorCode;

	private DeployUtilsErrors(int errorCode) {
		this.errorCode = errorCode;
	}

	public int getErrorCode() {
		return errorCode;
	}
}
