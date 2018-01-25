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
var vmdirschema_service_1 = require("./vmdirschema.service");
var vmdir_utils_1 = require("./vmdir.utils");
var clarity_angular_1 = require("clarity-angular");
var ObjectAddComponent = (function () {
    function ObjectAddComponent(vmdirUtils, vmdirSchemaService, vmdirService) {
        this.vmdirUtils = vmdirUtils;
        this.vmdirSchemaService = vmdirSchemaService;
        this.vmdirService = vmdirService;
        this.notify = new core_1.EventEmitter();
        this.sel = [];
        this.classNameFilter = new ClassNameFilter();
        this.auxNameFilter = new ClassNameFilter();
        this.mayAttrMap = {};
        this.mustAttrMap = {};
        this.signPostObj = [];
        this.curSchemaMap = {};
        this.mayAttrArr = [];
        this.allDone = false;
        this.mustAttrArr = [];
        this.showCancelConfirm = false;
        this.isOpen = true;
        this.classesArr = [];
        this.classNames = [];
        this.auxClassNames = [];
        this.classMap = {};
        this.auxClassMap = {};
        this.stage = '';
    }
    ObjectAddComponent.prototype.ngOnInit = function () {
        this.decodedRootDn = decodeURIComponent(this.rootDn);
        console.log(this.rootDn);
        this.classNamesObs = this.getAllStructuralObjectClasses();
    };
    ObjectAddComponent.prototype.handleClassSelection = function (selClass) {
        if (selClass) {
            this.mayAttrArr = [];
            this.mustAttrArr = [];
            this.mayAttrMap = {};
            this.mustAttrMap = {};
            this.prepareAuxClasses(selClass);
            this.getAllAttributes(selClass);
            this.wizard.next();
        }
    };
    ObjectAddComponent.prototype.handleClose = function () {
        this.showCancelConfirm = false;
        this.wizard.close();
        this.notify.emit('done');
    };
    ObjectAddComponent.prototype.doCancel = function () {
        if (confirm("Are you sure you want to close the add object wizard?")) {
            this.handleClose();
        }
    };
    ObjectAddComponent.prototype.accumilateAuxClasses = function (classObj) {
        if (classObj && classObj.auxClass) {
            for (var _i = 0, _a = classObj.auxClass; _i < _a.length; _i++) {
                var cls = _a[_i];
                this.auxClassMap[cls] = true;
            }
        }
    };
    ObjectAddComponent.prototype.prepareAuxClasses = function (className) {
        var classObj = this.classMap[className];
        this.accumilateAuxClasses(classObj);
        this.getAllAttributes('top');
        while (classObj && classObj.subClassOf != 'top') {
            classObj = this.classMap[classObj.subClassOf];
            this.getAllAttributes(classObj.subClassOf);
            this.accumilateAuxClasses(classObj);
        }
        this.mustAttrMap['objectClass'] = className;
        this.mustAttrArr.push('objectClass');
        this.auxClassNames = Object.keys(this.auxClassMap);
        this.stage = 'showAuxClass';
    };
    ObjectAddComponent.prototype.handleMustAttr = function () {
        var unfilledAttr = false;
        for (var i = 0; i < this.mustAttrArr.length; i++) {
            if (!this.mustAttrMap[this.mustAttrArr[i]].length) {
                unfilledAttr = true;
                break;
            }
        }
        if (!unfilledAttr) {
            this.wizard.next();
        }
    };
    ObjectAddComponent.prototype.createObject = function () {
        var _this = this;
        var res;
        var cn = this.mustAttrMap['cn'].length ? this.mustAttrMap['cn'] : this.mayAttrMap['cn'];
        if (cn.length) {
            var newRootDn = 'cn=' + cn + ',' + this.decodedRootDn;
            this.vmdirService.addNewObject(newRootDn, this.mustAttrMap, this.mayAttrMap)
                .subscribe(function (result) {
                res = result;
                _this.handleClose();
            }, function (error) {
                console.log(error);
            });
        }
    };
    ObjectAddComponent.prototype.handleAuxClassSel = function (auxClasses) {
        for (var _i = 0, auxClasses_1 = auxClasses; _i < auxClasses_1.length; _i++) {
            var aux = auxClasses_1[_i];
            this.getAllAttributes(aux);
            this.mustAttrMap['objectClass'] += ',' + (aux);
        }
        this.wizard.next();
    };
    ObjectAddComponent.prototype.populateAttribMap = function (schemaObj) {
        var attrArr = schemaObj.result[0].attributes;
        for (var _i = 0, attrArr_1 = attrArr; _i < attrArr_1.length; _i++) {
            var attr = attrArr_1[_i];
            if (attr.type == 'systemMayContain' || attr.type == 'mayContain') {
                for (var _a = 0, _b = attr.value; _a < _b.length; _a++) {
                    var may = _b[_a];
                    if (!(may in this.mayAttrMap)) {
                        this.mayAttrArr.push(may);
                        this.mayAttrMap[may] = '';
                    }
                }
            }
            if (attr.type == 'systemMustContain' || attr.type == 'mustContain') {
                for (var _c = 0, _d = attr.value; _c < _d.length; _c++) {
                    var must = _d[_c];
                    if (!(must in this.mustAttrMap)) {
                        this.mustAttrMap[must] = '';
                        if (must != 'nTSecurityDescriptor') {
                            this.mustAttrArr.push(must);
                        }
                    }
                }
            }
        }
    };
    ObjectAddComponent.prototype.displayProperties = function (propName) {
        var _this = this;
        this.signPostObj = [];
        var schemaObj;
        this.vmdirSchemaService.getSchema(propName)
            .subscribe(function (schema) {
            schemaObj = schema;
            _this.signPostObj = schemaObj.result[0].attributes;
            /* remove stuff that need not be displayed */
            _this.signPostObj.splice(0, 2);
            _this.signPostObj.pop();
            _this.signPostObj.pop();
            _this.vmdirUtils.setAttributeDataType(_this.signPostObj);
            for (var _i = 0, _a = _this.signPostObj; _i < _a.length; _i++) {
                var schema_1 = _a[_i];
                _this.curSchemaMap[schema_1.type] = schema_1.value;
            }
            console.log(_this.curSchemaMap);
        }, function (error) {
        });
    };
    ObjectAddComponent.prototype.getAllAttributes = function (className) {
        var _this = this;
        var schemaObj;
        this.vmdirSchemaService.getSchema(className)
            .subscribe(function (schema) {
            schemaObj = schema;
            _this.populateAttribMap(schemaObj);
            console.log(className);
            console.log(schemaObj);
        }, function (error) {
        });
    };
    ObjectAddComponent.prototype.constructClassMap = function () {
        var cn;
        for (var _i = 0, _a = this.classesArr; _i < _a.length; _i++) {
            var cls = _a[_i];
            var obj = {};
            for (var _b = 0, _c = cls.attributes; _b < _c.length; _b++) {
                var attr = _c[_b];
                if (attr.type == 'cn') {
                    cn = attr.value[0];
                }
                else if (attr.type == 'objectClassCategory') {
                    obj.objectClassCategory = attr.value[0];
                }
                else if (attr.type == 'auxiliaryClass') {
                    obj.auxClass = attr.value;
                }
                else if (attr.type == 'subClassOf') {
                    obj.subClassOf = attr.value[0];
                }
                else if (attr.type == 'systemMayContain') {
                    obj.sysMayContain = attr.value;
                }
                else if (attr.type == 'systemMustContain') {
                    obj.sysMustContain = attr.value;
                }
                else if (attr.type == 'mayContain') {
                    obj.mayContain = attr.value;
                }
                else if (attr.type == 'mustContain') {
                    obj.mustContain = attr.value;
                }
            }
            this.classMap[cn] = obj;
            if (obj.objectClassCategory == '1') {
                this.classNames.push(cn);
            }
        }
        console.log(this.classMap);
        console.log(this.classNames);
    };
    ObjectAddComponent.prototype.getAllStructuralObjectClasses = function () {
        var _this = this;
        var res;
        return this.vmdirService.getAllStrObjectClasses()
            .map(function (result) {
            console.log(result);
            res = result;
            _this.classesArr = result.result;
            _this.constructClassMap();
            return _this.classNames;
        }).first();
    };
    return ObjectAddComponent;
}());
__decorate([
    core_1.Input(),
    __metadata("design:type", String)
], ObjectAddComponent.prototype, "rootDn", void 0);
__decorate([
    core_1.Output(),
    __metadata("design:type", core_1.EventEmitter)
], ObjectAddComponent.prototype, "notify", void 0);
__decorate([
    core_1.ViewChild("wizard"),
    __metadata("design:type", clarity_angular_1.Wizard)
], ObjectAddComponent.prototype, "wizard", void 0);
__decorate([
    core_1.ViewChild("pageThree"),
    __metadata("design:type", clarity_angular_1.WizardPage)
], ObjectAddComponent.prototype, "pageMayAttr", void 0);
__decorate([
    core_1.ViewChild("pageFive"),
    __metadata("design:type", clarity_angular_1.WizardPage)
], ObjectAddComponent.prototype, "pageMustAttr", void 0);
__decorate([
    core_1.ViewChild("pageFive"),
    __metadata("design:type", clarity_angular_1.WizardPage)
], ObjectAddComponent.prototype, "pageConfirm", void 0);
ObjectAddComponent = __decorate([
    core_1.Component({
        moduleId: module.id,
        selector: 'objectadd',
        templateUrl: './objectadd.component.html',
    }),
    __metadata("design:paramtypes", [vmdir_utils_1.VmdirUtils, vmdirschema_service_1.VmdirSchemaService, vmdir_service_1.VmdirService])
], ObjectAddComponent);
exports.ObjectAddComponent = ObjectAddComponent;
var ClassNameFilter = (function () {
    function ClassNameFilter() {
    }
    ClassNameFilter.prototype.accepts = function (className, search) {
        return className.toLowerCase().indexOf(search) >= 0;
    };
    return ClassNameFilter;
}());
exports.ClassNameFilter = ClassNameFilter;
//# sourceMappingURL=objectadd.component.js.map