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

let DaclFlags = {
    'NO_ACCESS_CONTROL': 'NULL_ACL',
    'AR': 'SE_DACL_AUTO_INHERIT_REQ ',
    'AI': 'SE_DACL_AUTO_INHERITED ',
    'P': 'SE_DACL_PROTECTED'
}

let AceTypes = {
    'A': 'ALLOW',
    'D': 'DENY'
}

let AceFlags = {
    'CI': 'CONTAINER_INHERIT',
    'OI': 'OBJECT_INHERIT',
    'NP': 'NO_PROPAGATE_INHERIT',
    'IO': 'INHERIT_ONLY',
    'ID': 'INHERITED',
    'SA': 'SUCCESSFUL_ACCESS',
    'FA': 'FAILED_ACCESS'
}

let AccountSidsMap = {
    'AN': 'Anonymous Logon',
    'AO': 'Account Operators',
    'RU': 'Alias to allow previous Windows 2000',
    'AU': 'Authenticated Users',
    'BA': 'Built-in Administrators',
    'BG': 'Built in Guests',
    'BO': 'Backup Operators',
    'BU': 'Built-in Users',
    'CA': 'Certificate Server Administrators',
    'CG': 'Creator Group',
    'CO': 'Creator Owner',
    'DA': 'Domain Administrators',
    'DC': 'Domain Computers',
    'DD': 'Domain Controllers',
    'DG': 'Domain Guests',
    'DU': 'Domain Users',
    'EA': 'Enterprise Administrators',
    'ED': 'Enterprise Domain Controllers',
    'WD': 'Everyone',
    'PA': 'Group Policy Administrators',
    'IU': 'Interactively logged-on user',
    'LA': 'Local Administrator',
    'LG': 'Local Guest',
    'LS': 'Local Service Account',
    'SY': 'Local System',
    'NU': 'Network Logon User',
    'NO': 'Network Configuration Operators',
    'NS': 'Network Service Account',
    'PO': 'Printer Operators',
    'PS': 'Self',
    'PU': 'Power Users',
    'RS': 'RAS Servers group',
    'RD': 'Terminal Server Users',
    'RE': 'Replicator',
    'RC': 'Restricted Code',
    'SA': 'Schema Administrators',
    'SO': 'Server Operators',
    'SU': 'Service Logon User'
}

export var AccountSidsRevMap = {
    'Anonymous Logon':'AN',
    'Account Operators':'AO',
    'Alias to allow previous Windows 2000':'RU',
    'Authenticated Users':'AU',
    'Built-in Administrators':'BA',
    'Built in Guests':'BG',
    'Backup Operators':'BO',
    'Built-in Users':'BU',
    'Certificate Server Administrators':'CA',
    'Creator Group':'CG',
    'Creator Owner':'CO',
    'Domain Administrators':'DA',
    'Domain Computers':'DC',
    'Domain Controllers':'DD',
    'Domain Guests':'DG',
    'Domain Users':'DU',
    'Enterprise Administrators':'EA',
    'Enterprise Domain Controllers':'ED',
    'Everyone':'WD',
    'Group Policy Administrators':'PA',
    'Interactively logged-on user':'IU',
    'Local Administrator':'LA',
    'Local Guest':'LG',
    'Local Service Account':'LS',
    'Local System':'SY',
    'Network Logon User':'NU',
    'Network Configuration Operators':'NO',
    'Network Service Account':'NS',
    'Printer Operators':'PO',
    'Self':'PS',
    'Power Users':'PU',
    'RAS Servers group':'RS',
    'Terminal Server sers':'RD',
    'Replicator':'RE',
    'Restricted Code':'RC',
    'Schema Administrators':'SA',
    'Server Operators':'SO',
    'Service Logon User':'SU'
}


