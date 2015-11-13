/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.idm.client;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;
import org.junit.internal.runners.InitializationError;
import org.junit.internal.runners.JUnit4ClassRunner;

public class OrderedRunner extends JUnit4ClassRunner
{
    private static final Logger _logger = Logger.getLogger(OrderedRunner.class);

    public OrderedRunner(Class<?> klass) throws InitializationError {
        super(klass);
    }

    protected List<Method> getTestMethods()
    {
        List<Method> list = super.getTestMethods();
        _logger.info("=========================OrderedRunner=========================");
        _logger.info("=               Got the following methods order:              =");
        _logger.info("=========================OrderedRunner=========================");
        printMethodsList(list);
        _logger.info("");
        List<Method> copy = new ArrayList<Method>(list);
        checkOrder(list);
        Collections.sort(copy, new Comparator<Method>() {
            @Override
            public int compare(Method arg0, Method arg1)
            {
                if ( arg0 == null )
                {
                    throw new RuntimeException("Arg0 is null!");
                }
                if ( arg1 == null )
                {
                    throw new RuntimeException("Arg1 is null!");
                }

                int o1Order = getOrder(arg0);
                int o2Order = getOrder(arg1);
                if( o1Order == o2Order )
                {
                    return 0;
                }
                else if(o1Order > o2Order)
                {
                    return 1;
                }
                else
                {
                    return -1;
                }
            }
        });
        _logger.info("=========================OrderedRunner=========================");
        _logger.info("=            Will run the following methods order:            =");
        _logger.info("=========================OrderedRunner=========================");
        printMethodsList(copy);
        _logger.info("");

        return copy;
    }

    private static void checkOrder(List<Method> list)
    {
        Map<Integer, String> orders = new HashMap<Integer, String>(list.size());

        for(int i = 0; i < list.size(); i ++)
        {
            Method m = list.get(i);
            TestOrderAnnotation annotation = m.getAnnotation(TestOrderAnnotation.class);
            if ( annotation != null )
            {
                Integer order = annotation.order();
                // require all new test methods to be order independent!
                if ( ( order < 0 ) || ( order > 62 ) )
                {
                    _logger.error( String.format("All new methods require to be order independent! Test=[%s], order=[%s]", m.getName(), order) );
                    throw new RuntimeException( String.format("All new methods require to be order independent! Test=[%s], order=[%s]", m.getName(), order) );
                }

                if ( orders.containsKey(order) )
                {
                    _logger.error( String.format("Another test method [%s] has the same order [%s] set as the current one [%s]", orders.get(order), order, m.getName()) );
                    throw new RuntimeException( String.format("Another test method [%s] has the same order [%s] set as the current one [%s]", orders.get(order), order, m.getName()) );
                }
                else
                {
                    orders.put(order, m.getName());
                }
            }
        }
    }

    private static void printMethodsList(List<Method> list)
    {
        for(int i = 0; i < list.size(); i ++)
        {
            Method m = list.get(i);
            TestOrderAnnotation annotation = m.getAnnotation(TestOrderAnnotation.class);
            Integer order = null;
            if ( annotation != null )
            {
                order = annotation.order();
            }
            _logger.info(String.format("%s [order=%s]", m.getName(), ((order != null) ? order.toString() : "(UNSET)" ) ));
        }
    }

    private static int getOrder(Method m)
    {
        int order = 0;
        TestOrderAnnotation annotation = m.getAnnotation(TestOrderAnnotation.class);
        if ( annotation != null )
        {
            order = annotation.order();
        }
        else
        {
            order = Integer.MAX_VALUE;
        }
        return order;
    }
}
