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

'use strict';

var module = angular.module('lightwave.ui.shared.utils');
module.factory('Util', Util);

Util.$inject = ['dateUtil', 'certUtil', 'numberUtil'];

function Util(dateUtil, certUtil, numberUtil) {

    var util = {};
    util.unixToDateText = dateUtil.unixToDateText;
    util.numberToTime = dateUtil.numberToTime;
    util.getCertificateDetails = certUtil.getCertificateDetails;
    util.extractBase64Encoded = certUtil.extractBase64Encoded;
    util.decodeJWT = certUtil.decodeJWT;
    util.viewCertificate = certUtil.viewCertificate;
    util.isValidBase64 = certUtil.isValidBase64;
    util.isInteger = numberUtil.isInteger;
    return util;
}