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

package com.vmware.identity.openidconnect.server;

import java.util.Date;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map.Entry;

import org.apache.commons.lang3.Validate;
import org.apache.commons.lang3.tuple.Pair;

/**
 * @author Yehia Zayour
 */
public class SlidingWindowMap<K, V> {
    public interface TimeProvider {
        public Date getCurrentTime();
    }

    private static class DefaultTimeProvider implements TimeProvider {
        @Override
        public Date getCurrentTime() {
            return new Date();
        }
    }

    private final long widthMilliseconds;
    private final TimeProvider timeProvider;
    private final LinkedHashMap<K, Pair<V, Date>> map;

    public SlidingWindowMap(long widthMilliseconds) {
        this(widthMilliseconds, new DefaultTimeProvider());
    }

    public SlidingWindowMap(long widthMilliseconds, TimeProvider timeProvider) {
        Validate.isTrue(widthMilliseconds > 0, "widthMilliseconds should be a positive value");
        Validate.notNull(timeProvider, "timeProvider");

        this.widthMilliseconds = widthMilliseconds;
        this.timeProvider = timeProvider;
        this.map = new LinkedHashMap<K, Pair<V, Date>>();
    }

    public void add(K key, V value) {
        Validate.notNull(key, "key");
        Validate.notNull(value, "value");

        if (this.map.containsKey(key)) {
            throw new IllegalArgumentException("specified key is already in the map: " + key.toString());
        }

        Date now = this.timeProvider.getCurrentTime();
        cleanUp(now);
        this.map.put(key, Pair.of(value, now));
    }

    public V remove(K key) {
        Validate.notNull(key, "key");

        Date now = this.timeProvider.getCurrentTime();
        cleanUp(now);
        Pair<V, Date> pair = this.map.remove(key);
        return (pair == null) ? null : pair.getLeft();
    }

    public V get(K key) {
        Validate.notNull(key, "key");

        Date now = this.timeProvider.getCurrentTime();
        cleanUp(now);
        Pair<V, Date> pair = this.map.get(key);
        return (pair == null) ? null : pair.getLeft();
    }

    private void cleanUp(Date now) {
        // the oldest items are at the head of the queue, as long as we see expired items we continue removing
        Iterator<Entry<K, Pair<V, Date>>> iterator = this.map.entrySet().iterator();
        while (iterator.hasNext()) {
            Entry<K, Pair<V, Date>> entry = iterator.next();
            Date insertionDate = entry.getValue().getRight();
            if (now.after(new Date(insertionDate.getTime() + this.widthMilliseconds))) {
                iterator.remove();
            } else {
                break;
            }
        }
    }
}