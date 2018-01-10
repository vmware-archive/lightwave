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

import { EventEmitter, Output, Component, Input } from '@angular/core';
import { VmdirService } from './vmdir.service';
import {StringFilter, DatagridFilter } from "clarity-angular";
import { UserAccountFilter, SimpleStringFilter} from './customstringfilter';
import { SDDLParser, AccountSidsRevMap, AceRightsRevMap } from './sddlparser';

@Component({
  moduleId: module.id,
  selector: 'sdeditor',
  templateUrl: './sdeditor.component.html',
})

export class SdeditorComponent  {
    @Input() aclString:string; /* per view */
    @Input() rootDn:string;
    @Input() usersArr:any[];
    @Input() groupsArr:any[];
    @Output() notify: EventEmitter<string> = new EventEmitter<string>();
    SDObj:any;
    userFilter = new UserAccountFilter();
    groupFilter = new UserAccountFilter();
    builtinFilter = new SimpleStringFilter();
    selectedAcct:any;
    private error:any = '';
    owner:string ='';
    selectedOwner:string ='';
    group:string = '';
    newAceInher:any;
    newAceType:any;
    dacl:any;
    display:string;
    allRights:string[];
    isRightsEdited:boolean;
    newAce:any;
    isOwnerEdited:boolean;
    accountsArr:any[];
    wellKnownActs:string[];
    daclFlags:any;
    constructor(private vmdirService: VmdirService, private sddlParser:SDDLParser){
        this.accountsArr = [];
        this.isRightsEdited = false;
        this.isOwnerEdited = false;
        this.display = 'sdEditor';
        this.selectedOwner = 'Owner';
        this.wellKnownActs = Object.keys(AccountSidsRevMap);
        console.log(this.wellKnownActs);
    }

    ngOnInit(){
        this.initializeSecurityDescriptor()
        console.log(this.usersArr);
    }

    initializeSecurityDescriptor(){
        this.SDObj = this.sddlParser.parseSDDL(this.aclString);
        this.allRights = this.sddlParser.getAllRights();
        this.parseOwnerAndGroup();
        this.rootDn = decodeURIComponent(this.rootDn);
        this.extractDacl();
    }

    rightAlreadySelected(right:string, rightsArr:any[]):number{
        for(var j = 0; j < rightsArr.length; j ++){
            if(rightsArr[j].details == right){
                return j;
            }
        }
        return -1;
    }

    getAccountSid(selectedAcct):string{
        if(selectedAcct.attributes){
            return window.atob(selectedAcct.objectSid);
        }else{
            return AccountSidsRevMap[selectedAcct];
        }
    }

    getACLString() {
        let sddlString:string = '';
        let sddlObj:any;
        this.vmdirService.getACLString(encodeURIComponent(this.rootDn))
        .subscribe(
             aclstring => {
                              sddlString = JSON.stringify(aclstring);
                              sddlObj = JSON.parse(sddlString).result[0];
                              console.log(sddlObj);
                              this.aclString = sddlObj.attributes[0].value[0];
                              console.log(this.aclString);
                              this.initializeSecurityDescriptor()
                          });
    }


    setOwner(selectedAcct:any){
        let origOwnerPrefix = 'O:' + this.SDObj.ownerValue;
        this.SDObj.ownerValue = this.getAccountSid(selectedAcct);
        this.SDObj.value = this.SDObj.value.replace(origOwnerPrefix, 'O:'+ this.SDObj.ownerValue);
        this.submitChanges(this.SDObj.value);
    }

    closeSD(){
        this.notify.emit('done');
    }
    handleUpdateError(error:any){
        console.log(error);
    }
    clickAce(){
        this.isRightsEdited = false;
    }

    submitChanges(aceString:string){
        this.vmdirService.updateAclString(this.rootDn, aceString)
            .subscribe(
                result => {
                    console.log(result);
                    this.isRightsEdited = false;
                    // Reload the security descriptor from server
                    this.getACLString();
                    },
                error => {
                         this.handleUpdateError(error);
                }
             );
    }
    onSubmit(updatedAce:any){
        // find the modified rights
        console.log(updatedAce);
        let noChanges:boolean = true, result;
        //for each right in rightsMap set to true search if its present in rights array
        // if not add it.
        let allRights = Object.keys(updatedAce.rightsMap);
        let oldRightsStr = '';
        if(updatedAce.rightsArr){ // check every subobject to be safe
            oldRightsStr = updatedAce.rightsValue;
        }
        let rightsArr = updatedAce.rightsArr;
        //let rightsArr = updatedAce.rightsArr;
        for (var i = 0; i < allRights.length; i ++){
            let id = this.rightAlreadySelected(allRights[i], rightsArr);
            if(updatedAce.rightsMap[allRights[i]] && (-1 == id)){  // test and add if present in rights array if found in Map
                let newRight:any = {};
                updatedAce.rightsValue += AceRightsRevMap[allRights[i]];
                noChanges = false;
            }else if(!updatedAce.rightsMap[allRights[i]] && (-1 != id)){ //test and remove if present in rights array if absent in Map
                updatedAce.rightsValue = updatedAce.rightsValue.substr(0, id * 2) + updatedAce.rightsValue.substr((id+1)*2);
                noChanges = false;
            }
        }
        if(!noChanges){
            if(oldRightsStr.length > 0){ // if rights exist, serialize sddl string
                let origAceValue = updatedAce.value;
                updatedAce.value = updatedAce.value.replace(oldRightsStr, updatedAce.rightsValue);
                this.SDObj.value = this.SDObj.value.replace(origAceValue, updatedAce.value);
            }else{ //TODO insert rights value
            }
            this.submitChanges(this.SDObj.value);
        }
        //this.notify.emit('done');
    }
    parseOwnerAndGroup(){
        let ownerSID:string;
        let res:any, resObj:any;
        if(!this.SDObj.wellKnownOwner){// If owner not in predefined set, query it based on SID
            ownerSID = this.SDObj.owner;
            this.vmdirService.getObjectBySID(ownerSID)
            .subscribe(
            listing => {
                    res = JSON.stringify(listing);
                    resObj = JSON.parse(res);
                    console.log(resObj);
                    this.owner = resObj.result[0].dn;
                    });
            this.group = this.SDObj.wellKnownGroup;
        }
    }

