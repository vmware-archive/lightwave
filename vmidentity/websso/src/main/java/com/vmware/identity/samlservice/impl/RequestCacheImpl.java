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
package com.vmware.identity.samlservice.impl;

import java.util.LinkedHashMap;

import org.apache.commons.lang.Validate;

import com.vmware.identity.samlservice.RequestCache;

/**
 * Singleton request cache implementation which allows
 *  - MAX_RETRIES for a single request
 *  - MAX_ENTRIES for a total number of requests being tracked
 *
 */
public class RequestCacheImpl extends LinkedHashMap<String, Integer> implements RequestCache {
	private static RequestCacheImpl singleton;

	// hide the constructor
	private RequestCacheImpl() {
	}

	private static final long serialVersionUID = 1L;
	private static final int MAX_RETRIES = 5; // maximum number of times we can process same SamlRequest (we allow for a few retries due to multi-step SPNEGO)
	private static final int MAX_ENTRIES = 1024; // maximum number of simultaneous requests to track (then eldest request will be removed from cache)

	/**
	 * Return singleton instance of request cache
	 * @return
	 */
	public static RequestCacheImpl getRequestCache() {
		if (singleton == null) {
			singleton = new RequestCacheImpl();
		}
		return singleton;
	}

	/* (non-Javadoc)
	 * @see com.vmware.identity.samlservice.RequestCache#shouldDenyRequest(java.lang.String)
	 */
	public boolean shouldDenyRequest(String request) {
		boolean retval = false;

		if (this.containsKey(request)) {
			Integer retries = this.get(request);
			Validate.notNull(retries);
			if (retries >= MAX_RETRIES) {
				retval = true;
			}
		}
		return retval;
	}

	/* (non-Javadoc)
	 * @see com.vmware.identity.samlservice.RequestCache#storeRequest(java.lang.String)
	 */
	public void storeRequest(String request) {
		if (this.containsKey(request)) {
			Integer retries = this.get(request);
			Validate.notNull(retries);
			retries++;
			this.put(request, retries);
		} else {
			this.put(request, 1);
		}
	}

	/**
	 * Query a request. Return true if the request can be found in the cache.
	 * @param	request		The full quest string
	 */
	public boolean isExistingRequest(String request) {
		if (this.containsKey(request)) {
			return true;
		} else {
			return false;
		}
	}


	@Override
	protected boolean removeEldestEntry(
			java.util.Map.Entry<String, Integer> eldest) {
		return this.size() > MAX_ENTRIES; // our strategy to keep cache size in check
	}
}
