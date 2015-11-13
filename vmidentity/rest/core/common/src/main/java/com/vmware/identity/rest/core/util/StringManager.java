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
package com.vmware.identity.rest.core.util;

import java.util.HashMap;
import java.util.Hashtable;
import java.util.Locale;
import java.util.Map;
import java.util.ResourceBundle;

/**
 * The {@code StringManager} class is an internationalization/localization helper class
 * inspired by Tomcat's {@code StringManager} class. This helps to reduce the burden of
 * dealing with {@code ResourceBundles} and can handle message formatting.
 * <p>
 * When fetching a {@code StringManager} for a specific package, it will search for a file
 * named "LocalStrings.properties" in that location. Other languages have their two-letter
 * language code appended onto the end of the file, e.g. "LocalStrings.properties_DE".
 */
public class StringManager {
    // TODO limit the cache size through eviction of the package locales.

    private static final Map<String, Map<Locale, StringManager>> managers = new Hashtable<String, Map<Locale, StringManager>>();

    private static final ThreadLocal<Locale> threadLocale = new ThreadLocal<Locale>() {
        @Override
        protected Locale initialValue() {
            return Locale.getDefault();
        }
    };

    private ResourceBundle bundle;

    private Locale locale;

    private StringManager(String packageName, Locale locale) {
        String bundleName = packageName + ".LocalStrings";
        this.bundle = ResourceBundle.getBundle(bundleName, locale);

        Locale bundleLocale = bundle.getLocale();
        if (bundleLocale.equals(Locale.ROOT)) {
            this.locale = Locale.ENGLISH;
        } else {
            this.locale = bundleLocale;
        }
    }

    /**
     * Get the locale of the {@code StringManager}.
     *
     * @return the locale of this {@code StringManager}.
     */
    public Locale getLocale() {
        return locale;
    }

    /**
     * Get a string corresponding to the property key from the LocalStrings file.
     *
     * @param key the key corresponding to the string to retrieve.
     * @return the string corresponding to the key.
     */
    public String getString(String key) {
        return bundle.getString(key);
    }

    /**
     * Get a string coresponding to the property key from the LocalStrings file
     * and perform a {@code String#format(String, Object...)} call with the supplied arguments.
     *
     * @param key the key corresponding to the string to retrieve
     * @param args the args to supply for formatting.
     * @return the formatted string corresponding to the key.
     * @see String#format(String, Object...)
     */
    public String getString(String key, Object... args) {
        return String.format(getString(key), args);
    }

    /**
     * Set the locale for a thread.
     *
     * @param locale locale to set for the thread
     */
    public static void setThreadLocale(Locale locale) {
        if (locale != null) {
            threadLocale.set(locale);
        }
    }

    /**
     * Get the locale for a thread.
     *
     * @return the locale set for this thread.
     */
    public static Locale getThreadLocale() {
        return threadLocale.get();
    }

    /**
     * Retrieve the {@code StringManager} for the supplied package using the
     * Java Virtual Machine's default locale.
     *
     * @param packageName the package name to search for the LocalStrings file.
     * @return a {@code StringManager} object for the package and default locale.
     */
    public static StringManager getManager(String packageName) {
        return getManager(packageName, Locale.getDefault());
    }

    /**
     * Retrieve the StringManager for the supplied package and locale.
     *
     * @param packageName name of the package to search for the LocalStrings file.
     * @param locale locale to find the resource for. If {@code null}, it will be
     * considered the default locale
     * @return a {@code StringManager} object for the package and locale
     */
    public static StringManager getManager(String packageName, Locale locale) {
        if (locale == null) {
            locale = Locale.getDefault();
        }

        Map<Locale, StringManager> pack = managers.get(packageName);
        if (pack == null) {
            pack = new HashMap<Locale, StringManager>();
            managers.put(packageName, pack);
        }

        StringManager manager = pack.get(locale);
        if (manager == null) {
            manager = new StringManager(packageName, locale);
            pack.put(locale, manager);
        }

        return manager;
    }

}
