﻿/*
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
 */using System;using System.Collections.Generic;using System.Linq;using System.Text;using System.Drawing.Design;using System.Windows.Forms.Design;using System.ComponentModel;using System.Windows.Forms;using System.IO;

namespace VMwareMMCIDP.UI.Common.GridEditors
{    class PGCertificateFileDialog : UITypeEditor    {        public override UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext context)        {            return UITypeEditorEditStyle.Modal;        }
        public override object EditValue(ITypeDescriptorContext context, IServiceProvider provider, object value)        {            var svc = (IWindowsFormsEditorService)provider.GetService(typeof(IWindowsFormsEditorService));            if (svc != null)            {                using (var ofd = new OpenFileDialog())                {                    ofd.Filter = "Certificate Files (*.crt)|*.crt|All Files (*.*)|*.*";                    if (ofd.ShowDialog() == DialogResult.OK)                    {                        value = File.ReadAllText(ofd.FileName);                    }                }            }            return value;        }    }}