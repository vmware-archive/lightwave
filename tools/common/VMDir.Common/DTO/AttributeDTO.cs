/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
using VMDir.Common.Schema;

namespace VMDir.Common.DTO
{
    public class AttributeDTO
    {
        public string Name { get; set; }
        public string Value { get; set; }
        public AttributeTypeDTO AttrSyntaxDTO { get; set; }
		public bool Dirty { get; set; }
        public AttributeDTO(string name, string value, AttributeTypeDTO attrSyntaxDTO)
        {
            this.Name = name;
            this.Value = value;
            this.AttrSyntaxDTO = attrSyntaxDTO;
        }
		public AttributeDTO(string name, string value, AttributeTypeDTO attrSyntaxDTO, bool dirty)
		{
			this.Name = name;
			this.Value = value;
			this.AttrSyntaxDTO = attrSyntaxDTO;
			this.Dirty = dirty;
		}
    }
}
