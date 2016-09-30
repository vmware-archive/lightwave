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
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto.PropertyDescriptors
{
    public class HttpTransportCollectionPropertyDescriptor : PropertyDescriptor
    {
        private HttpTransportCollection collection = null;
        private int index = -1;

        public HttpTransportCollectionPropertyDescriptor(HttpTransportCollection coll, int idx): base("#" + idx.ToString(), null)
        {
            this.collection = coll;
            this.index = idx;
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
                return this.collection.GetType();
            }
        }

        public override string DisplayName
        {
            get
            {
                HttpTransportItem tran = this.collection[index];
                return tran.RequestUri + " (" + tran.RequestTimestamp + ")";
            }
        }

        public override string Description
        {
            get
            {
                 HttpTransportItem tran = this.collection[index];
                 return string.Format("{0} method called on {1} endpoint at {2}. The total time taken is {3}", tran.Method, tran.RequestUri, tran.RequestTimestamp, tran.TimeTaken);
            }
        }

        public override object GetValue(object component)
        {
            return this.collection[index];
        }

        public override bool IsReadOnly
        {
            get { return true; }
        }

        public override string Name
        {
            get { return "#" + index.ToString(); }
        }

        public override Type PropertyType
        {
            get { return this.collection[index].GetType(); }
        }

        public override void ResetValue(object component) { }

        public override bool ShouldSerializeValue(object component)
        {
            return true;
        }

        public override void SetValue(object component, object value)
        {
            // this.collection[index] = value;
        }
    }
}
