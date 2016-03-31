package com.vmware.pscsetup.interop;

import java.util.HashMap;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import com.vmware.identity.configure.DeployUtilsErrors;
import com.vmware.identity.configure.DomainControllerNativeException;

class DeployUtilsAdapterErrorHandler {

	interface ErrorHandler {
		void handlerError(int errorCode) throws DomainControllerNativeException;
	}

	private static final Log log = LogFactory
			.getLog(DeployUtilsAdapterErrorHandler.class);

	private static final HashMap<Integer, ErrorHandler> deployUtilsToHandler = new HashMap<Integer, ErrorHandler>();

	static {
		deployUtilsToHandler.put(
				DeployUtilsErrors.ERROR_ACCESS_DENIED.getErrorCode(),
				new ErrorHandler() {

					@Override
					public void handlerError(int errorCode)
							throws DomainControllerNativeException {

						log.error("DomainControllerAccessDeniedException "
								+ errorCode);
						throw new DomainControllerAccessDeniedException(
								errorCode);
					};

				});

		deployUtilsToHandler.put(
				DeployUtilsErrors.ERROR_INVALID_PARAMETER.getErrorCode(),
				new ErrorHandler() {

					@Override
					public void handlerError(int errorCode)
							throws DomainControllerNativeException {

						log.error("DomainControllerInvalidParameterException "
								+ errorCode);
						throw new DomainControllerInvalidParameterException(
								errorCode);
					};

				});

		deployUtilsToHandler.put(
				DeployUtilsErrors.ERROR_INVALID_NETNAME.getErrorCode(),
				new ErrorHandler() {

					@Override
					public void handlerError(int errorCode)
							throws DomainControllerNativeException {

						log.error("DomainControllerInvalidHostnameException "
								+ errorCode);
						throw new DomainControllerInvalidHostnameException(
								errorCode);
					};

				});

		deployUtilsToHandler.put(
				DeployUtilsErrors.ERROR_PASSWORD_RESTRICTION.getErrorCode(),
				new ErrorHandler() {

					@Override
					public void handlerError(int errorCode)
							throws DomainControllerNativeException {

						log.error("DomainControllerInvalidPasswordException "
								+ errorCode);
						throw new DomainControllerInvalidPasswordException(
								errorCode);
					};

				});

		deployUtilsToHandler.put(
				DeployUtilsErrors.LW_ERROR_PASSWORD_RESTRICTION.getErrorCode(),
				new ErrorHandler() {

					@Override
					public void handlerError(int errorCode)
							throws DomainControllerNativeException {

						log.error("DomainControllerInvalidPasswordException "
								+ errorCode);
						throw new DomainControllerInvalidPasswordException(
								errorCode);
					};

				});
	}

	public static void handleErrorCode(int errorCode)
			throws DomainControllerNativeException {
		if (errorCode != 0) {
			ErrorHandler handler = deployUtilsToHandler.get(errorCode);
			if (handler != null) {
				handler.handlerError(errorCode);
			} else {
				throw new DomainControllerNativeException(errorCode);
			}
		}
	}
}
