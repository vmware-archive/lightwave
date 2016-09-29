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
using System.Linq;
using System.Text;
using System.ComponentModel;

namespace VMCASnapIn.UI
{
    public class BagItem
    {
        public object Value { get; set; }
        public bool IsReadOnly { get; set; }
        public bool IsRequired { get; set; }
        public string Description { get; set; }
    }
    //Provides a way to create a property grid friendly dynamic key value bag.
    [TypeDescriptionProvider(typeof(KeyValueBagTypeDescriptionProvider))]
    public class KeyValueBag : Dictionary<string, BagItem>
    {
        public long Ticks = DateTime.Now.Ticks;
    }

    //With caching so the multitude of GetTypeDescriptor calls do not keep creating
    //new objects. Guid might be better than ticks.
    public class KeyValueBagTypeDescriptionProvider : TypeDescriptionProvider
    {
        private static TypeDescriptionProvider defaultProvider = TypeDescriptor.GetProvider(typeof(KeyValueBag));
        Dictionary<long, KeyValueBagTypeDescriptor> _providerCache = new Dictionary<long, KeyValueBagTypeDescriptor>();

        public KeyValueBagTypeDescriptionProvider() : base(defaultProvider) { }

        public override ICustomTypeDescriptor GetTypeDescriptor(Type objectType, object instance)
        {
            ICustomTypeDescriptor defaultDescriptor = base.GetTypeDescriptor(objectType, instance);
            if (instance == null)
                return defaultDescriptor;
            var bag = instance as KeyValueBag;
            if (bag == null)
                return defaultDescriptor;
            KeyValueBagTypeDescriptor descriptor = null;
            if (_providerCache.TryGetValue(bag.Ticks, out descriptor))
            {
                if (descriptor.GetProperties().Count == bag.Count)
                    return descriptor;
                else
                    _providerCache.Remove(bag.Ticks);
            }
            descriptor = new KeyValueBagTypeDescriptor(defaultDescriptor, instance);
            _providerCache[bag.Ticks] = descriptor;
            return descriptor;
        }
    }

    class KeyValueBagTypeDescriptor : CustomTypeDescriptor
    {
        PropertyDescriptorCollection _descriptorCollection;
        public KeyValueBagTypeDescriptor(ICustomTypeDescriptor parent, object instance)
            : base(parent)
        {
            var bag = instance as KeyValueBag;
            var propDescriptorArray = bag.Select(x => new KeyValuePropertyDescriptor(x)).ToArray();
            _descriptorCollection = new PropertyDescriptorCollection(propDescriptorArray);
        }

        public override PropertyDescriptorCollection GetProperties()
        {
            return _descriptorCollection;
        }

        public override PropertyDescriptorCollection GetProperties(Attribute[] attributes)
        {
            return _descriptorCollection;
        }
    }

    class KeyValuePropertyDescriptor : PropertyDescriptor
    {
        Type _type;
        bool _readOnly = false;
        public bool IsRequired { get; protected set; }

        public KeyValuePropertyDescriptor(KeyValuePair<string, BagItem> entry)
            : base(entry.Key, new Attribute[] { new DescriptionAttribute(entry.Value.Description) })
        {
            var item = entry.Value;
            _readOnly = item.IsReadOnly;
            _type = item.Value.GetType();
            IsRequired = entry.Value.IsRequired;
        }

        public override bool CanResetValue(object component) { return false; }
        public override Type ComponentType { get { return typeof(KeyValueBag); } }
        public override bool IsReadOnly { get { return _readOnly; } }
        public override Type PropertyType { get { return _type; } }
        public override bool ShouldSerializeValue(object component) { return false; }

        public override void ResetValue(object component)
        {
            var bag = component as KeyValueBag;
            bag[Name].Value = GetDefaultValue(_type);
        }

        public override object GetValue(object component)
        {
            var bag = component as KeyValueBag;
            return bag[Name].Value;
        }

        public override void SetValue(object component, object value)
        {
            var bag = component as KeyValueBag;
            bag[Name].Value = value;
        }

        public object GetDefaultValue(Type t)
        {
            if (t.IsValueType)
            {
                return Activator.CreateInstance(t);
            }
            else
            {
                return null;
            }
        }
    }
}
