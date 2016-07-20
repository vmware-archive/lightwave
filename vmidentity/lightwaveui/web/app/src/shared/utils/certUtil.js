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
module.factory('certUtil', certUtil);

certUtil.$inject = ['dateUtil', 'popupUtil'];

function certUtil(dateUtil, popupUtil) {

    var util = {};
    util.getCertificateDetails = getCertificateDetails;
    util.extractBase64Encoded = extractBase64Encoded;
    util.decodeJWT = decodeJWT;
    util.viewCertificate = viewCertificate;
    return util;

    function checkexpired(before){

        var beforeDate = new Date(before);
        var currentDate = new Date();
        return beforeDate.getTime() < currentDate.getTime();
    }

    function getCertificateDetails(pem) {
        // console.log('pem: ' + pem);
        var c = new X509();
        c.readCertPEM(pem);
        var issuer = c.getIssuerString();
        issuer = issuer.replace('undefined','DC');
        issuer = reverse(issuer, '/');
        var subject = c.getSubjectString();
        subject = subject.replace('undefined','DC');
        subject = reverse(subject, '/');
        var after = c.getNotAfter();
        var expired = checkexpired(after);
        after = dateUtil.toDate(after);
        var before = c.getNotBefore();
        before = dateUtil.toDate(before);
        //var fingerprint = getfingerprint(c.hex);


        var metadata = {
            "issuer" : issuer,
            "subject" : subject ,
            "after" : after,
            "before" : before,
            "expired": expired
            //"fingerprint" : fingerprint
        };

        // console.log('Certificate metadata: ' + JSON.stringify(metadata));

        return metadata;
    }

    function getfingerprint(s) {
        return KJUR.crypto.Util.hashString(s, 'sha1');
    }

    function reverse(text, delimiter)
    {
        var splitText = text.split(delimiter);
        var reversedText = '';

        for(var i=0;i<splitText.length;i++){
            reversedText = reversedText + splitText[splitText.length-1-i] + '/';
        }

        if(splitText.length > 1){
            reversedText = reversedText.substring(0, reversedText.length - 2);
        }

        return reversedText;
    }

    function extractBase64Encoded(pKey){
        var beginRSA = "-----BEGIN RSA PRIVATE KEY-----";
        var endRSA = "-----END RSA PRIVATE KEY-----";
        var begin = "-----BEGIN PRIVATE KEY-----";
        var end = "-----END PRIVATE KEY-----";
        pKey = pKey.replace(begin, '');
        pKey = pKey.replace(end, '');
        pKey = pKey.replace(beginRSA, '');
        pKey = pKey.replace(endRSA, '');
        return pKey;
    }

    function decodeJWT(jwt) {
        var sJWS = jwt;

        var components = sJWS.split(".");
        var uHeader = b64utos(components[1]).toString();
        //var uClaim = b64utos(components[2]).toString();

        // console.log('Header base64:' + components[1]);
        // console.log('Claims base64:' + components[2]);

        // console.log('Header string: ' + uHeader);
        // console.log('Claims string: ' + uClaim);

        //var pHeader = KJUR.jws.JWS.readSafeJSONString(uHeader);
        //var pClaim = KJUR.jws.JWS.readSafeJSONString(uClaim);

        //var sHeader = JSON.stringify(pHeader, null, "  ");
        //var sClaim = JSON.stringify(pClaim, null, "  ");

        // console.log('Header string: ' + sHeader);
        // console.log('Claims string: ' + sClaim);

        var decodedJWT =
        {
            header: JSON.parse(uHeader)
            //claims: JSON.parse(uClaim)
        };

        return decodedJWT;
    }

    function viewCertificate(scope, encoded, template, controller){

            scope.metadata  = getCertificateDetails(encoded);
            popupUtil.open(scope, template, controller);
            /*ngDialog.open({
                    template: 'shared/components/certificate/certificate.view.html',
                    controller: 'CertificateViewerCntrl',
                    className: 'ngdialog-theme-default',

                    scope: scope
                });*/
    }
}