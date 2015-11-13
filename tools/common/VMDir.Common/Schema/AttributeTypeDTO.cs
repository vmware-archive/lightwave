/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

namespace VMDir.Common.Schema
{
    public class AttributeTypeDTO
    {
        public string Name { get; set; }

        public string Description { get; set; }

        public string SyntaxName { get; set; }

        public string Type { get; set; }

        public bool ReadOnly { get; set; }

        public bool SingleValue { get; set; }

        public int Length { get; set; }
    }
}
