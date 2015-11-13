/*
 * Copyright 2015 VMware, Inc.  All rights reserved.
 */

using System;

namespace VMDir.Common.VMDirUtilities
{
    public class LdapException : Exception
    {
        public LdapException ()
        {
        }

        public LdapException (string message)
            : base (message)
        {
        }

        public LdapException (string message, Exception inner)
            : base (message, inner)
        {
        }
    }
}

