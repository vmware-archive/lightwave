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

var module = angular.module('lightwave.ui.shared.directives');
module.directive('onReadFile', fileReaderDirective);

function fileReaderDirective($parse) {
    return {
        require:'ngModel',
        restrict: 'A',
        scope : false,
        link: function(scope, element, attrs, ngModel) {
            element.bind('change', function(e) {
                ngModel.$setViewValue(element.val());
                ngModel.$render();
                var onFileReadFn = $parse(attrs.onReadFile);
                var reader = new FileReader();
                reader.onload = function() {
                    var fileContents = reader.result;
                    scope.$apply(function() {
                        if(fileContents != null && fileContents != undefined) {
                            onFileReadFn(scope, {
                                'contents': fileContents
                            });
                        }
                    });
                };

                if(element[0].files != null && element[0].files.length > 0) {
                    reader.readAsText(element[0].files[0]);
                }
            });
        }
    };
}
