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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * This implementation processes the performance data, writes it
 * to log file, and provides two internal reporters for
 * dumping metrics summary to log file.
 *
 */
public final class PerfDataSink implements IPerfDataSink
{
    private static IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(PerfDataSink.class);

    private final ConcurrentMap<PerfBucketKey, PerfBucketMetrics> metricsFromKey
        = new ConcurrentHashMap<PerfBucketKey, PerfBucketMetrics>();

    private Thread timer;
    private AtomicInteger dataEntryCount = new AtomicInteger();
    private long hitCountInterval;
    private long millisInterval;

    MetricsReporter hitCountReporter =
            new MetricsReporter("HitCountReporter");
    MetricsReporter timeIntervalReporter =
            new MetricsReporter("TimeIntervalReporter");

    class MetricsReporter {
        private final String name;

        MetricsReporter(String aName) {
            assert aName != null;

            this.name = aName;
        }

        void reportSummary()
        {
            log.info(String.format("Performance metrics summary provided by %s", name));
            Map<PerfBucketKey, PerfBucketMetrics> map = PerfDataSink.this.metricsFromKey;
            // Note: keys will only be added and not removed.
            List<PerfBucketKey> keys = new ArrayList<PerfBucketKey>(map.keySet());
            Collections.sort(keys);
            for (PerfBucketKey key : keys)
            {
                log.info(key);
                log.info(String.format("\t: %s", map.get(key).toString()));
            }
        }

        @Override
        public String toString()
        {
            return name;
        }
    }

    /**
     *
     * @param aHitCountInterval  Triggering reportSummary dump by total number
     *                           of measurements received periodically
     * @param aMinutesInterval   Triggering reportSummary dump by time interval
     *                           specified in minute. This will be translated
     *                           to millis internally.
     */
    public PerfDataSink(long aHitCountInterval, long aMinutesInterval)
    {
        Validate.isTrue(aHitCountInterval > 0);
        Validate.isTrue(aMinutesInterval > 0);

        log.info("restarting PerfDataSink.");

        hitCountInterval = aHitCountInterval;
        millisInterval = TimeUnit.MINUTES.toMillis(aMinutesInterval);

        this.timer = new Thread(new Runnable() {

            @Override
            public void run()
            {
                do {
                    try {
                        Thread.sleep(millisInterval);
                        timeIntervalReporter.reportSummary();
                    }
                    catch (InterruptedException e)
                    {
                       log.info("thread exiting: {}", timeIntervalReporter);
                       break;
                    }
                }while (true);
            }
        });

        timer.start();
    }

    @Override
    public void addMeasurement(PerfBucketKey key, long value)
    {
       if (key != null && value >= 0) {
          // log the new entry in perf logger
          log.info(String.format("%s, ms=%d", key, value));

          // Metrics entries in the map are never deleted.
          // Update the metrics with concurrency in mind.
          PerfBucketMetrics metrics = metricsFromKey.get(key);
          if (metrics != null) {
             metrics.addMeasurement(value);
          } else {
             // no entry at this point
             if (null != metricsFromKey.putIfAbsent(key, new PerfBucketMetrics(value))) {
                // another thread has sneaked in before us
                metricsFromKey.get(key).addMeasurement(value);
             }
          }

          if ((dataEntryCount.incrementAndGet() % hitCountInterval) == 0) {
             hitCountReporter.reportSummary();
          }
       } else {
          log.error(String.format("Problem with performance measurement. Key: %s, ms=%d", key, value));
       }
    }
}