let AceRights = {
    // Generic access rights
    'GA': 'GENERIC_ALL',
    'GR': 'GENERIC_READ',
    'GW': 'GENERIC_WRITE',
    'GX': 'GENERIC_EXECUTE',
    // Standard access rights
    'RC': 'READ_CONTROL',
    'SD': 'DELETE',
    'WD': 'WRITE_DAC',
    'WO': 'WRITE_OWNER',
    // Directory service object access rights
    'RP': 'ADS_RIGHT_DS_READ_PROP',
    'WP': 'ADS_RIGHT_DS_WRITE_PROP',
    'CC': 'ADS_RIGHT_DS_CREATE_CHILD',
    'DC': 'ADS_RIGHT_DS_DELETE_CHILD',
    'LC': 'ADS_RIGHT_ACTRL_DS_LIST',
    'SW': 'ADS_RIGHT_DS_SELF',
    'LO': 'ADS_RIGHT_DS_LIST_OBJECT',
    'DT': 'ADS_RIGHT_DS_DELETE_TREE',
    'CR': 'ADS_RIGHT_DS_CONTROL_ACCESS',
    // File access rights
    'FA': 'FILE_ALL_ACCESS',
    'FR': 'FILE_GENERIC_READ',
    'FW': 'FILE_GENERIC_WRITE',
    'FX': 'FILE_GENERIC_EXECUTE',
    // Registry key access rights
    'KA': 'KEY_ALL_ACCESS',
    'KR': 'KEY_READ',
    'KW': 'KEY_WRITE',
    'KX': 'KEY_EXECUTE',
    // Mandatory label rights
    'NR': 'SYSTEM_MANDATORY_LABEL_NO_READ_UP',
    'NW': 'SYSTEM_MANDATORY_LABEL_NO_WRITE_UP',
    'NX': 'SYSTEM_MANDATORY_LABEL_NO_EXECUTE'
}

export var AceRightsRevMap = {
    // Generic access rights
    'GENERIC_ALL': 'GA',
    'GENERIC_READ':'GR',
    'GENERIC_WRITE':'GW',
    'GENERIC_EXECUTE':'GX',
    // Standard access rights
    'READ_CONTROL':'RC',
    'DELETE':'SD',
    'WRITE_DAC':'WD',
    'WRITE_OWNER':'WO',
    // Directory service object access rights
    'ADS_RIGHT_DS_READ_PROP':'RP',
    'ADS_RIGHT_DS_WRITE_PROP':'WP',
    'ADS_RIGHT_DS_CREATE_CHILD':'CC',
    'ADS_RIGHT_DS_DELETE_CHILD':'DC',
    'ADS_RIGHT_ACTRL_DS_LIST':'LC',
    'ADS_RIGHT_DS_SELF':'SW',
    'ADS_RIGHT_DS_LIST_OBJECT':'LO',
    'ADS_RIGHT_DS_DELETE_TREE':'DT',
    'ADS_RIGHT_DS_CONTROL_ACCESS':'CR',
    // File access rights
    'FILE_ALL_ACCESS':'FA',
    'FILE_GENERIC_READ':'FR',
    'FILE_GENERIC_WRITE':'FW',
    'FILE_GENERIC_EXECUTE':'FX',
    // Registry key access rights
    'KEY_ALL_ACCESS':'KA',
    'KEY_READ':'KR',
    'KEY_WRITE':'KW',
    'KEY_EXECUTE':'KX',
    // Mandatory label rights
    'SYSTEM_MANDATORY_LABEL_NO_READ_UP':'NR',
    'SYSTEM_MANDATORY_LABEL_NO_WRITE_UP':'NW',
    'SYSTEM_MANDATORY_LABEL_NO_EXECUTE':'NX'
}

export class SDDLParser{
    parseSDDL(sddl:string):any{
        let i:number = 0;
        let pos:number = 0;
        let blockMap:any = {};
        let sddlObj:any = {};
        sddlObj.value = sddl;
        while (i != -1) {
            i = sddl.indexOf(':', pos+1)
            if (i == -1) {
                break
            } else if (pos > 0) {
                blockMap[sddl[pos-1]] = sddl.substring(pos+1, i-1);
            }
            pos = i
       }
       if(pos > 0) {
          blockMap[sddl[pos-1]] = sddl.substring(pos+1);
       }

       //parse Onwer
       sddlObj.ownerValue = blockMap['O'];
       if(blockMap['O']){
           if(AccountSidsMap[blockMap['O']]){
               sddlObj.wellKnownOwner = AccountSidsMap[blockMap['O']];
           }else{
               sddlObj.owner = blockMap['O'];
           }
       }

       //parse Group
       if(blockMap['G']){
           if(AccountSidsMap[blockMap['G']]){
               sddlObj.wellKnownGroup = AccountSidsMap[blockMap['G']];
           }else{
               sddlObj.group = blockMap['G'];
           }
       }

       //parse DACL
       if(blockMap['D']){
           sddlObj.dacl = this.parseDACL(blockMap['D']);
       }
       return sddlObj;
    }

