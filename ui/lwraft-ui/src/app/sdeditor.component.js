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
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
var __metadata = (this && this.__metadata) || function (k, v) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function") return Reflect.metadata(k, v);
};
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = require("@angular/core");
var vmdir_service_1 = require("./vmdir.service");
var customstringfilter_1 = require("./customstringfilter");
var sddlparser_1 = require("./sddlparser");
var SdeditorComponent = (function () {
    function SdeditorComponent(vmdirService, sddlParser) {
        this.vmdirService = vmdirService;
        this.sddlParser = sddlParser;
        this.notify = new core_1.EventEmitter();
        this.userFilter = new customstringfilter_1.UserAccountFilter();
        this.groupFilter = new customstringfilter_1.UserAccountFilter();
        this.builtinFilter = new customstringfilter_1.SimpleStringFilter();
        this.error = '';
        this.owner = '';
        this.selectedOwner = '';
        this.group = '';
        this.accountsArr = [];
        this.isRightsEdited = false;
        this.isOwnerEdited = false;
        this.display = 'sdEditor';
        this.selectedOwner = 'Owner';
        this.wellKnownActs = Object.keys(sddlparser_1.AccountSidsRevMap);
        console.log(this.wellKnownActs);
    }
    SdeditorComponent.prototype.ngOnInit = function () {
        this.initializeSecurityDescriptor();
        console.log(this.usersArr);
    };
    SdeditorComponent.prototype.initializeSecurityDescriptor = function () {
        this.SDObj = this.sddlParser.parseSDDL(this.aclString);
        this.allRights = this.sddlParser.getAllRights();
        this.parseOwnerAndGroup();
        this.rootDn = decodeURIComponent(this.rootDn);
        this.extractDacl();
    };
    SdeditorComponent.prototype.rightAlreadySelected = function (right, rightsArr) {
        for (var j = 0; j < rightsArr.length; j++) {
            if (rightsArr[j].details == right) {
                return j;
            }
        }
        return -1;
    };
    SdeditorComponent.prototype.getAccountSid = function (selectedAcct) {
        if (selectedAcct.attributes) {
            return window.atob(selectedAcct.objectSid);
        }
        else {
            return sddlparser_1.AccountSidsRevMap[selectedAcct];
        }
    };
    SdeditorComponent.prototype.getACLString = function () {
        var _this = this;
        var sddlString = '';
        var sddlObj;
        this.vmdirService.getACLString(encodeURIComponent(this.rootDn))
            .subscribe(function (aclstring) {
            sddlString = JSON.stringify(aclstring);
            sddlObj = JSON.parse(sddlString).result[0];
            console.log(sddlObj);
            _this.aclString = sddlObj.attributes[0].value[0];
            console.log(_this.aclString);
            _this.initializeSecurityDescriptor();
        });
    };
    SdeditorComponent.prototype.setOwner = function (selectedAcct) {
        var origOwnerPrefix = 'O:' + this.SDObj.ownerValue;
        this.SDObj.ownerValue = this.getAccountSid(selectedAcct);
        this.SDObj.value = this.SDObj.value.replace(origOwnerPrefix, 'O:' + this.SDObj.ownerValue);
        this.submitChanges(this.SDObj.value);
    };
    SdeditorComponent.prototype.closeSD = function () {
        this.notify.emit('done');
    };
    SdeditorComponent.prototype.handleUpdateError = function (error) {
        console.log(error);
    };
    SdeditorComponent.prototype.clickAce = function () {
        this.isRightsEdited = false;
    };
    SdeditorComponent.prototype.submitChanges = function (aceString) {
        var _this = this;
        this.vmdirService.updateAclString(this.rootDn, aceString)
            .subscribe(function (result) {
            console.log(result);
            _this.isRightsEdited = false;
            // Reload the security descriptor from server
            _this.getACLString();
        }, function (error) {
            _this.handleUpdateError(error);
        });
    };
    SdeditorComponent.prototype.onSubmit = function (updatedAce) {
        // find the modified rights
        console.log(updatedAce);
        var noChanges = true, result;
        //for each right in rightsMap set to true search if its present in rights array
        // if not add it.
        var allRights = Object.keys(updatedAce.rightsMap);
        var oldRightsStr = '';
        if (updatedAce.rightsArr) {
            oldRightsStr = updatedAce.rightsValue;
        }
        var rightsArr = updatedAce.rightsArr;
        //let rightsArr = updatedAce.rightsArr;
        for (var i = 0; i < allRights.length; i++) {
            var id = this.rightAlreadySelected(allRights[i], rightsArr);
            if (updatedAce.rightsMap[allRights[i]] && (-1 == id)) {
                var newRight = {};
                updatedAce.rightsValue += sddlparser_1.AceRightsRevMap[allRights[i]];
                noChanges = false;
            }
            else if (!updatedAce.rightsMap[allRights[i]] && (-1 != id)) {
                updatedAce.rightsValue = updatedAce.rightsValue.substr(0, id * 2) + updatedAce.rightsValue.substr((id + 1) * 2);
                noChanges = false;
            }
        }
        if (!noChanges) {
            if (oldRightsStr.length > 0) {
                var origAceValue = updatedAce.value;
                updatedAce.value = updatedAce.value.replace(oldRightsStr, updatedAce.rightsValue);
                this.SDObj.value = this.SDObj.value.replace(origAceValue, updatedAce.value);
            }
            else {
            }
            this.submitChanges(this.SDObj.value);
        }
        //this.notify.emit('done');
    };
    SdeditorComponent.prototype.parseOwnerAndGroup = function () {
        var _this = this;
        var ownerSID;
        var res, resObj;
        if (!this.SDObj.wellKnownOwner) {
            ownerSID = this.SDObj.owner;
            this.vmdirService.getObjectBySID(ownerSID)
                .subscribe(function (listing) {
                res = JSON.stringify(listing);
                resObj = JSON.parse(res);
                console.log(resObj);
                _this.owner = resObj.result[0].dn;
            });
            this.group = this.SDObj.wellKnownGroup;
        }
    };
    SdeditorComponent.prototype.submitNewAce = function (selectedAccount) {
        var newAceStr = this.constructNewAce(selectedAccount);
        var finalAce = '(' + newAceStr + ')';
        var newSdStr = this.SDObj.value + finalAce;
        this.submitChanges(newSdStr);
    };
    SdeditorComponent.prototype.constructRightsMap = function () {
        var rmap = {};
        for (var _i = 0, _a = this.allRights; _i < _a.length; _i++) {
            var right = _a[_i];
            rmap[right] = false;
        }
        return rmap;
    };
    SdeditorComponent.prototype.prepareNewAce = function () {
        this.newAce = {};
        this.newAceInher = {};
        this.newAceInher.display = "This object only";
        this.newAceInher.value = '';
        this.newAceType = {};
        this.newAceType.display = 'Allow';
        this.newAceType.value = 'A';
        this.newAce.rights = {};
        this.newAce.rights.map = this.constructRightsMap();
        this.isOwnerEdited = false;
    };
    SdeditorComponent.prototype.constructNewAce = function (selectedAccount) {
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
        for (var _i = 0, _a = this.allRights; _i < _a.length; _i++) {
            var right = _a[_i];
            if (this.newAce.rights.map[right]) {
                this.newAce.rights.value += sddlparser_1.AceRightsRevMap[right];
            }
        }
        this.newAce.value = this.newAce.value.replace('Type', this.newAce.type.value);
        this.newAce.value = this.newAce.value.replace('Flags', this.newAce.flags.value);
        this.newAce.value = this.newAce.value.replace('Permissions', this.newAce.rights.value);
        this.newAce.value = this.newAce.value.replace('Principal', this.newAce.account.value);
        console.log(this.newAce.value);
        return this.newAce.value;
    };
    SdeditorComponent.prototype.extractDacl = function () {
        var res, resObj;
        var listing;
        //this.dacl = this.sdObj.details[2].details.details[22].details
        this.dacl = this.SDObj.dacl;
        var aces = this.dacl.aces;
        var _loop_1 = function (ace) {
            ace.disableForm = false;
            console.log(ace);
            var accountSID = ace.accountSID;
            if (accountSID.startsWith('S-1')) {
                if (accountSID == 'S-1-7-32-666') {
                    ace.account = 'SELF';
                }
                else {
                    this_1.vmdirService.getObjectBySID(accountSID)
                        .subscribe(function (listing) {
                        res = JSON.stringify(listing);
                        resObj = JSON.parse(res);
                        console.log(resObj);
                        if (resObj.result_count > 0) {
                            ace.account = resObj.result[0].dn;
                        }
                        else {
                            ace.account = accountSID;
                        }
                    });
                }
            }
            else {
                ace.account = accountSID;
            }
            // extract if the ACE is inherited
            ace.inheritedFrom = 'none';
            if (ace.flagsMap && ace.flagsMap['INHERITED']) {
                ace.inheritedFrom = 'Parent Object';
                ace.disableForm = true;
            }
            // Determine to whom this ACE appiles to
            if (ace.type == 'ALLOW' ||
                ace.type == 'DENY') {
                if (ace.flagsMap) {
                    if (!ace.flagsMap['INHERIT_ONLY']) {
                        ace.appliesTo = 'This Object';
                    }
                    if (ace.flagsMap['CONTAINER_INHERIT']) {
                        if (ace.appliesTo.length) {
                            ace.appliesTo += ',';
                        }
                        ace.appliesTo += 'Child Containers';
                    }
                    if (ace.flagsMap['OBJECT_INHERIT']) {
                        if (ace.appliesTo.length) {
                            ace.appliesTo += ',';
                        }
                        ace.appliesTo += 'Child Objects';
                    }
                }
                else {
                    ace.appliesTo = 'This object only';
                }
            }
        };
        var this_1 = this;
        // extract the trustee for each ACE
        for (var _i = 0, aces_1 = aces; _i < aces_1.length; _i++) {
            var ace = aces_1[_i];
            _loop_1(ace);
        }
    };
    return SdeditorComponent;
}());
__decorate([
    core_1.Input(),
    __metadata("design:type", String)
], SdeditorComponent.prototype, "aclString", void 0);
__decorate([
    core_1.Input(),
    __metadata("design:type", String)
], SdeditorComponent.prototype, "rootDn", void 0);
__decorate([
    core_1.Input(),
    __metadata("design:type", Array)
], SdeditorComponent.prototype, "usersArr", void 0);
__decorate([
    core_1.Input(),
    __metadata("design:type", Array)
], SdeditorComponent.prototype, "groupsArr", void 0);
__decorate([
    core_1.Output(),
    __metadata("design:type", core_1.EventEmitter)
], SdeditorComponent.prototype, "notify", void 0);
SdeditorComponent = __decorate([
    core_1.Component({
        moduleId: module.id,
        selector: 'sdeditor',
        templateUrl: './sdeditor.component.html',
    }),
    __metadata("design:paramtypes", [vmdir_service_1.VmdirService, sddlparser_1.SDDLParser])
], SdeditorComponent);
exports.SdeditorComponent = SdeditorComponent;
//# sourceMappingURL=sdeditor.component.js.map