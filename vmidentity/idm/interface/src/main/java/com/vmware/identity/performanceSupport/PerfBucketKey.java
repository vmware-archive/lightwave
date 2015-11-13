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

import java.net.URI;
import java.net.URISyntaxException;

import org.apache.commons.lang.Validate;

/**
 * Define the bucket (measurement point) used to track performance metrics.
 * Besides {@code PerfMeasurementInterface} and {@code PerfMeasurementTransaction},
 * {@code providerInfo} is also used for data bucketing since network connection
 * should have an impact on measurements.
 *
 */
public final class PerfBucketKey implements Comparable<PerfBucketKey>
{
    private static final String PROVIDER_INFO_NA = "PROVIDERID-NA";
    private static final String PROVIDER_INFO_DEFAULT = "PROVIDERID-DEFAULT";

    private final PerfMeasurementPoint measurementPt;
    private final String providerInfo;  // domain name, or domain ID provider's uri

    /**
     * c'tor with fully specified parameters
     * @param pt    specified the point of the measurement, cannot be null.
     * @param idStr either an upn which will be parsed to get the domain name,
     *              or a domain's identity provider uri such as ldap://localhost:11711.
     *              Can be null, which will be translated to {@code PROVIDER_INFO_NA}.
     */
    public PerfBucketKey(PerfMeasurementPoint pt, String idStr)
    {
        Validate.notNull(pt);

        measurementPt = pt;
        providerInfo = (idStr == null ?
                PROVIDER_INFO_NA.toUpperCase() :
                    extractProviderInfo(idStr).toUpperCase());
    }

    /**
    * c'tor with default value for {@code providerInfo}
     * @param pt    specified the point of the measurement, cannot be null.
    */
    public PerfBucketKey(PerfMeasurementPoint pt)
    {
        this(pt, PROVIDER_INFO_DEFAULT);
    }

    /**
     * Getter
     * @return
     */
    public PerfMeasurementPoint getMeasurementPoint()
    {
        return measurementPt;
    }

    /**
     * Getter
     * @return
     */
    public String getProviderInfo()
    {
        return providerInfo;
    }

    @Override
    public String toString()
    {
        return "PerfBucketKey [measurementPt=" + measurementPt
                + ", providerInfo=" + providerInfo + "]";
    }

    @Override
    public int hashCode()
    {
        final int prime = 31;
        int result = 1;
        result =
                prime * result + providerInfo.hashCode();
        result =
                prime * result +  measurementPt.hashCode();
        return result;
    }

    @Override
    public boolean equals(Object obj)
    {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        PerfBucketKey other = (PerfBucketKey) obj;
        if (!providerInfo.equalsIgnoreCase(other.providerInfo))
            return false;
        if (measurementPt != other.measurementPt)
            return false;
        return true;
    }

    @Override
    public int compareTo(PerfBucketKey arg0)
    {
        if (measurementPt.getItf() != arg0.getMeasurementPoint().getItf())
            return measurementPt.getItf().compareTo(
                    arg0.getMeasurementPoint().getItf());
        else if (measurementPt.getTxId() != arg0.getMeasurementPoint().getTxId())
            return this.measurementPt.getTxId().compareToIgnoreCase(
                    arg0.measurementPt.getTxId());
        else
            return this.providerInfo.compareToIgnoreCase(arg0.providerInfo);
    }

    private String extractProviderInfo(String idStr)
    {
        assert idStr != null;
        /**
         * figure out the provider info from the following format:
         * <li> someone@example.com </li>
         * <li> example.com\someone </li>
         * <li> domain ID provider's URI </li>
         **/
       if (idStr.contains("@"))
       {
          return idStr.substring(idStr.indexOf('@')+1);
       }
       else if (idStr.contains("\\"))
       {
          return idStr.substring(0, idStr.indexOf("\\"));
       }
       else
       {
           try {
               new URI(idStr);
               return idStr;
           }
           catch (URISyntaxException e)
           {
              return "unknowDomain";
           }
       }
    }
}
