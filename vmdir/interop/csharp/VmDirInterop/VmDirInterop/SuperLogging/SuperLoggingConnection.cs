/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using VmDirInterop.SuperLogging.Constants;
using VmDirInterop.SuperLogging.Exceptions;
using VmDirInterop.SuperLogging.Interfaces;

namespace VmDirInterop.SuperLogging
{
    public class SuperLoggingConnection : ISuperLoggingConnection
    {
        private IntPtr _serverContext = IntPtr.Zero;

        public SuperLoggingConnection()
        {
        }

        public void openA(string networkAddress, string domain, string userName, string password)
        {
            uint error = SuperLoggingClientLibrary.VmDirOpenServerA(
                networkAddress,
                userName,
                domain,
                password,
                0,
                IntPtr.Zero,
                out _serverContext);
            validate("openA", error);
        }

        public void openW(string networkAddress, string domain, string userName, string password)
        {
            uint error = SuperLoggingClientLibrary.VmDirOpenServerW(
                networkAddress,
                userName,
                domain,
                password,
                0,
                IntPtr.Zero,
                out _serverContext);
            validate("openW", error);
        }

        public void close()
        {
            uint error = SuperLoggingClientLibrary.VmDirCloseServer(_serverContext);
            validate("close", error);
        }

        public void enable()
        {
            uint error = SuperLoggingClientLibrary.VmDirSuperLogEnable(_serverContext);
            validate("enable", error);
        }

        public void disable()
        {
            uint error = SuperLoggingClientLibrary.VmDirSuperLogDisable(_serverContext);
            validate("disable", error);
        }

        public bool isEnabled()
        {
            byte enabled;
            uint error = SuperLoggingClientLibrary.VmDirIsSuperLogEnabled(_serverContext, out enabled);
            validate("isEnabled", error);
            return enabled == 0 ? false : true;
        }

        public void clear()
        {
            uint error = SuperLoggingClientLibrary.VmDirSuperLogFlush(_serverContext);
            validate("clear", error);
        }

        public void setCapacity(uint capacity)
        {
            uint error = SuperLoggingClientLibrary.VmDirSuperLogSetSize(_serverContext, capacity);
            validate("setCapacity", error);
        }

        public uint getCapacity()
        {
            UInt32 size = 0;
            uint error = SuperLoggingClientLibrary.VmDirSuperLogGetSize(_serverContext, out size);
            validate("getCapacity", error);
            return size;
        }

        public ISuperLogEntryList getAllEntries()
        {
            ISuperLogEntryList list;
            try
            {
                list = getPagedEntries(new SuperLoggingCookie(), 0);
            }
            catch (SuperLoggingException e)
            {
                throw new SuperLoggingException("SuperLogging getAllEntries failed", e);
            }
            return list;
        }

        public ISuperLogEntryList getPagedEntries(ISuperLoggingCookie cookie, uint pageSize)
        {
            IntPtr cookieVal = cookie.getCookieVal();
            IntPtr entries = IntPtr.Zero;
            uint error = SuperLoggingClientLibrary.VmDirSuperLogGetEntriesLdapOperation(
                _serverContext,
                cookieVal,
                pageSize,
                out entries);
            validate("getPagedEntries", error);
            return new SuperLogEntryList(entries);
        }

        public ISuperLogTable aggregate(ISuperLogEntryList entries, SuperLogTableColumnSet colSet)
        {
            IntPtr table = IntPtr.Zero;
            uint error = SuperLoggingClientLibrary.VmDirSuperLogGetTable(
                ((SuperLogEntryList)entries).getIntPtr(),
                convertColumnSetToIntPtr(colSet),
                out table
                );
            validate("aggregate", error);
            return new SuperLogTable(table);
        }

        private IntPtr convertColumnSetToIntPtr(SuperLogTableColumnSet colSet)
        {
            VMDIR_SUPERLOG_TABLE_COLUMN_SET colSetStruct = colSet.ToStruct();
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(VMDIR_SUPERLOG_TABLE_COLUMN_SET)));
            Marshal.StructureToPtr(colSetStruct, ptr, false);
            return ptr;
        }

        private void validate(string operation, uint error)
        {
            if (error != 0)
            {
                throw new SuperLoggingException(
                    "SuperLogging " + operation + " failed [error=" + error + "]");
            }
        }
    }
}
