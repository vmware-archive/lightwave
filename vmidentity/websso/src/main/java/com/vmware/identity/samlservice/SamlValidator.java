/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
package com.vmware.identity.samlservice;

import java.util.Locale;

import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.springframework.context.MessageSource;
import org.springframework.context.NoSuchMessageException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * Interface that validates SAML messages.
 * We return a ValidationResult object which is tuple of {response code, status, substatus, message}
 *
 */
public interface SamlValidator<T> {

	/**
	 * Validate the message and return validation result structure
	 * @param t
	 * @return
	 */
	ValidationResult validate(T t);

	public class ValidationResult {
		private int responseCode;
		private String status;
		private String substatus;
	    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
	            .getLogger(ValidationResult.class);
		/**
		 * Create result {success}
		 */
		public ValidationResult() {
			// preset to return 200 (success)
			setResponseCode(HttpServletResponse.SC_OK);
		}

		/**
		 * Create result as specified.
		 * requirement: there must be an entry in message.properties matching 's.ss'
		 * @param rc
		 * @param s
		 * @param ss
		 */
		public ValidationResult(int rc, String s, String ss) {
			setResponseCode(rc);
			setStatus(s);
			setSubstatus(ss);
		}

		/**
		 * Create result as specified (result code is 200)
		 * requirement: there must be an entry in message.properties matching 's.ss'
		 * @param s
		 * @param ss
		 */
		public ValidationResult(String s, String ss) {
			setResponseCode(HttpServletResponse.SC_OK);
			setStatus(s);
			setSubstatus(ss);
		}

		/**
		 * Create result as specified (result code is 200, substatus is null)
		 * @param s
		 */
		public ValidationResult(String s) {
			setResponseCode(HttpServletResponse.SC_OK);
			setStatus(s);
			setSubstatus(null);
		}

		public int getResponseCode() {
			return responseCode;
		}

		public void setResponseCode(int responseCode) {
			this.responseCode = responseCode;
		}

		public String getStatus() {
			return status;
		}

		public void setStatus(String status) {
			this.status = status;
		}

		public String getSubstatus() {
			return substatus;
		}

		public void setSubstatus(String substatus) {
			this.substatus = substatus;
		}

		public String getMessage(MessageSource messageSource, Locale locale) {
			Validate.notNull(messageSource);
			String translatedMessage = null;

			try {
				if (status == null) {
					translatedMessage = messageSource.getMessage("DefaultMessage", null, locale);
				} else {
					if (substatus == null) {
						translatedMessage = messageSource.getMessage(
								status.replace(OasisNames.STATUS_PREFIX, ""), null, locale);
					} else {
						translatedMessage = messageSource.getMessage(
								status.replace(OasisNames.STATUS_PREFIX, "") + "." +
										substatus.replace(OasisNames.STATUS_PREFIX, ""), null, locale);
					}
				}
			} catch (NoSuchMessageException e) {
                log.warn("Encountered status code that is not localized. "+e.getMessage());
                translatedMessage = this.getBaseLocaleMessage();
            }
			return translatedMessage;
		}

		public boolean isValid() {
			return responseCode == HttpServletResponse.SC_OK && status == null && substatus == null;
		}

		/**
		 * return if the validation result expect login view to be served.
		 */
		public boolean needsLogonView() {
			return responseCode == HttpServletResponse.SC_FOUND && status == null;
		}

		/**
		 * get the unstranslated message
		 */
		public String getBaseLocaleMessage() {
			String message = null;
			if (status == null) {
			    message = OasisNames.SUCCESS;
			} else {
			    message = status.toString();
			    if (substatus != null) {
			        message = message + ", "+ substatus.toString();
			    }
			}
			return message;
        }

		public boolean isRedirect() {
			return responseCode == HttpServletResponse.SC_FOUND && status != null;
        }
	}
}
