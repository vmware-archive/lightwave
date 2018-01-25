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
var __param = (this && this.__param) || function (paramIndex, decorator) {
    return function (target, key) { decorator(target, key, paramIndex); }
};
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = require("@angular/core");
var vmdir_service_1 = require("./vmdir.service");
var vmdir_utils_1 = require("./vmdir.utils");
var vmdirschema_service_1 = require("./vmdirschema.service");
var auth_service_1 = require("./auth.service");
var utils_service_1 = require("./utils.service");
var platform_browser_1 = require("@angular/platform-browser");
var customstringfilter_1 = require("./customstringfilter");
var VmdirComponent = (function () {
    function VmdirComponent(renderer, utilsService, vmdirUtils, document, vmdirSchemaService, vmdirService, authService) {
        this.renderer = renderer;
        this.utilsService = utilsService;
        this.vmdirUtils = vmdirUtils;
        this.document = document;
        this.vmdirSchemaService = vmdirSchemaService;
        this.vmdirService = vmdirService;
        this.authService = authService;
        this.error = '';
        this.mustAttrFilter = new customstringfilter_1.AttributeFilter();
        this.mayAttrFilter = new customstringfilter_1.AttributeFilter();
        this.defaultActive = 'true';
        console.log("App component - vmdir");
        this.isEdited = false;
        this.confirm = false;
        this.confirmDel = false;
        this.showErrorAlert = false;
        this.showSuccessAlert = false;
        this.isListEdited = false;
        this.isSdEdited = false;
        this.isDateEdited = false;
        this.showSignPost = true;
    }
    VmdirComponent.prototype.ngOnInit = function () {
        this.attribsMap = new Map();
        this.updatedAttributes = new Map();
        this.schemaMap = new Map();
        this.curSchemaMap = new Map();
        this.schemaMayAttribsArr = new Array();
        this.schemaMustAttribsArr = new Array();
        this.setSchemaMustAttribsArr = new Array();
        this.setSchemaMayAttribsArr = new Array();
        this.usersArr = new Array();
        this.groupsArr = new Array();
        this.getDirListing();
        this.rootDN = this.authService.getRootDN();
        this.header = this.rootDN;
        this.getRootAttributes();
        this.containerHeight = (90 / 100 * window.innerHeight);
        this.cardHeight = (85 / 100 * this.containerHeight);
        this.datagridHeight = (90 / 100 * this.cardHeight);
        this.treeHeight = (80 / 100 * this.containerHeight);
        /*if(window.outerHeight < 900){
            this.cardHeight = (75/100*window.outerHeight);
        }*/
        console.log(this.cardHeight);
        this.getUsersAndGroups();
    };
    VmdirComponent.prototype.getRootAttributes = function () {
        this.selectedDN = this.authService.getRootDnQuery();
        this.getAttributes();
    };
    VmdirComponent.prototype.onSdNotify = function (message) {
        this.isSdEdited = false;
        this.getACLString();
    };
    VmdirComponent.prototype.onAddNotify = function (message) {
        this.isAddObject = false;
    };
    VmdirComponent.prototype.prepareUpdatedAttributesList = function () {
        var result;
        this.noUpdatesYet = true;
        this.updatedAttributes = new Map();
        // find dirty input fields
        for (var _i = 0, _a = this.setSchemaMayAttribsArr; _i < _a.length; _i++) {
            var attr = _a[_i];
            // test if a value exists originally
            if (this.attribsMapCopy[attr.attrType]) {
                var origValue = this.attribsMapCopy[attr.attrType].toString();
                var newValue = attr.value.toString();
                if (origValue != newValue) {
                    this.updatedAttributes[attr.attrType] = attr;
                }
            }
            else if (attr.value && attr.value.toString().length > 0) {
                this.updatedAttributes[attr.attrType] = attr;
            }
        }
        for (var _b = 0, _c = this.setSchemaMustAttribsArr; _b < _c.length; _b++) {
            var attr = _c[_b];
            if (this.attribsMapCopy[attr.attrType]) {
                var origValue = this.attribsMapCopy[attr.attrType].toString();
                var newValue = attr.value.toString();
                if (origValue != newValue) {
                    this.updatedAttributes[attr.attrType] = attr;
                }
            }
            else {
                this.updatedAttributes[attr.attrType] = attr;
            }
        }
        this.updatedAttributesArr = Object.keys(this.updatedAttributes);
        if (this.updatedAttributesArr.length > 0) {
            this.noUpdatesYet = false;
        }
        console.log(this.updatedAttributesArr);
    };
    VmdirComponent.prototype.constructUsersArr = function (usersObjArr) {
        for (var i = 0; i < usersObjArr.length;) {
            for (var j = 0; j < usersObjArr[i].attributes.length; j++) {
                if (usersObjArr[i].attributes[j].type == 'cn') {
                    usersObjArr[i].cn = usersObjArr[i].attributes[j].value[0];
                }
                else if (usersObjArr[i].attributes[j].type == 'objectClass') {
                    usersObjArr[i].objectClass = usersObjArr[i].attributes[j].value;
                }
                else if (usersObjArr[i].attributes[j].type == 'objectSid') {
                    usersObjArr[i].objectSid = usersObjArr[i].attributes[j].value;
                }
            }
            if (usersObjArr[i].cn) {
                if (usersObjArr[i].objectClass[0] == 'group') {
                    this.groupsArr.push(usersObjArr[i]);
                }
                else {
                    this.usersArr.push(usersObjArr[i]);
                }
            }
            i++;
        }
        console.log(this.usersArr);
        console.log(this.groupsArr);
    };
    VmdirComponent.prototype.handleEdit = function () {
        this.setSchemaMustAttribsArr = this.setSchemaMustAttribsArr.concat(this.schemaMustAttribsArr);
        this.setSchemaMayAttribsArr = this.setSchemaMayAttribsArr.concat(this.schemaMayAttribsArr);
    };
    VmdirComponent.prototype.getUsersAndGroups = function () {
        var _this = this;
        var result;
        // get all user and group accounts
        this.vmdirService.getAllUsersAndGroups().
            subscribe(function (result) {
            result.result.splice(0, 1);
            console.log(result);
            _this.constructUsersArr(result.result);
        }, function (error) {
            console.log(error);
        });
    };
    VmdirComponent.prototype.handleUpdateError = function (error) {
        console.log(error);
        this.errorMsg = "Unknown error";
        if (error.error_message) {
            this.errorMsg = error.error_message;
        }
        this.showErrorAlert = true;
    };
    VmdirComponent.prototype.handleDateUpdate = function () {
        console.log(this.setDate);
        var dateStr = this.setDate.toISOString();
        dateStr = dateStr.replace(/[-:T]/g, '');
        if (this.formReq) {
            this.formReq['controls'][this.curSchema].setValue(dateStr);
        }
    };
    VmdirComponent.prototype.updateView = function (valueArr) {
        var l = valueArr.length;
        if (l > 0 && valueArr[l - 1].length == 0) {
            valueArr.pop();
        }
        if (this.document.getElementById(this.curSchema).value != this.curSchemaValue.toString()) {
            this.formReq.form.markAsDirty();
        }
        this.document.getElementById(this.curSchema).value = this.curSchemaValue;
        this.isListEdited = false;
    };
    VmdirComponent.prototype.trackByFn = function (index, item) {
        return index;
    };
    VmdirComponent.prototype.handleListEditCompletion = function (valueArr) {
        console.log(valueArr);
    };
    VmdirComponent.prototype.submitAttributeUpdates = function () {
        var _this = this;
        var result;
        this.vmdirService.updateAttributes(this.selectedDN, this.attribsMapCopy, this.updatedAttributesArr, this.updatedAttributes, this.schemaMap)
            .subscribe(function (result) {
            console.log(result);
            _this.getAttributes();
            _this.isEdited = false;
        }, function (error) {
            _this.handleUpdateError(error);
        });
    };
    VmdirComponent.prototype.handleListAdd = function (array) {
        var l = array.length;
        if ((l > 0 && array[l - 1].length > 0) || l == 0) {
            array.push('');
        }
    };
    VmdirComponent.prototype.handleListRemoval = function (valArray, index) {
        valArray.splice(index, 1);
    };
    VmdirComponent.prototype.onChildSelected = function (childDN) {
        console.log(childDN);
        this.selectedDN = childDN;
        this.header = decodeURIComponent(this.selectedDN);
        this.getAttributes();
    };
    VmdirComponent.prototype.displayProperties = function (attrType, arrId, attrValue) {
        this.getSchema(attrType, true);
        this.curSchema = attrType;
        this.curSchemaValue = attrValue;
    };
    VmdirComponent.prototype.constructAttribsMap = function (attribsArr) {
        this.attribsMap = new Map();
        var id = 0;
        for (var _i = 0, attribsArr_1 = attribsArr; _i < attribsArr_1.length; _i++) {
            var attr = attribsArr_1[_i];
            if (attr.type == 'objectClass') {
                attr.value.push('top');
            }
            if (attr.type == 'nTSecurityDescriptor') {
                attribsArr.splice(id, 1);
            }
            if (attr.type.includes('TimeStamp')) {
                if (attr.value && attr.value[0]) {
                    var dateObj = new Date(this.utilsService.toValidTimeStr(attr.value[0]));
                    attr.value = [dateObj.toString()];
                }
            }
            this.attribsMap[attr.type] = attr.value;
            id++;
        }
        console.log(this.attribsMap);
    };
    /* store the schema(single or multi valued) for updated fields and use it while submitting changes*/
    VmdirComponent.prototype.storeSchema = function (cn) {
        if (!this.schemaMap[cn]) {
            for (var _i = 0, _a = this.signPostObj; _i < _a.length; _i++) {
                var schema = _a[_i];
                this.curSchemaMap[schema.type] = schema.value;
            }
            this.schemaMap[cn] = this.signPostObj[0];
            console.log(this.schemaMap);
        }
    };
    VmdirComponent.prototype.getAttributes = function () {
        var _this = this;
        this.formReq.form.markAsPristine();
        this.updatedAttributes = new Map();
        this.updatedAttributesArr = [];
        this.schemaMap.clear();
        this.showSuccessAlert = false;
        this.isEdited = false;
        this.showErrorAlert = false;
        var attribsObj;
        this.header = decodeURIComponent(this.selectedDN);
        this.vmdirService.getAttributes(this.selectedDN)
            .subscribe(function (attribs) {
            _this.attribs = JSON.stringify(attribs);
            attribsObj = JSON.parse(_this.attribs);
            //this.Attributes(this.listingObj.attributes);
            console.log(attribsObj.result[0].attributes);
            _this.attribsArr = attribsObj.result[0].attributes;
            _this.constructAttribsMap(_this.attribsArr);
            _this.getAllSchemas();
        }, function (error) { return _this.error = error; });
        this.getACLString();
    };
    VmdirComponent.prototype.getACLString = function () {
        var _this = this;
        var sddlString = '';
        var sddlObj;
        this.vmdirService.getACLString(this.selectedDN)
            .subscribe(function (aclstring) {
            sddlString = JSON.stringify(aclstring);
            sddlObj = JSON.parse(sddlString).result[0];
            console.log(sddlObj);
            _this.aclString = sddlObj.attributes[0].value[0];
            console.log(_this.aclString);
        });
    };
    VmdirComponent.prototype.deleteEntry = function () {
        var _this = this;
        console.log(this.selectedDN);
        this.vmdirService.delete(this.selectedDN)
            .subscribe(function (result) {
            console.log(result);
            _this.getDirListing();
            /* refresh the directory tree at parent ?*/
        }, function (error) { return _this.error = error; });
    };
    VmdirComponent.prototype.getAllSchemas = function () {
        this.schemaMustAttribsArr = [];
        this.schemaMayAttribsArr = [];
        this.setSchemaMustAttribsArr = [];
        this.setSchemaMayAttribsArr = [];
        for (var _i = 0, _a = this.attribsMap['objectClass']; _i < _a.length; _i++) {
            var schemaType = _a[_i];
            this.getSchema(schemaType, false);
        }
        this.attribsMapCopy = JSON.parse(JSON.stringify(this.attribsMap));
    };
    VmdirComponent.prototype.deduplicateArray = function (arr) {
        for (var i = 0; i < arr.length; ++i) {
            for (var j = i + 1; j < arr.length; ++j) {
                if (arr[i].attrType === arr[j].attrType) {
                    arr.splice(j--, 1);
                }
            }
        }
    };
    VmdirComponent.prototype.constructSchemaAttribsArr = function (attributes) {
        for (var _i = 0, attributes_1 = attributes; _i < attributes_1.length; _i++) {
            var attr = attributes_1[_i];
            if (attr.type == 'mayContain' || attr.type == 'systemMayContain') {
                for (var i = 0; i < attr.value.length; i++) {
                    var o = {};
                    o.attrType = attr.value[i];
                    if (this.attribsMap[o.attrType]) {
                        o.value = this.attribsMap[o.attrType];
                        this.setSchemaMayAttribsArr.push(o);
                    }
                    else {
                        o.value = [];
                        this.schemaMayAttribsArr.push(o);
                    }
                }
            }
            else if (attr.type == 'mustContain' || attr.type == 'systemMustContain') {
                for (var i = 0; i < attr.value.length; i++) {
                    var o = {};
                    o.attrType = attr.value[i];
                    if (this.attribsMap[o.attrType] && o.attrType != 'nTSecurityDescriptor') {
                        o.value = this.attribsMap[o.attrType];
                        this.setSchemaMustAttribsArr.push(o);
                    }
                }
            }
        }
    };
    VmdirComponent.prototype.constructSchemaMap = function (schemaName, schemaArr) {
        this.curSchemaMap = new Map();
        for (var _i = 0, schemaArr_1 = schemaArr; _i < schemaArr_1.length; _i++) {
            var schema = schemaArr_1[_i];
            this.curSchemaMap[schema.type] = schema.value;
        }
        this.schemaMap[schemaName] = this.curSchemaMap;
        console.log(this.schemaMap);
    };
    VmdirComponent.prototype.setBooleanElement = function (schemaName) {
        var _this = this;
        var selectElem = this.renderer.createElement('select');
        var inputElem = this.document.getElementById(schemaName);
        inputElem.style.display = 'none';
        var parent = inputElem.parentElement;
        var optionTrue = this.renderer.createElement('option');
        var optionFalse = this.renderer.createElement('option');
        this.renderer.setAttribute(optionTrue, "value", "TRUE");
        this.renderer.appendChild(optionTrue, this.renderer.createText('true'));
        this.renderer.setAttribute(optionFalse, "value", "FALSE");
        this.renderer.appendChild(optionFalse, this.renderer.createText('false'));
        this.renderer.listen(selectElem, 'change', function () {
            if (_this.formReq) {
                _this.formReq['controls'][schemaName].setValue(selectElem.value);
            }
        });
        if (inputElem.value.length == 0) {
            var optionUnset = this.renderer.createElement('option');
            this.renderer.appendChild(selectElem, optionUnset);
        }
        else {
            if (inputElem.value == 'FALSE') {
                this.renderer.setAttribute(optionFalse, "selected", "selected");
            }
            else if (inputElem.value == 'TRUE') {
                this.renderer.setAttribute(optionTrue, "selected", "selected");
            }
        }
        this.renderer.appendChild(selectElem, optionTrue);
        this.renderer.appendChild(selectElem, optionFalse);
        this.renderer.setAttribute(selectElem, 'value', inputElem.value);
        this.renderer.insertBefore(parent, selectElem, inputElem);
        console.log(inputElem);
        console.log(selectElem);
    };
    VmdirComponent.prototype.setDateTimeElement = function (schemaName) {
        var inputElem = this.document.getElementById('dateInput');
        inputElem.focus();
    };
    VmdirComponent.prototype.getSchema = function (schemaName, forSignPost) {
        var _this = this;
        this.showSignPost = true;
        console.log("getScemas - component");
        this.schemaMap.clear();
        this.vmdirSchemaService.getSchema(schemaName)
            .subscribe(function (schema) {
            _this.schema = JSON.stringify(schema);
            _this.schemaObj = JSON.parse(_this.schema);
            //this.Attributes(this.listingObj.attributes);
            console.log(_this.schemaObj);
            if (!forSignPost) {
                _this.constructSchemaAttribsArr(_this.schemaObj.result[0].attributes);
                _this.deduplicateArray(_this.schemaMustAttribsArr);
                _this.deduplicateArray(_this.schemaMayAttribsArr);
                _this.deduplicateArray(_this.setSchemaMustAttribsArr);
                _this.deduplicateArray(_this.setSchemaMayAttribsArr);
                console.log(_this.schemaMayAttribsArr);
                console.log(_this.schemaMustAttribsArr);
                console.log(_this.setSchemaMustAttribsArr);
                console.log(_this.setSchemaMayAttribsArr);
            }
            else {
                _this.signPostObj = {};
                _this.signPostObj = _this.schemaObj.result[0].attributes;
                /* remove stuff that need not be displayed */
                _this.signPostObj.splice(0, 2);
                _this.signPostObj.pop();
                _this.signPostObj.pop();
                _this.vmdirUtils.setAttributeDataType(_this.signPostObj);
                _this.constructSchemaMap(schemaName, _this.signPostObj);
                /* Handle multi value attributes */
                if (_this.isEdited && 'FALSE' == _this.curSchemaMap['isSingleValued'][0]) {
                    _this.isListEdited = true;
                }
                /* Handle Boolean Attributes */
                if (_this.isEdited && 'ADSTYPE_BOOLEAN' == _this.curSchemaMap['dataType']) {
                    _this.setBooleanElement(schemaName);
                }
                /* Handle Editing security descriptor */
                if (_this.isEdited && 'ADSTYPE_NT_SECURITY_DESCRIPTOR' == _this.curSchemaMap['dataType']) {
                    _this.isSdEdited = true;
                    _this.showSignPost = false;
                }
                /* Handle Date Tiime attributes */
                if (_this.isEdited && 'ADSTYPE_UTC_TIME' == _this.curSchemaMap['dataType']) {
                    _this.isDateEdited = true;
                    //this.setDateTimeElement(schemaName);
                }
                console.log(_this.signPostObj);
            }
        }, function (error) { return _this.error = error; });
    };
    VmdirComponent.prototype.getDirListing = function () {
        var _this = this;
        console.log("getDirListing - app component");
        this.vmdirService.getDirListing(null)
            .subscribe(function (listing) {
            _this.listing = JSON.stringify(listing);
            _this.listingObj = JSON.parse(_this.listing);
            _this.utilsService.extractName(_this.listingObj.result, true);
            console.log(_this.listingObj);
        }, function (error) { return _this.error = error; });
    };
    return VmdirComponent;
}());
__decorate([
    core_1.ViewChild('vmdirForm'),
    __metadata("design:type", Object)
], VmdirComponent.prototype, "formReq", void 0);
VmdirComponent = __decorate([
    core_1.Component({
        moduleId: module.id,
        selector: 'vmdir',
        templateUrl: './vmdir.component.html',
    }),
    __param(3, core_1.Inject(platform_browser_1.DOCUMENT)),
    __metadata("design:paramtypes", [core_1.Renderer2, utils_service_1.UtilsService, vmdir_utils_1.VmdirUtils, Object, vmdirschema_service_1.VmdirSchemaService, vmdir_service_1.VmdirService, auth_service_1.AuthService])
], VmdirComponent);
exports.VmdirComponent = VmdirComponent;
var LazyLoadedLevel1Component = (function () {
    function LazyLoadedLevel1Component(utilsService, vmdirService) {
        this.utilsService = utilsService;
        this.vmdirService = vmdirService;
        this.onChildSelected = new core_1.EventEmitter();
        console.log("App component - vmdir");
    }
    LazyLoadedLevel1Component.prototype.ngOnInit = function () {
        this.loading = true;
        // This would be a call to your service that communicates with the server
        console.log('------->oninit-lazyloadedcomponent' + this.rootDn);
        this.getDirListing();
        //        console.log(this.listingObjObs);
    };
    LazyLoadedLevel1Component.prototype.onRecChildSelected = function (childDN) {
        console.log("recursively calling emit");
        this.onChildSelected.emit(childDN);
    };
    LazyLoadedLevel1Component.prototype.getAttributes = function () {
        console.log("getAttributes - component");
        var attribsObj;
        this.onChildSelected.emit(this.selectedDN);
    };
    LazyLoadedLevel1Component.prototype.getDirListing = function () {
        var _this = this;
        console.log("-------->getting child compoment");
        this.loading = true;
        this.vmdirService.getDirListing(encodeURIComponent(this.rootDn))
            .subscribe(function (listing) {
            _this.listing = JSON.stringify(listing);
            _this.listingObj = JSON.parse(_this.listing);
            _this.utilsService.extractName(_this.listingObj.result, false);
            _this.listArr = _this.listingObj.result;
            _this.loading = false;
        }, function (error) { return _this.error = error; });
    };
    return LazyLoadedLevel1Component;
}());
__decorate([
    core_1.Input(),
    __metadata("design:type", String)
], LazyLoadedLevel1Component.prototype, "rootDn", void 0);
__decorate([
    core_1.Output(),
    __metadata("design:type", Object)
], LazyLoadedLevel1Component.prototype, "onChildSelected", void 0);
LazyLoadedLevel1Component = __decorate([
    core_1.Component({
        moduleId: module.id,
        selector: "lazy-loaded-level1",
        template: "\n        <ng-template #loadingChild>\n            <clr-tree-node>\n                <div align=\"center\">\n                <span class=\"spinner spinner-inline\">\n                   Loading...\n                </span>\n                <span>\n                    Loading subtree...\n                </span>\n                </div>\n            </clr-tree-node>\n        </ng-template>\n            <ng-container [clrLoading]=\"loading\">\n                <clr-tree-node *ngFor=\"let commonName of listArr\">\n                    <button\n                        (click)=\"selectedDN=commonName.encodedDN ;getAttributes()\"\n                         class=\"clr-treenode-link\">\n                               {{commonName.displayName}}\n                    </button>\n                    <ng-template clrIfExpanded>\n                        <lazy-loaded-level1 [(rootDn)]=commonName.dn (onChildSelected)=\"onRecChildSelected($event)\"></lazy-loaded-level1>\n                    </ng-template>\n                </clr-tree-node>\n            </ng-container>\n       <!--/span-->\n    "
    }),
    __metadata("design:paramtypes", [utils_service_1.UtilsService, vmdir_service_1.VmdirService])
], LazyLoadedLevel1Component);
exports.LazyLoadedLevel1Component = LazyLoadedLevel1Component;
//# sourceMappingURL=vmdir.component.js.map