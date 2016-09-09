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

var module = angular.module('lightwave.ui.sso');
module.controller('subjectFormatCntrl', [ '$scope', '$rootScope',
    function($scope, $rootScope) {

        $scope.saveSubjectFormat = saveSubjectFormat;
        $scope.isValidSubjectFormat = isValidSubjectFormat;

        init();

        function init() {
            $rootScope.globals.errors = '';
            $rootScope.globals.popup_errors = null;
            $scope.newSubjectFormat = {};
        }

        function saveSubjectFormat(subjectFormat) {
            $scope.vm.selectedIdentityProvider.SubjectFormatServices.push(subjectFormat);
            $scope.closeThisDialog('save');
        }

        function isValidSubjectFormat(subjectFormat) {
            return (subjectFormat &&
            subjectFormat.key &&
            subjectFormat.value);
        }

        function isValidUri(uri) {
            var str = uri.toString();
            var expression = /[-a-zA-Z0-9@:%_\+.~#?&//=]{2,256}\.[a-z]{2,4}\b(\/[-a-zA-Z0-9@:%_\+.~#?&//=]*)?/gi;
            var regex = new RegExp(expression);
            return str.match(regex);
        }
    }]);