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
package com.vmware.identity.rest.core.server.authorization.token;

/**
 * An enumeration describing the styles of tokens that are
 * accepted by authorization.
 */
public enum TokenStyle {
    /**
     * A token defined through the use of an HTTP request header field.
     *
     * @see <a href="https://tools.ietf.org/html/rfc6749">The OAuth 2.0 Authorization Framework</a>
     */
    HEADER,

    /**
     * A tokens defined through the use of the query component of a URI.
     *
     * @see <a href="https://tools.ietf.org/html/rfc3986">Uniform Resource Identifier (URI): Generic Syntax</a>
     */
    QUERY,

    /**
     * TODO A token defined through the use of an HTTP request message body.
     *
     * @see <a href="https://tools.ietf.org/html/rfc7230">Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing</a>
     */
    BODY
}
