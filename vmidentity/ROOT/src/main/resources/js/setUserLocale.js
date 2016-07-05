/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
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

/**
 * Setting the resource for non-English locale.
 * By default en is always loaded in index.html
 */
var supportedLocales = [ "de", "fr", "ja", "ko", "zh" ];
var cLocale = navigator.languages[0] || navigator.language || navigator.userLanguage;

cLocale = cLocale.toLowerCase();

// Generate a pattern for all supported non-English locales.
pattern = '(' + supportedLocales.join('|') + ')' + '(-|\\b)';
var re = new RegExp(pattern, "i" );
if (re.test(cLocale)) {
   var langID;
   // Need special handling for Chinese simplified and traditional
   if (cLocale.indexOf("zh-tw") != -1 ||
      cLocale.indexOf("zh-hk") != -1 ||
      cLocale.indexOf("zh-hant") != -1) {
      langID = 'zh-TW';
   } else if(cLocale.indexOf("zh-cn") != -1 ||
      cLocale.indexOf("zh-hans") != -1 ||
      cLocale.indexOf("zh") != -1) { // default for chinese
      langID = 'zh-CN';
   } else {
      langID = cLocale.substr(0,2);
   }

   document.write('<script type="text/javascript" src="./resources/locale/' + langID
      + '/welcomeRes.js"> </script>');
}
