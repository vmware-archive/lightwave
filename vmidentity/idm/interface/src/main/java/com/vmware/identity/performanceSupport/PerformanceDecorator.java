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

import java.util.concurrent.TimeUnit;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * Abstract class providing performance measurement functionality
 * scaffold for {@code task}.
 *
 */
public abstract class PerformanceDecorator
{
    private static IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(PerformanceDecorator.class);

    /**
     * Execute the {@code task} and add measured time to the {@code PerfDataSink}
     * <li> Task will always be executed, returning result or re-throwing
     * {@code RuntimeException} from execution. Other exceptions will be
     * translated to {@code IllegalStateException}</li>
     * <li> Any exception when adding measurement to performance data sink
     * will be swallowed with a logged error. </li>
     *
     * @param task              task to execute, cannot be null
     * @param aPerfDataSink     task to execute, cannot be null
     * @return                  task execution result.
     * @throws                  RuntimeException
     */
    public final <T> T exec(CallableDecorator<T> task, IPerfDataSink aPerfDataSink)
            throws RuntimeException
    {
        assert(task != null);
        assert(aPerfDataSink != null);
        long startedAt = System.nanoTime();
        T result = null;
        try {
            result = task.call();
        }catch (RuntimeException rex){
            throw rex;      // re-throw runtime exception
        }catch (Exception ex){
            // All tasks don't throw checked exception.
            // Therefore, for anything other than runtime exception, translate it.
            throw new IllegalStateException(ex);
        }

        try {
            PerfBucketKey key = task.getPerfBucketKey();
            assert key != null;

            aPerfDataSink.addMeasurement(
                    key, TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startedAt));
        }catch (Exception e){
            //swallow any performance data processing error
            log.error("Exception occurred when adding performance data", e);
        }
        return result;
    }
}
