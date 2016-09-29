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
using System.Runtime.Serialization;
namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    [TypeConverter(typeof(ExpandableObjectConverter))]
    [Serializable]
    public class PasswordPolicyDto : IDataContext
    {
        public PasswordPolicyDto() { }
        [DataMember]
        private string description;

        [DescriptionAttribute("Password policy description")]
        public string Description
        {
            get { return description; }
            set { description = value; }
        }
        [DataMember]
        private int maxIdenticalAdjacentCharacters;

        [DescriptionAttribute("Policy to allow maximum identical adjacent characters.")]
        public int MaxIdenticalAdjacentCharacters
        {
            get { return maxIdenticalAdjacentCharacters; }
            set { maxIdenticalAdjacentCharacters = value; }
        }
        [DataMember]
        private int maxLength;

        [DescriptionAttribute("Policy for maximum length allowed for the password.")]
        public int MaxLength
        {
            get { return maxLength; }
            set { maxLength = value; }
        }
        [DataMember]
        private int minAlphabeticCount;

        [DescriptionAttribute("Policy for minimum alphabets allowed in the password.")]
        public int MinAlphabeticCount
        {
            get { return minAlphabeticCount; }
            set { minAlphabeticCount = value; }
        }
        [DataMember]
        private int minLength;

        [DescriptionAttribute("Policy for minimum length allowed for the password.")]
        public int MinLength
        {
            get { return minLength; }
            set { minLength = value; }
        }
        [DataMember]
        private int minLowercaseCount;

        [DescriptionAttribute("Policy for minimum lower case alphabets allowed in the password.")]
        public int MinLowercaseCount
        {
            get { return minLowercaseCount; }
            set { minLowercaseCount = value; }
        }
        [DataMember]
        private int minNumericCount;
        [DescriptionAttribute("Policy for minimum numeric characters allowed in the password.")]
        public int MinNumericCount
        {
            get { return minNumericCount; }
            set { minNumericCount = value; }
        }
        [DataMember]
        private int minSpecialCharCount;

        [DescriptionAttribute("Policy for minimum special  characters allowed in the password.")]
        public int MinSpecialCharCount
        {
            get { return minSpecialCharCount; }
            set { minSpecialCharCount = value; }
        }
        [DataMember]
        private int minUppercaseCount;

        [DescriptionAttribute("Policy for minimum upper case alphabets allowed in the password.")]
        public int MinUppercaseCount
        {
            get { return minUppercaseCount; }
            set { minUppercaseCount = value; }
        }
        [DataMember]
        private int passwordLifetimeDays;

        [DescriptionAttribute("Policy for password expiration in days.")]
        public int PasswordLifetimeDays
        {
            get { return passwordLifetimeDays; }
            set { passwordLifetimeDays = value; }
        }
        [DataMember]
        private int prohibitedPreviousPasswordCount;

        [DescriptionAttribute("Policy for count of previous passwords that are prohibited.")]
        public int ProhibitedPreviousPasswordCount
        {
            get { return prohibitedPreviousPasswordCount; }
            set { prohibitedPreviousPasswordCount = value; }
        }
    }
}