    submitNewAce(selectedAccount){
        let newAceStr:string = this.constructNewAce(selectedAccount);
        let finalAce = '(' + newAceStr + ')';
        let newSdStr = this.SDObj.value + finalAce;
        this.submitChanges(newSdStr);
    }
    constructRightsMap():any{
        let rmap = {};
        for(let right of this.allRights){
           rmap[right] = false;
        }
        return rmap;
    }

    prepareNewAce(){
        this.newAce = {}
        this.newAceInher = {};
        this.newAceInher.display = "This object only";
        this.newAceInher.value = '';
        this.newAceType = {};
        this.newAceType.display = 'Allow';
        this.newAceType.value = 'A';
        this.newAce.rights = {};
        this.newAce.rights.map = this.constructRightsMap();
        this.isOwnerEdited = false;
    }

    constructNewAce(selectedAccount:any):string{
        this.newAce.value = 'Type;Flags;Permissions;;;Principal';
        //Construct type
        this.newAce.type = {};
        this.newAce.type.value = this.newAceType.value;
        //Construct Flags
        this.newAce.flags = {};
        this.newAce.flags.value = this.newAceInher.value;
        //construct Principal
        this.newAce.account = {};
        this.newAce.account.value = this.getAccountSid(selectedAccount);
        //Construct Permissions
        this.newAce.rights.value = '';
        for(let right of this.allRights){
            if(this.newAce.rights.map[right]){
                this.newAce.rights.value += AceRightsRevMap[right];
            }
        }
        this.newAce.value = this.newAce.value.replace('Type', this.newAce.type.value);
        this.newAce.value = this.newAce.value.replace('Flags', this.newAce.flags.value);
        this.newAce.value = this.newAce.value.replace('Permissions', this.newAce.rights.value);
        this.newAce.value = this.newAce.value.replace('Principal', this.newAce.account.value);
        console.log(this.newAce.value);
        return this.newAce.value;
    }

    extractDacl() {
        let res:any, resObj:any;
        let listing:any;
        //this.dacl = this.sdObj.details[2].details.details[22].details
        this.dacl = this.SDObj.dacl;
        let aces = this.dacl.aces;
        // extract the trustee for each ACE
        for (let ace of aces){
            ace.disableForm = false;
            console.log(ace);
            let accountSID = ace.accountSID;
            if(accountSID.startsWith('S-1')){
                if(accountSID == 'S-1-7-32-666'){
                    ace.account = 'SELF';
                }else{
                    this.vmdirService.getObjectBySID(accountSID)
                    .subscribe(
                    listing => {
                            res = JSON.stringify(listing);
                            resObj = JSON.parse(res);
                            console.log(resObj);
                            if(resObj.result_count > 0){
                                ace.account = resObj.result[0].dn;
                            }else{
                                ace.account = accountSID;
                            }
                        });
                 }
            }else{
                ace.account = accountSID;
            }

            // extract if the ACE is inherited
            ace.inheritedFrom = 'none';
            if(ace.flagsMap && ace.flagsMap['INHERITED']){
                ace.inheritedFrom = 'Parent Object';
                ace.disableForm = true;
            }

            // Determine to whom this ACE appiles to
            if(ace.type == 'ALLOW' ||
               ace.type == 'DENY'){
                if(ace.flagsMap){
                    if(!ace.flagsMap['INHERIT_ONLY']){
                        ace.appliesTo = 'This Object';
                    }
                    if(ace.flagsMap['CONTAINER_INHERIT']){
                        if(ace.appliesTo.length){
                            ace.appliesTo += ',';
                        }
                        ace.appliesTo += 'Child Containers';
                    }
                    if(ace.flagsMap['OBJECT_INHERIT']){
                        if(ace.appliesTo.length){
                            ace.appliesTo += ',';
                        }
                        ace.appliesTo += 'Child Objects';
                    }
                }else{
                    ace.appliesTo = 'This object only';
                }
            }
        }
    }
}
