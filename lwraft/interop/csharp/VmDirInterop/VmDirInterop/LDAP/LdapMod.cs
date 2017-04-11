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
using System.Collections;
using System.Runtime.InteropServices;

namespace VMDirInterop.LDAP
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct LdapModData
    {
        public int firstvalue;

        public IntPtr secondvalue;

        public IntPtr values;
    }

    public class LdapMod
    {
        public enum mod_ops
        {
            LDAP_MOD_ADD = 0x00,
            LDAP_MOD_DELETE = 0x01,
            LDAP_MOD_REPLACE = 0x02
        }

        public LdapModData lModData;

        int operationType;

        public string attributeName;

        public string[] attributeValues;

        public LdapMod()
        {
            this.lModData = new LdapModData();

            this.operationType = 0;
            this.attributeName = null;
            this.attributeValues = null;

        }

        public LdapMod(int operationType, string attributeName, string[] attributeValues)
        {
            this.lModData = new LdapModData();

            this.operationType = operationType;
            this.attributeName = attributeName;
            this.attributeValues = attributeValues;

        }

        public IntPtr convertToUnmanaged()
        {
            IntPtr zeroIntPtr = IntPtr.Zero;
            IntPtr strIntPtr = IntPtr.Zero;
            IntPtr lModDataPtr = IntPtr.Zero;

            int sizeOfIntPtr = 0;
            int counter = 0 ;
            int Length = attributeValues.Length;

            sizeOfIntPtr = Marshal.SizeOf(typeof(IntPtr));

            lModData.firstvalue = this.operationType;   /*  First Value Set*/

            IntPtr attributeNamePtr = Marshal.StringToHGlobalAnsi(attributeName);  /*  Second Value Set */

            lModData.secondvalue = attributeNamePtr;

            //
            // +1 for terminating NULL
            //
            lModData.values = Marshal.AllocHGlobal(sizeOfIntPtr * (Length + 1));

            for (counter = 0; counter < Length; counter++)
            {
                strIntPtr = Marshal.StringToHGlobalAnsi(attributeValues[counter]);
                Marshal.WriteIntPtr(this.lModData.values, counter * sizeOfIntPtr, strIntPtr);
            }

            Marshal.WriteIntPtr(this.lModData.values, counter * sizeOfIntPtr, zeroIntPtr);  /* for NULL value termination */

            //  Now  Convert to IntPtr

            lModDataPtr = Marshal.AllocHGlobal(Marshal.SizeOf (this.lModData));

            Marshal.StructureToPtr(this.lModData, lModDataPtr, false);

            return lModDataPtr;
        }

        public void Free()
        {
            if (lModData.secondvalue != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(lModData.secondvalue);
            }

            if (lModData.values != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(lModData.values);
            }
        }
    }
}
