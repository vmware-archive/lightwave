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
module.factory('dateUtil', dateUtil);

function dateUtil() {

    var util = {};
    util.unixToDateText = unixToDateText;
    util.numberToTime = numberToTime;
    util.toDate = toDate;
    return util;

    function unixToDateText(UNIX_timestamp) {
        var a = new Date(UNIX_timestamp * 1000);

        if (!isNaN(a.getTime())) {
            var months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
            var year = a.getFullYear();
            var month = months[a.getMonth()];
            var date = a.getDate();
            var hour = a.getHours();
            var min = a.getMinutes();
            var sec = a.getSeconds();
            var time = date + ' ' + month + ' ' + year + ' ' + hour + ':' + min + ':' + sec;
            return time;
        }
        return '';
    }

    function numberToTime(a) {
        var hours = Math.trunc(a / 60);
        var days = Math.trunc(hours / 24);
        hours = hours % 24;
        var minutes = a % 60;
        // console.log(hours + ":" + minutes);
        return days + " days " + hours + " hours " + minutes + " minutes";
    }


    function toDate(dateString) {
        // console.log('Time to date: ' + dateString);
        var yy = dateString.substring(0,2);
        // console.log('yy: ' + yy + 'dateString: ' + dateString);
        var mm = dateString.substring(2,4);
        // console.log('mm: ' + mm + 'dateString: ' + dateString);
        var dd = dateString.substring(4,6);
        // console.log('dd: ' + dd + 'dateString: ' + dateString);
        var hh = dateString.substring(6,8);
        // console.log('hh: ' + hh + 'dateString: ' + dateString);
        var mn = dateString.substring(8,10);
        // console.log('mn: ' + mn + 'dateString: ' + dateString);
        var ss = dateString.substring(10,12);
        // console.log('ss: ' + ss + 'dateString: ' + dateString);
        var ampm = '';
        if(hh > 12) {
            ampm = 'PM';
            hh = hh - 12;
        }
        else {
            ampm = 'AM';
        }
        return mm + "/" + dd + "/" + yy + " " + hh + ":" + mn + " " + ampm;
    }
}