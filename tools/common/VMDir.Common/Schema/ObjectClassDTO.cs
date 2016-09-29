/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System.Collections.Generic;
using System;

namespace VMDir.Common.Schema
{
    public class ObjectClassDTO
    {
        public enum ObjectClassType
        {
            Structural = 1,
            Abstract = 2,
            Auxiliary = 3
        }

        public ObjectClassType ClassType { get; set; }

        //Name or cn
        public string Name { get; set; }

        public string GovernsID { get; set; }
        //Optional description
        public string Description { get; set; }

        //subclassof
        public string SuperClass { get; set; }

        public List<string> Must { get; set; }

        public List<string> May { get; set; }

        public List<string> Aux { get; set; }

        public List<string> ObjectClass { get; set; }

        public int GetObjectClassType()
        {
            return (int)ClassType;
        }

        public string GetObjectClassTypeAsString()
        {
            return ClassType.ToString();
        }
    }
}
