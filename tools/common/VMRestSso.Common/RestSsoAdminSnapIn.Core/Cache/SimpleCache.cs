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
using System.Xml;
using System.Xml.Schema;
using System.Xml.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Cache
{
    public class SimpleCache<T> : IXmlSerializable where T : class 
    {
        private IDictionary<string, T> _store;
        private const long Capacity = 100000;
        private long _size;
        private readonly object _mutex = new object();

        public SimpleCache()
        {
            _store = new Dictionary<string, T>();
        }

        public void Add(string key, T value)
        {
            if (_size < Capacity)
            {
                lock (_mutex)
                {
                    if (_size < Capacity)
                    {
                        _store.Add(key, value);
                        _size++;
                    }
                }
            }
            else
            {
                throw new Exception("SimpleCache full");
            }
        }

        public bool Remove(string key)
        {
            var success = false;
            if (_store.ContainsKey(key))
            {
                lock (_mutex)
                {
                    if (_store.ContainsKey(key))
                    {
                        _store.Remove(key);
                        _size--;
                        success = true;
                    }
                }
            }

            return success;
        }

        public T Get(string key)
        {
            T value;
            _store.TryGetValue(key, out value);
            return value;
        }

        public ICollection<T> GetAll()
        {
            return _store.Values;
        }

        public ICollection<string> GetKeys()
        {
            return _store.Keys;
        }

        public void Clear()
        {
            _store = new Dictionary<string, T>();
        }
        XmlSchema IXmlSerializable.GetSchema()
        {
            return null;
        }

        public void ReadXml(System.Xml.XmlReader reader)
        {
            var keySerializer = new XmlSerializer(typeof(string));
            var valueSerializer = new XmlSerializer(typeof(T));

            bool wasEmpty = reader.IsEmptyElement;
            reader.Read();

            if (wasEmpty)
                return;

            while (reader.NodeType != System.Xml.XmlNodeType.EndElement)
            {
                reader.ReadStartElement("item");

                reader.ReadStartElement("key");
                var key = (string)keySerializer.Deserialize(reader);
                reader.ReadEndElement();

                reader.ReadStartElement("value");
                var value = (T)valueSerializer.Deserialize(reader);
                reader.ReadEndElement();

                this.Add(key, value);

                reader.ReadEndElement();
                reader.MoveToContent();
            }
            reader.ReadEndElement();
        }

        public void WriteXml(System.Xml.XmlWriter writer)
        {
            var keySerializer = new XmlSerializer(typeof(string));
            var valueSerializer = new XmlSerializer(typeof(T));

            foreach (var key in _store.Keys)
            {
                writer.WriteStartElement("item");

                writer.WriteStartElement("key");
                keySerializer.Serialize(writer, key);
                writer.WriteEndElement();

                writer.WriteStartElement("value");
                T value = _store[key];
                valueSerializer.Serialize(writer, value);
                writer.WriteEndElement();
                writer.WriteEndElement();
            }
        }
    }
}
