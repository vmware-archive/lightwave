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

using System;
using System.ComponentModel;

namespace Vmware.Tools.RestSsoAdminSnapIn.HttpTransport
{
    public class SsoHttpTransportCollectionPropertyDescriptor: PropertyDescriptor
    {
        private readonly SsoHttpTransportCollection _collection;
        private readonly int _index;
        public SsoHttpTransportCollectionPropertyDescriptor(SsoHttpTransportCollection coll, int idx) : base( "#"+idx, null )
        {
            _collection = coll;
            _index = idx;
        }
        public override AttributeCollection Attributes
        {
            get
            {
                return new AttributeCollection(null);
            }
        }
        public override bool CanResetValue(object component)
        {
            return true;
        }
        public override Type ComponentType
        {
            get
            {
                return _collection.GetType();
            }
        }
        public override string DisplayName
        {
            get
            {
                SsoHttpTransport tran = _collection[_index];
                return tran.Method + " (" + tran.TimeTaken + ")";
            }
        }
        public override string Description
        {
            get
            {
                return "";
            }
        }
        public override object GetValue(object component)
        {
            return _collection[_index];
        }
        public override bool IsReadOnly
        {
            get { return true;  }
        }
        public override string Name
        {
            get { return "#"+_index; }
        }
        public override Type PropertyType
        {
            get { return _collection[_index].GetType(); }
        }
        public override void ResetValue(object component) {}
        public override bool ShouldSerializeValue(object component)
        {
            return true;
        }
        public override void SetValue(object component, object value)
        {
        }
    }
}
