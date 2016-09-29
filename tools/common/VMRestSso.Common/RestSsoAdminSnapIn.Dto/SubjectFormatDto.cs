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
using System.Xml.Schema;
using System.Xml.Serialization;
namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{    
    public class SubjectFormatDto : IDataContext
    {
        public string Key { get; set; }
        public string Value { get; set; }
    }

    public class SubjectFormatDtoCollection<Tk,Tv>: IXmlSerializable where Tk:class
    {
        private readonly IDictionary<Tk, Tv> _store;

        public SubjectFormatDtoCollection()
        {
            _store = new Dictionary<Tk, Tv>();
        }
        public void Add(Tk key, Tv value)
        {
            _store.Add(key, value);            
        }
        XmlSchema IXmlSerializable.GetSchema()
        {
            return null;
        }
        public void ReadXml(System.Xml.XmlReader reader)
        {
            var keySerializer = new XmlSerializer(typeof(Tk));
            var valueSerializer = new XmlSerializer(typeof(Tv));

            bool wasEmpty = reader.IsEmptyElement;
            reader.Read();

            if (wasEmpty)
                return;

            while (reader.NodeType != System.Xml.XmlNodeType.EndElement)
            {
                reader.ReadStartElement("item");

                reader.ReadStartElement("key");
                var key = (Tk)keySerializer.Deserialize(reader);
                reader.ReadEndElement();

                reader.ReadStartElement("value");
                var value = (Tv)valueSerializer.Deserialize(reader);
                reader.ReadEndElement();

                this.Add(key, value);

                reader.ReadEndElement();
                reader.MoveToContent();
            }
            reader.ReadEndElement();
        }

        public void WriteXml(System.Xml.XmlWriter writer)
        {
            var keySerializer = new XmlSerializer(typeof(Tk));
            var valueSerializer = new XmlSerializer(typeof(Tv));

            foreach (var key in _store.Keys)
            {
                writer.WriteStartElement("item");

                writer.WriteStartElement("key");
                keySerializer.Serialize(writer, key);
                writer.WriteEndElement();

                writer.WriteStartElement("value");
                Tv value = _store[key];
                valueSerializer.Serialize(writer, value);
                writer.WriteEndElement();
                writer.WriteEndElement();
            }
        }
    }
}