    parseDaclFlags(flags:string):any{
        let flagMap={};
        let keys = Object.keys(DaclFlags);
        let incr = 0;
        while (flags.length > 0) {
            for (let i = 0; i < keys.length; i++) {
                let flag = keys[i]
                if(!flagMap[DaclFlags[flag]]){
                    flagMap[DaclFlags[flag]] = false;
                }
                if (flags.length >= flag.length &&
                    flags.substring(0, flag.length) == flag) {
                    incr = flag.length
                    flagMap[DaclFlags[flag]] = true;
                }
            }
            flags = flags.substring(incr)
        }
        return flagMap;
    }

    getAllRights() {
        let allRights = [];
        for(let k in AceRights){
            allRights.push(AceRights[k]);
        }
        return allRights;
    }

    parseAceFlags(flags:string):any{
        let flagMap={};
        let keys = Object.keys(AceFlags);
        let incr = 0;
        while (flags.length > 0) {
            for (let i = 0; i < keys.length; i++) {
                let flag = keys[i]
                if(!flagMap[AceFlags[flag]]){
                    flagMap[AceFlags[flag]] = false;
                }
                if (flags.length >= flag.length &&
                    flags.substring(0, flag.length) == flag) {
                    incr = flag.length
                    flagMap[AceFlags[flag]] = true;
                }
            }
            flags = flags.substring(incr)
        }
        return flagMap;
    }

    parseAceRights(rights:string){
        let rightMap={};
        let keys = Object.keys(AceRights);
        let incr = 0;
        let rightsArr = [];
        while (rights.length > 0) {
            for (let i = 0; i < keys.length; i++) {
                let right = keys[i]
                if(!rightMap[AceRights[right]]){
                    rightMap[AceRights[right]] = false;
                }
                if (rights.length >= right.length &&
                    rights.substring(0, right.length) == right) {
                    incr = right.length
                    rightMap[AceRights[right]] = true;
                    rightsArr.push({
                    'value': right,
                    'details': AceRights[right]
                })

                }
            }
            rights = rights.substring(incr)
        }
        return [rightsArr,rightMap];
    }

    parseACEs(aceString:string):any{
       let aceObj:any = {};
       aceString = aceString.replace('(','');
       aceString = aceString.replace(')','');
       aceString = aceString.replace(')','');
       aceObj.value = aceString;
       let aceComponents = aceString.split(';');
       if(aceComponents.length < 6)
           return aceObj;

       //parse ACE type
       if(aceComponents[0].length > 0){
           aceObj.type = {};
           aceObj.type = AceTypes[aceComponents[0]];
       }

       //parse ACE Flags
       if(aceComponents[1].length > 0){
           aceObj.flagsMap = {};
           aceObj.flagsMap = this.parseAceFlags(aceComponents[1]);
       }

       //parse rights
       if(aceComponents[2].length > 0){
          aceObj.rightsMap = {};
          aceObj.rightsArr = [];
          aceObj.rightsValue = aceComponents[2];
          let retRights = this.parseAceRights(aceComponents[2]);
          aceObj.rightsArr = retRights[0];
          aceObj.rightsMap = retRights[1];
       }

       //parse Account SID
       if(aceComponents[5].length > 0){
          if(AccountSidsMap[aceComponents[5]]){
              aceObj.accountSID = AccountSidsMap[aceComponents[5]];
          }else{
              aceObj.accountSID = aceComponents[5];
          }
       }

       return aceObj;
    }

    parseDACL(dacl:string):any{
       let pos = dacl.indexOf('(');
       let flags = '';
       let aces = '';
       let daclObj:any = {};
       if(pos != -1){
           flags = dacl.substring(0,pos);
           aces = dacl.substring(pos);
       }else{ // If no ACEs are present
           flags = dacl;
       }


       // Parse the flags
       daclObj.daclFlags = this.parseDaclFlags(flags);

       // Parse the ACEs
       daclObj.aces = []
       let acesArr = aces.split('(');
       for(let i = 0; i < acesArr.length; i++){
           acesArr[i] = acesArr[i].trim();
           if(acesArr[i].length){
               daclObj.aces.push(this.parseACEs(acesArr[i] + ')'));
           }
       }
       return daclObj;
    }
}
