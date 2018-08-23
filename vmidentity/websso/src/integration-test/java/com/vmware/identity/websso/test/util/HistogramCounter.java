/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.websso.test.util;

public class HistogramCounter {
  String name;
  long valueMin;
  long valueMax;
  int numIntervals;
  int intervalSize;
  long frequency[];
  long successCount = 0;
  long failureCount = 0;

  private HistogramCounter(
    String name, int valueMin, int valueMax, int intervalSize
  ) {
    this.name = name; this.valueMin = valueMin; this.valueMax = valueMax; this.intervalSize = intervalSize;
    // Create an array of frequency values to keep count of results in
    // each interval
    //     + 1 for all values less than valueMin and 1 for all values >
    // valueMax
    // The idea of this class is to get a set of buckets of where the
    // latency value lies.
    // For eg. Having min 0, max 1000 millseconds and interval as 100
    // we will find what is the common latency time whether it is b/w
    // 200-300 ms or 500-600ms
    this.numIntervals = (valueMax + 1 - valueMin) / intervalSize + 2;
    frequency = new long[numIntervals];
  }

  public static HistogramCounter createCounter(String name) {
    return new HistogramCounter(name, 0, 1000, 100);
  }

  public void addCounterValues(HistogramCounter h) {
    for (int i = 0; i < numIntervals; ++i) {
      frequency[i] += h.frequency[i];
    }
  }

  public long getSuccessCount() {
    return successCount;
  }

  public long getFailureCount() {
    return failureCount;
  }

  public long getTotal() {
    long total = 0; for (int i = 0; i < numIntervals; ++i) {
      total += frequency[i];
    }
    return total;
  }

  public void addValue(long value) {
    if (value < valueMin) {
      frequency[0]++;
    } else if (value > valueMax) {
      frequency[numIntervals - 1]++;
    } else {
      int index = (int) ((value - valueMin) / intervalSize) + 1; frequency[index]++;
    }
  }

  public static void addSuccessValue(HistogramCounter counter, long value) {
    if (counter != null) {
      counter.addValue(value);
      counter.incSuccessCount();
    }
  }

  public static void addFailureValue(HistogramCounter counter, long value) {
    if (counter != null) {
      counter.addValue(value);
      counter.incFailureCount();
    }
  }

  public void incSuccessCount() {
    ++successCount;
  }

  public void incFailureCount() {
    ++failureCount;
  }

  public void reset() {
    frequency = new long[numIntervals];
  }

  @Override public String toString() {
    StringBuilder sb = new StringBuilder(); sb.append("CounterName: " + name);
    sb.append(String.format("[<%d]=%d\n", valueMin, frequency[0]));
    for (int i = 1; i < numIntervals - 1; ++i) {
      sb.append(
        String.format("[%d-%d]=%d\n", valueMin + (i - 1) * intervalSize, valueMin + i * intervalSize, frequency[i]));
    }
    sb.append(String.format("[>%d]=%d\n", valueMax, frequency[numIntervals - 1]));
    return sb.toString();
  }

  public String toCSV() {
    long total = 0;
    StringBuilder sb = new StringBuilder();
    sb.append(frequency[0] + ",");
    total += frequency[0];
    for (int i = 1; i < numIntervals - 1; ++i) {
      sb.append(frequency[i] + ",");
      total += frequency[i];
    }
    sb.append(frequency[numIntervals - 1] + ",");
    total += frequency[numIntervals - 1];
    sb.append(total + ",");
    sb.append(successCount + ",");
    sb.append(failureCount);
    return sb.toString();
  }
}

