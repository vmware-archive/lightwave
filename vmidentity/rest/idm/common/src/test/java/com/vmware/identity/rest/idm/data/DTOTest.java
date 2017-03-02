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
package com.vmware.identity.rest.idm.data;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.Set;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;
import org.reflections.Reflections;

import com.fasterxml.jackson.core.JsonParseException;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.vmware.identity.rest.core.data.DTO;
import com.vmware.identity.rest.core.util.ObjectMapperSingleton;

@RunWith(Parameterized.class)
public class DTOTest {

    private static final String PACKAGE_LOCATION = "com.vmware.identity.rest.idm.data";
    private static final String SERIALIZED = "{ \"unknownField\": \"randomjunk\" }";

    private final Class<? extends DTO> klass;

    public DTOTest(Class<? extends DTO> klass) {
        this.klass = klass;
    }

    @Parameters(name="{0}")
    public static Set<Class<? extends DTO>> getClasses() {
        Reflections reflections = new Reflections(PACKAGE_LOCATION);
        return reflections.getSubTypesOf(DTO.class);
    }

    @Test
    public void testDeserialization_UnknownField() throws JsonParseException, JsonMappingException, IOException {
        ObjectMapper om = ObjectMapperSingleton.getInstance();
        om.readValue(SERIALIZED, klass);
    }

    @Test
    public void testSerialization_EmptyFields() throws InstantiationException, IllegalAccessException, IllegalArgumentException, InvocationTargetException, JsonProcessingException {
        ObjectMapper om = ObjectMapperSingleton.getInstance();
        Constructor<?>[] ctors = klass.getDeclaredConstructors();

        Constructor<?> ctor = ctors[0];
        Object[] initargs = new Object[ctor.getGenericParameterTypes().length];
        Object dto = ctor.newInstance(initargs);


        String str = om.writeValueAsString(dto);
        assertEquals("{}", str);
    }

}
