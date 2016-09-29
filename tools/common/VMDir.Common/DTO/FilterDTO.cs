/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
 */
using System;

namespace VMDir.Common.DTO
{
    public enum LogicalOp
    {
        AND = 0,
        OR,
        NOT
    }

    public enum Condition
    {
        EQL_TO = 0,
        NOT_EQL_TO,
        BEG_WITH,
        NOT_BEG_WITH,
        END_WITH,
        NOT_END_WITH,
        CONTAINING,
        NOT_CONTAINING,
        GREAT_THAN_EQL,
        NOT_GREAT_THAN_EQL,
        LESS_THAN_EQL,
        NOT_LESS_THAN_EQL
    }

    public class FilterDTO
    {
        public string Attribute { get; set; }
        public Condition Condition { get; set; }
        public string Value { get; set; }
        public FilterDTO()
        {

        }
        public FilterDTO(string attribute, Condition condition, string value)
        {
            this.Attribute = attribute;
            this.Condition = condition;
            this.Value = value;
        }

        public override string ToString()
        {
            string condString = "{0}{1}{2}{3}{4}";
            string lhs = Attribute;
            string rhs = Value;
            string op = "=";

            if (Condition == Condition.END_WITH || Condition == Condition.NOT_END_WITH)
                rhs = "*" + Value;
            else if (Condition == Condition.BEG_WITH || Condition == Condition.NOT_BEG_WITH)
                rhs = Value + "*";
            else if (Condition == Condition.CONTAINING)
                rhs = "*" + Value + "*";
            if (Condition == Condition.GREAT_THAN_EQL || Condition == Condition.NOT_GREAT_THAN_EQL)
                op = ">=";
            else if (Condition == Condition.LESS_THAN_EQL || Condition == Condition.NOT_LESS_THAN_EQL)
                op = "<=";

            if (Condition == Condition.NOT_EQL_TO || Condition == Condition.NOT_BEG_WITH ||
                Condition == Condition.NOT_END_WITH || Condition == Condition.NOT_CONTAINING ||
                Condition == Condition.NOT_GREAT_THAN_EQL || Condition == Condition.NOT_LESS_THAN_EQL)
                return String.Format(condString, "(!(", lhs, op, rhs, "))");
            else
                return String.Format(condString, "(", lhs, op, rhs, ")");
        }
    }
}
