"use strict";
/*
 *  Copyright (c) 2012-2017 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
Object.defineProperty(exports, "__esModule", { value: true });
var VmdirUtils = (function () {
    function VmdirUtils() {
    }
    VmdirUtils.prototype.setAttributeDataType = function (attrArr) {
        var obj = {};
        for (var _i = 0, attrArr_1 = attrArr; _i < attrArr_1.length; _i++) {
            var attrObj = attrArr_1[_i];
            if (attrObj.type == 'attributeSyntax') {
                obj.type = 'dataType';
                switch (attrObj.value[0]) {
                    case '1.3.6.1.4.1.1466.115.121.1.2': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.3': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.4': {
                        obj.value = 'ADSTYPE_OCTET_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.5': {
                        obj.value = 'ADSTYPE_OCTET_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.6': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.7': {
                        obj.value = 'ADSTYPE_BOOLEAN';
                        break;
                    }
                    case '1.2.840.113556.1.4.1362': {
                        obj.value = 'ADSTYPE_CASE_EXACT_STRING';
                        break;
                    }
                    case '1.2.840.113556.1.4.1221': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.8': {
                        obj.value = 'ADSTYPE_OCTET_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.9': {
                        obj.value = 'ADSTYPE_OCTET_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.10': {
                        obj.value = 'ADSTYPE_OCTET_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.11': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.13': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.14': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.15': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.12': {
                        obj.value = 'ADSTYPE_DN_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.19': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.21': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.22': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.23': {
                        obj.value = 'ADSTYPE_OCTET_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.24': {
                        obj.value = 'ADSTYPE_UTC_TIME';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.25': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.26': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.27': {
                        obj.value = 'ADSTYPE_INTEGER';
                        break;
                    }
                    case '1.2.840.113556.1.4.906': {
                        obj.value = 'ADSTYPE_LARGE_INTEGER';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.28': {
                        obj.value = 'ADSTYPE_OCTET_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.32': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.34': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.36': {
                        obj.value = 'ADSTYPE_NUMERIC_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.37': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.40': {
                        obj.value = 'ADSTYPE_OCTET_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.38': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.2.840.113556.1.4.1221': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.39': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.41': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.43': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.44': {
                        obj.value = 'ADSTYPE_PRINTABLE_STRING';
                        break;
                    }
                    case '1.2.840.113556.1.4.907': {
                        obj.value = 'ADSTYPE_NT_SECURITY_DESCRIPTOR';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.50': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.51': {
                        obj.value = 'ADSTYPE_OCTET_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.52': {
                        obj.value = 'ADSTYPE_CASE_IGNORE_STRING';
                        break;
                    }
                    case '1.3.6.1.4.1.1466.115.121.1.53': {
                        obj.value = 'ADSTYPE_UTC_TIME';
                        break;
                    }
                }
            }
        }
        attrArr.push(obj);
    };
    return VmdirUtils;
}());
exports.VmdirUtils = VmdirUtils;
//# sourceMappingURL=vmdir.utils.js.map