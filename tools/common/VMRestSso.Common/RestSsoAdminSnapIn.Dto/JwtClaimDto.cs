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

using System.Runtime.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{

    [DataContract]
    public class JwtClaimDto : IDataContext
    {
        [DataMember(EmitDefaultValue = false)]
        private string token_class;

        public string TokenClass
        {
            get { return token_class; }
            set { token_class = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private string token_type;

        public string TokenType
        {
            get { return token_type; }
            set { token_type = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private string jti;

        public string Jti
        {
            get { return jti; }
            set { jti = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private string iss;

        public string Iss
        {
            get { return iss; }
            set { iss = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private string sub;

        public string Sub
        {
            get { return sub; }
            set { sub = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private string aud;

        public string Aud
        {
            get { return aud; }
            set { aud = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private string iat;

        public string Iat
        {
            get { return iat; }
            set { iat = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private string exp;

        public string Exp
        {
            get { return exp; }
            set { exp = value; }
        }
    }
}
