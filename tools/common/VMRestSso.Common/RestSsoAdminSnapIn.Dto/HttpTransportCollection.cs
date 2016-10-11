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
using System.Linq;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Xml.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Dto.PropertyDescriptors;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [Serializable]
    public class HttpTransportCollection : CollectionBase, ICustomTypeDescriptor, IXmlSerializable, IDataContext
    {
        private int _capacity = 5000;

        private readonly object _mutex = new object();

        public HttpTransportCollection()
        {
        }
        public HttpTransportCollection(int capacity)
        {
            _capacity = capacity;
        }

        public String GetClassName()
        {
            return TypeDescriptor.GetClassName(this, true);
        }

        public AttributeCollection GetAttributes()
        {
            return TypeDescriptor.GetAttributes(this, true);
        }

        public String GetComponentName()
        {
            return TypeDescriptor.GetComponentName(this, true);
        }

        public TypeConverter GetConverter()
        {
            return TypeDescriptor.GetConverter(this, true);
        }

        public EventDescriptor GetDefaultEvent()
        {
            return TypeDescriptor.GetDefaultEvent(this, true);
        }

        public PropertyDescriptor GetDefaultProperty()
        {
            return TypeDescriptor.GetDefaultProperty(this, true);
        }

        public object GetEditor(Type editorBaseType)
        {
            return TypeDescriptor.GetEditor(this, editorBaseType, true);
        }

        public EventDescriptorCollection GetEvents(System.Attribute[] attributes)
        {
            return TypeDescriptor.GetEvents(this, attributes, true);
        }

        public EventDescriptorCollection GetEvents()
        {
            return TypeDescriptor.GetEvents(this, true);
        }

        public object GetPropertyOwner(PropertyDescriptor pd)
        {
            return this;
        }

        public PropertyDescriptorCollection GetProperties(System.Attribute[] attributes)
        {
            return GetProperties();
        }
        public PropertyDescriptorCollection GetProperties()
        {
            var pds = new PropertyDescriptorCollection(null);

            for (int i = 0; i < this.List.Count; i++)
            {
                var pd = new HttpTransportCollectionPropertyDescriptor(this, i);
                pds.Add(pd);
            }
            return pds;
        }
        public void Add(HttpTransportItem dto)
        {
            lock (_mutex)
            {
                if (this.List.Count >= _capacity)
                    this.List.RemoveAt(0);
                this.List.Add(dto);
            }
        }
        public void Remove(HttpTransportItem dto)
        {
            lock (_mutex)
            {
                this.List.Remove(dto);
            }
        }
        public HttpTransportItem this[int index]
        {
            get
            {
                return (HttpTransportItem)this.List[index];
            }
        }

        public System.Xml.Schema.XmlSchema GetSchema()
        {
            return null;
        }
        public void ReadXml(System.Xml.XmlReader reader)
        {
            var serializer = new XmlSerializer(typeof(HttpTransportItem));
            var archiveList = new List<HttpTransportItem>();

            bool wasEmpty = reader.IsEmptyElement;
            reader.Read();

            if (wasEmpty)
                return;
            while (reader.NodeType != System.Xml.XmlNodeType.EndElement)
            {
                reader.ReadStartElement("item");
                var value = (HttpTransportItem)serializer.Deserialize(reader);
                reader.ReadEndElement();
                this.List.Add(value);
                reader.MoveToContent();
            }
            reader.ReadEndElement();
        }
        public void WriteXml(System.Xml.XmlWriter writer)
        {
            var seralizer = new XmlSerializer(typeof(HttpTransportItem));
            int counter = 1;
            foreach (HttpTransportItem item in this.List)
            {
                writer.WriteStartElement("item");
                seralizer.Serialize(writer, item);
                writer.WriteEndElement();

                // Restrict size to capacity in case someone changes the persisted xml by hand.
                if (++counter > _capacity)
                    break;
            }
        }

        public void Sort(SortingOrder order)
        {
            var items = new List<HttpTransportItem>();
            foreach (HttpTransportItem item in this.List)
            {
                items.Add(item);
            }

            var result = new List<HttpTransportItem>();
            if (order == SortingOrder.Ascending)
                result = items.OrderBy(x => x.RequestTimestamp).ToList();
            else
                result = items.OrderByDescending(x => x.RequestTimestamp).ToList();
            this.List.Clear();
            foreach (HttpTransportItem item in result)
            {
                this.List.Add(item);
            }
        }

		public IList<HttpTransportItem> Items { get { 
			
				var items = new List<HttpTransportItem>();
				foreach (HttpTransportItem item in this.List)
				{
					items.Add(item);
				}
				return items;
			} }
    }

    public enum SortingOrder
    {
        Ascending,
        Descending
    }
}
