/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap;

import java.util.Arrays;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.vmware.identity.interop.NativeMemory;

/**
 * Do not use this class externally.
 * It is intended for internal use by Platform package,
 * but must stay public for interop ...
 */

public final class BerValNativeArray extends Structure implements AutoCloseable
{
    private static Log log = LogFactory.getLog(BerValNativeArray.class);
    public Pointer[] pBerVals;
    private NativeMemory[] memoryBlocks;//hold on to the allocated native memory

    public BerValNativeArray(LdapValue[] values)
    {
        if (values == null || values.length <= 0)
        {
            throw new IllegalArgumentException("Missing attribute values");
        }

        pBerVals = new Pointer[values.length + 1];
        memoryBlocks = new NativeMemory[values.length];

        int iVal = 0;

        if (log.isTraceEnabled())
        {
            log.trace("--adding LdapValue[] attributes to native peer");
        }
        for (LdapValue val : values)
        {
            int length = 0;

            byte[] contents = val.getValue();

            if (contents != null && contents.length > 0)
            {
                length = contents.length;

                memoryBlocks[iVal] = new NativeMemory(length);

                memoryBlocks[iVal].write(0, contents, 0, length);
            }

            BerValNative berVal = new BerValNative(length, memoryBlocks[iVal]);

            pBerVals[iVal] = berVal.getPointer();

//comment out tracing, uncommented when needed.
//            if (log.isTraceEnabled())
//            {
//                int hdrBufLen = 16;  /*byte[0~7]: contentLen; byte[8~0xf]: ptr to contentBuffer*/
//                byte[] hdrBuf = new byte[hdrBufLen];
//                berVal.getPointer().read(0, hdrBuf, 0, hdrBufLen);
//                StringBuilder hdrData = new StringBuilder();
//                for (byte b : hdrBuf)
//                {
//                    hdrData.append(String.format(" %02x", b));
//                }
//                log.trace(String.format("header: %s", hdrData));
//
//                byte[] buf = new byte[length];
//                memoryBlocks[iVal].read(0, buf, 0, length);
//                String prefix = String.format("payload %s", memoryBlocks[iVal].toString());
//                StringBuilder sb = new StringBuilder();
//                for (byte b: buf)
//                {
//                    sb.append(String.format(" %02x", b));
//                }
//                log.trace(String.format("%s: [%s]", prefix, sb.toString()));
//            }
            iVal++;
        }

        pBerVals[iVal] = Pointer.NULL;

        write();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "pBerVals"
        });
    }


    @Override
    public void close()
    {
        if ( this.memoryBlocks != null )
        {
            for(int i = 0; i < this.memoryBlocks.length; i++)
            {
                if ( this.memoryBlocks[i] != null )
                {
                    this.memoryBlocks[i].close();
                    this.memoryBlocks[i] = null;
                }
            }
            this.memoryBlocks = null;
        }
    }
}
