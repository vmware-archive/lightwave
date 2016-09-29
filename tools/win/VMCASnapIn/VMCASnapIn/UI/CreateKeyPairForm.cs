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
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using VMCASnapIn.Utilities;
using VMCA.Client;
using System.IO;
using VMCASnapIn.DTO;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.UI
{
    public partial class CreateKeyPairForm : Form
    {
        public KeyPairDTO DTO { get; protected set; }

        public CreateKeyPairForm()
        {
            InitializeComponent();

            DTO = new KeyPairDTO();
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            MMCActionHelper.CheckedExec(delegate()
            {
                var keyPair = VMCAKeyPair.Create((uint)numKeyLength.Value);
                DTO.CreatedDateTime = DateTime.Now;
                DTO.KeyLength = (int)numKeyLength.Value;
                DTO.PrivateKey = keyPair.PrivateKey;
                DTO.PublicKey = keyPair.PublicKey;

                if (MessageBox.Show("Do you want to save the keys created?", "Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
                {
                    Helper.SaveKeyData(keyPair);
                }
            });
        }
    }
}
