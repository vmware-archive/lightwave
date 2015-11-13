/*
 *
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
 *
 */
package com.vmware.identity.performanceSupport;

import org.apache.commons.lang.Validate;

/**
 * Maintains performance metrics of a measurement point
 *
 */
public class PerfBucketMetrics
{
    private int hits;
    private long totalMs;
    private long ceilingMs;            // the largest value, usually due to initial load
    private long effectiveCeilingMs;   // the second largest value
    private long floorMs;

    /**
     * c'tor of a metric object with first data entry
     * @param value date entry >=0
     */
    public PerfBucketMetrics(long value)
    {
        Validate.isTrue(value >= 0, Long.toString(value));
        hits = 1;
        totalMs = value;
        ceilingMs = value;
        effectiveCeilingMs = 0;
        floorMs   = value;
    }

    /**
     * Add measurement to the metrics. Thread safe.
     *
     * @param value New data entry, >=0
     */
    public synchronized void addMeasurement(long value)
    {
        Validate.isTrue(value >= 0, Long.toString(value));
        if (Long.MAX_VALUE - totalMs < value)
        {
            reset(); //overflow
        }
        ++hits;
        totalMs += value;
        if (value > ceilingMs)
        {
            effectiveCeilingMs = ceilingMs;
            ceilingMs = value;
        }
        else if (value > effectiveCeilingMs)
        {
            effectiveCeilingMs = value;
        }
        if (value < floorMs)
        {
            floorMs = value;
        }
    }

    private void reset()
    {   // shrink {hits, totalsMs} by 2^8 to reclaim capacity
        // while still have room to exclude the ceiling value when
        // calculating adjustAvg.
        hits = hits>>8 + 1;
        totalMs = totalMs>>8;
        //compensate the ceiling value so that the adjustAvg is exactly as before:
        // 1. deduct the ceilingMs portion in the shrinked data
        // 2. add back the original value of ceilingMs
        totalMs -= ceilingMs >> 8;
        totalMs += ceilingMs;
        // Leaving ceiling & floor unchanged
    }

    @Override
    public String toString()
    {
        long adjustedAvg = ceilingMs;  // average excluding the ceiling value due to initial load
        if (hits > 1)
        {
            adjustedAvg = (totalMs - ceilingMs) / (hits - 1);
        }
        return "PerfBucketMetrics [hits=" + hits
                + ", totalMs=" + totalMs
                + ", ceilingMs=" + ceilingMs
                + ", effectiveCeilingMs=" + effectiveCeilingMs
                + ", floorMs=" + (hits==0? 0:floorMs)
                + ", adjustedAvg=" + adjustedAvg + "]";
    }
}
