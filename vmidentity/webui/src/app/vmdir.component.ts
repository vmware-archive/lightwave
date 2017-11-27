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

import { ViewChild, Renderer2, EventEmitter, Output, Component, OnInit, Input, Inject, Injectable } from '@angular/core';
import { NgModel } from '@angular/forms';
import { VmdirService } from './vmdir.service';
import { VmdirUtils } from './vmdir.utils';
import { VmdirSchemaService } from './vmdirschema.service';
import { AuthService } from './auth.service';
import { UtilsService } from './utils.service';
import { Routes, RouterModule } from '@angular/router';
import { DOCUMENT } from '@angular/platform-browser';
import { Observable } from "rxjs/Rx";
import { SdeditorComponent } from './sdeditor.component';
import { AttributeFilter } from './customstringfilter';
interface AttribSchema {
    attrType:string;
    value:string;
}
@Component({
  moduleId: module.id,
  selector: 'vmdir',
  templateUrl: './vmdir.component.html',
})

export class VmdirComponent  {
    private error:any = '';
    listing: string;
    listingObj: any;
    setDate: any;
    schema: string;
    schemaObj: any;
    errorMsg:string;
    aclString:string;
    cardHeight:number;
    treeHeight:number;
    containerHeight:number;
    datagridHeight:number;
    isEdited:any;
    isAddObject:any;
    isDateEdited:boolean;
    showErrorAlert: boolean;
    showSignPost: boolean;
    isSdEdited: boolean;
    noUpdatesYet: boolean;
    confirmDel: boolean;
    mustAttrFilter: AttributeFilter = new AttributeFilter();
    mayAttrFilter: AttributeFilter = new AttributeFilter();
    showSuccessAlert: boolean;
    isListEdited: boolean;
    attribs: string;
    curSchema: string;
    curSchemaValue: string[];
    confirm: boolean;
    attribsMap:Map<string, any>;
    attribsMapCopy:any;
    schemaMap:Map<string, any>;
    defaultActive = 'true';
    attribsArr: any[];
    schemaMayAttribsArr: any[];
    schemaMustAttribsArr: any[];
    usersArr: any[];
    groupsArr: any[];
    signPostObj:any;
    curSchemaMap:Map<string, any>;
    setSchemaMustAttribsArr:any[];
    setSchemaMayAttribsArr:any[];
    rootDN:string;
    updatedAttributes:Map<string,any>;
    updatedAttributesArr:any[];
    selectedDN:string;
    header:string;
    @ViewChild('vmdirForm1') formReq: NgModel;
    @ViewChild('vmdirForm2') formOpt: NgModel;
    constructor(private renderer:Renderer2, private utilsService: UtilsService, private vmdirUtils: VmdirUtils, @Inject(DOCUMENT) private document: any, private vmdirSchemaService: VmdirSchemaService, private vmdirService: VmdirService, private authService: AuthService) {
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
    ngOnInit() {
        this.attribsMap = new Map<string,any[]>();
        this.updatedAttributes = new Map<string,any[]>();
        this.schemaMap = new Map<string,any[]>();
        this.curSchemaMap = new Map<string, any>();
        this.schemaMayAttribsArr = new Array<AttribSchema>();
        this.schemaMustAttribsArr = new Array<AttribSchema>();
        this.setSchemaMustAttribsArr = new Array<AttribSchema>();
        this.setSchemaMayAttribsArr = new Array<AttribSchema>();
        this.usersArr = new Array<any>();
        this.groupsArr = new Array<any>();
        this.getDirListing();
        this.rootDN = this.authService.getRootDN();
        this.header = this.rootDN;
        this.getRootAttributes();
        this.containerHeight = (90/100*window.innerHeight);
        this.cardHeight = (85/100*this.containerHeight);
        this.datagridHeight = (90/100*this.cardHeight);
        this.treeHeight = (80/100*this.containerHeight);
        /*if(window.outerHeight < 900){
            this.cardHeight = (75/100*window.outerHeight);
        }*/
        console.log(this.cardHeight);
        this.getUsersAndGroups();
    }


    getRootAttributes(){
        this.selectedDN = this.authService.getRootDnQuery();
        this.getAttributes();
    }
    onSdNotify(message:string){
        this.isSdEdited = false;
        this.getACLString();
    }

    onAddNotify(message:string){
        this.isAddObject = false;
    }
    prepareUpdatedAttributesList() {
        let result;
        this.noUpdatesYet = true;
        this.updatedAttributes = new Map<string, any>();
        // find dirty input fields
        for (let attr of this.setSchemaMayAttribsArr) {// test if original values are different from new values
                // test if a value exists originally
                if(this.attribsMapCopy[attr.attrType] ){
                    let origValue = this.attribsMapCopy[attr.attrType].toString();
                    let newValue = attr.value.toString();
                    if(origValue != newValue){
                        this.updatedAttributes[attr.attrType] = attr;
                    }
                }else if(attr.value && attr.value.toString().length > 0){
                    this.updatedAttributes[attr.attrType] = attr;
                }
        }
        for (let attr of this.setSchemaMustAttribsArr) {// test if original values are different from new values
             if(this.attribsMapCopy[attr.attrType] ){
                 let origValue = this.attribsMapCopy[attr.attrType].toString();
                 let newValue = attr.value.toString();
                 if(origValue != newValue){
                    this.updatedAttributes[attr.attrType] = attr;
                 }
             }else{
                 this.updatedAttributes[attr.attrType] = attr;
             }
        }
        this.updatedAttributesArr = Object.keys(this.updatedAttributes);
        if(this.updatedAttributesArr.length > 0) {
            this.noUpdatesYet = false;
        }
        console.log(this.updatedAttributesArr);
    }

    constructUsersArr(usersObjArr:any[]){
       for(var i = 0; i < usersObjArr.length; ){
           for(var j = 0; j < usersObjArr[i].attributes.length; j ++){
               if(usersObjArr[i].attributes[j].type == 'cn'){
                   usersObjArr[i].cn = usersObjArr[i].attributes[j].value[0];
               }else if(usersObjArr[i].attributes[j].type == 'objectClass'){
                   usersObjArr[i].objectClass = usersObjArr[i].attributes[j].value;
               }else if(usersObjArr[i].attributes[j].type == 'objectSid'){
                   usersObjArr[i].objectSid = usersObjArr[i].attributes[j].value;
               }
           }
           if(usersObjArr[i].cn){
               if(usersObjArr[i].objectClass[0] == 'group'){
                   this.groupsArr.push(usersObjArr[i]);
               }else{
                   this.usersArr.push(usersObjArr[i]);
               }
           }
           i ++;
       }
       console.log(this.usersArr);
       console.log(this.groupsArr);
    }

    handleEdit(){
       this.setSchemaMustAttribsArr = this.setSchemaMustAttribsArr.concat(this.schemaMustAttribsArr);
       this.setSchemaMayAttribsArr = this.setSchemaMayAttribsArr.concat(this.schemaMayAttribsArr);
    }
    getUsersAndGroups(){
        let result:any;
        // get all user and group accounts
        this.vmdirService.getAllUsersAndGroups().
            subscribe(
               result => {
                  result.result.splice(0,1);
                  console.log(result);
                  this.constructUsersArr(result.result);
               },
               error => {
                  console.log(error);
               }
            );
    }

    handleUpdateError(error) {
        console.log(error);
        this.errorMsg = "Unknown error";
        if(error.error_message) {
            this.errorMsg = error.error_message;
        }
        this.showErrorAlert = true;
    }

    handleDateUpdate(){
        console.log(this.setDate);
        let dateStr = this.setDate.toISOString();
        dateStr = dateStr.replace(/[-:T]/g, '');
        if(this.formReq) {
            this.formReq['controls'][this.curSchema].setValue(dateStr);
        }else if(this.formOpt) {
            this.formOpt['controls'][this.curSchema].setValue(dateStr);
        }
    }
    updateView(valueArr:any[]) {
        let l:number = valueArr.length;
        if(l > 0 && valueArr[l-1].length == 0){
            valueArr.pop();
        }
        this.document.getElementById(this.curSchema).value = this.curSchemaValue;
        this.isListEdited = false;
    }

    trackByFn(index: any, item: any) {
        return index;
    }

    handleListEditCompletion(valueArr:string[]){
        console.log(valueArr);
    }
    submitAttributeUpdates(){
        let result;
        this.vmdirService.updateAttributes(this.selectedDN, this.attribsMapCopy, this.updatedAttributesArr, this.updatedAttributes, this.schemaMap)
        .subscribe(
            result => {
                    console.log(result);
                    this.getAttributes();
                    this.isEdited = false;
                    },
            error => {
                         this.handleUpdateError(error);
                     });

    }
    handleListAdd(array:string[]) {
        let l:number = array.length;
        if((l > 0 && array[l-1].length > 0) || l == 0){
           array.push('');
        }
    }
    handleListRemoval(valArray:string[], index:number){
        valArray.splice(index, 1);
    }
    onChildSelected(childDN:string) {
        console.log(childDN);
        this.selectedDN = childDN;
        this.header = decodeURIComponent(this.selectedDN);
        this.getAttributes();
    }


   displayProperties(attrType:string, arrId:number, arr:any[], index:number) {
       this.getSchema(attrType, true);
       this.curSchema = attrType;
       this.curSchemaValue = arr[index].value;
   }

   constructAttribsMap(attribsArr:any[]){
        this.attribsMap = new Map<string, any>();
        for (let attr of attribsArr){
             if(attr.type == 'objectClass') {
                 attr.value.push('top');
             }
             if(attr.type.includes('TimeStamp')){
                 if(attr.value && attr.value[0]){
                     let dateObj = new Date(this.utilsService.toValidTimeStr(attr.value[0]));
                     attr.value = [dateObj.toString()];
                 }
             }
             this.attribsMap[attr.type] = attr.value;
        }
        console.log(this.attribsMap);
    }

    /* store the schema(single or multi valued) for updated fields and use it while submitting changes*/
    storeSchema(cn:string) {
         if(!this.schemaMap[cn]) {
             for(let schema of this.signPostObj) {
                 this.curSchemaMap[schema.type] = schema.value;
             }
             this.schemaMap[cn] = this.signPostObj[0]
             console.log(this.schemaMap);
         }
    }
    getAttributes() {
        console.log("getAttributes - component");
        this.updatedAttributes = new Map<string,any>();
        this.updatedAttributesArr = [];
        this.schemaMap.clear();
        this.showSuccessAlert = false;
        this.isEdited = false;
        this.showErrorAlert = false;
        let attribsObj:any;
        this.header = decodeURIComponent(this.selectedDN);
        this.vmdirService.getAttributes(this.selectedDN)
        .subscribe(
            attribs => {
                    this.attribs = JSON.stringify(attribs);
                    attribsObj = JSON.parse(this.attribs);
                    //this.Attributes(this.listingObj.attributes);
                    console.log(attribsObj.result[0].attributes);
                    this.attribsArr = attribsObj.result[0].attributes;
                    this.constructAttribsMap(this.attribsArr);
                    this.getAllSchemas();
                    },
            error => this.error = <any>error);
        this.getACLString();
    }

    getACLString() {
        let sddlString:string = '';
        let sddlObj:any;
        this.vmdirService.getACLString(this.selectedDN)
        .subscribe(
             aclstring => {
                              sddlString = JSON.stringify(aclstring);
                              sddlObj = JSON.parse(sddlString).result[0];
                              console.log(sddlObj);
                              this.aclString = sddlObj.attributes[0].value[0];
                              console.log(this.aclString);
                          });
    }

    deleteEntry() {
        console.log(this.selectedDN);
        this.vmdirService.delete(this.selectedDN)
        .subscribe(
            result => {
                    console.log(result);
                    this.getDirListing();
                    /* refresh the directory tree at parent ?*/
                    },
            error => this.error = <any>error);
    }
    getAllSchemas() {
        this.schemaMustAttribsArr = [];
        this.schemaMayAttribsArr = [];
        this.setSchemaMustAttribsArr = [];
        this.setSchemaMayAttribsArr = [];
        for (let schemaType of this.attribsMap['objectClass']) {
            this.getSchema(schemaType, false)
        }
        this.attribsMapCopy = JSON.parse(JSON.stringify(this.attribsMap));
    }
    deduplicateArray(arr: any[]) {
        for(var i=0; i<arr.length; ++i) {
            for(var j=i+1; j<arr.length; ++j) {
                 if(arr[i].attrType === arr[j].attrType) {
                     arr.splice(j--, 1);
                 }
            }
        }
    }
    constructSchemaAttribsArr(attributes:any[]) {

        for(let attr of attributes){
           if (attr.type == 'mayContain' || attr.type == 'systemMayContain') {
              for (var i = 0; i < attr.value.length; i ++) {
                     let o:any = {};
                     o.attrType = attr.value[i];
                     if(this.attribsMap[o.attrType]) {
                         o.value = this.attribsMap[o.attrType];
                         this.setSchemaMayAttribsArr.push(o);
                     }
                     else {
                         o.value = [];
                         this.schemaMayAttribsArr.push(o);
                     }
                  }
           } else if (attr.type == 'mustContain' || attr.type == 'systemMustContain') {
              for (var i = 0; i < attr.value.length; i ++) {
                     let o:any = {};
                     o.attrType = attr.value[i];
                     if(this.attribsMap[o.attrType]) {
                         o.value = this.attribsMap[o.attrType];
                         this.setSchemaMustAttribsArr.push(o);
                  } else {
                      o.value = [];
                      this.schemaMustAttribsArr.push(o);
                  }
              }
           }
       }
    }
    constructSchemaMap(schemaName:string, schemaArr:any[]){
        this.curSchemaMap = new Map<string, any>();
        for(let schema of schemaArr){
            this.curSchemaMap[schema.type] = schema.value;
        }
        this.schemaMap[schemaName] = this.curSchemaMap;
        console.log(this.schemaMap);
    }
    setBooleanElement(schemaName:string){
        let selectElem = this.renderer.createElement('select');
        let inputElem = this.document.getElementById(schemaName);
        inputElem.style.display = 'none';
        let parent = inputElem.parentElement;
        let optionTrue = this.renderer.createElement('option');
        let optionFalse = this.renderer.createElement('option');
        this.renderer.setAttribute(optionTrue, "value", "TRUE");
        this.renderer.appendChild(optionTrue, this.renderer.createText('true'));
        this.renderer.setAttribute(optionFalse, "value", "FALSE");
        this.renderer.appendChild(optionFalse, this.renderer.createText('false'));
        this.renderer.listen(selectElem, 'change', () => {
            if(this.formReq) {
                this.formReq['controls'][schemaName].setValue(selectElem.value);
            }else if(this.formOpt) {
                this.formOpt['controls'][schemaName].setValue(selectElem.value);
            }
        });
        if(inputElem.value.length == 0){
            let optionUnset = this.renderer.createElement('option');
            this.renderer.appendChild(selectElem, optionUnset);
        }else{
            if(inputElem.value == 'FALSE'){
                this.renderer.setAttribute(optionFalse, "selected", "selected");
            }else if(inputElem.value == 'TRUE'){
                this.renderer.setAttribute(optionTrue, "selected", "selected");
            }
        }
        this.renderer.appendChild(selectElem, optionTrue);
        this.renderer.appendChild(selectElem, optionFalse);
        this.renderer.setAttribute(selectElem, 'value', inputElem.value);
        this.renderer.insertBefore(parent, selectElem, inputElem);
        console.log(inputElem);
        console.log(selectElem);
    }
    setDateTimeElement(schemaName:string){
        let inputElem = this.document.getElementById('dateInput');
        inputElem.focus();
    }
    getSchema(schemaName:string, forSignPost:boolean) {
        this.showSignPost = true;
        console.log("getScemas - component");
        this.schemaMap.clear();
        this.vmdirSchemaService.getSchema(schemaName)
        .subscribe(
            schema => {
                    this.schema = JSON.stringify(schema);
                    this.schemaObj = JSON.parse(this.schema);
                    //this.Attributes(this.listingObj.attributes);
                    console.log(this.schemaObj);
                    if(!forSignPost) {
                        this.constructSchemaAttribsArr(this.schemaObj.result[0].attributes);
                        this.deduplicateArray(this.schemaMustAttribsArr);
                        this.deduplicateArray(this.schemaMayAttribsArr);
                        this.deduplicateArray(this.setSchemaMustAttribsArr);
                        this.deduplicateArray(this.setSchemaMayAttribsArr);
                        console.log(this.schemaMayAttribsArr);
                        console.log(this.schemaMustAttribsArr);
                        console.log(this.setSchemaMustAttribsArr);
                        console.log(this.setSchemaMayAttribsArr);
                    }else{ /* for signPost */
                        this.signPostObj = {};
                        this.signPostObj = this.schemaObj.result[0].attributes;
                        /* remove stuff that need not be displayed */
                        this.signPostObj.splice(0, 2);
                        this.signPostObj.pop();
                        this.signPostObj.pop();
                        this.vmdirUtils.setAttributeDataType(this.signPostObj)
                        this.constructSchemaMap(schemaName, this.signPostObj);

                        /* Handle multi value attributes */
                        if(this.isEdited && 'FALSE' == this.curSchemaMap['isSingleValued'][0]){
                           this.isListEdited = true;
                        }
                        /* Handle Boolean Attributes */
                        if(this.isEdited && 'ADSTYPE_BOOLEAN' == this.curSchemaMap['dataType']) {
                            this.setBooleanElement(schemaName);
                        }
                        /* Handle Editing security descriptor */
                        if(this.isEdited && 'ADSTYPE_NT_SECURITY_DESCRIPTOR' == this.curSchemaMap['dataType']){
                            this.isSdEdited = true;
                            this.showSignPost = false;
                        }
                        /* Handle Date Tiime attributes */
                        if(this.isEdited && 'ADSTYPE_UTC_TIME' == this.curSchemaMap['dataType']){
                            this.isDateEdited = true;
                            //this.setDateTimeElement(schemaName);
                        }
                        console.log(this.signPostObj);
                    }
                    },
            error => this.error = <any>error);
    }

    getDirListing() {
        console.log("getDirListing - app component");
        this.vmdirService.getDirListing(null)
        .subscribe(
            listing => {
                    this.listing = JSON.stringify(listing);
                    this.listingObj = JSON.parse(this.listing);
                    this.utilsService.extractName(this.listingObj.result, true);
                    console.log(this.listingObj);
                    },
            error => this.error = <any>error);
    }
}

@Component({
    moduleId: module.id,
    selector: "lazy-loaded-level1",
    template: `
        <ng-template #loadingChild>
            <clr-tree-node>
                <div align="center">
                <span class="spinner spinner-inline">
                   Loading...
                </span>
                <span>
                    Loading subtree...
                </span>
                </div>
            </clr-tree-node>
        </ng-template>
            <ng-container [clrLoading]="loading">
                <clr-tree-node *ngFor="let commonName of listArr">
                    <button
                        (click)="selectedDN=commonName.encodedDN ;getAttributes()"
                         class="clr-treenode-link">
                               {{commonName.displayName}}
                    </button>
                    <ng-template clrIfExpanded>
                        <lazy-loaded-level1 [(rootDn)]=commonName.dn (onChildSelected)="onRecChildSelected($event)"></lazy-loaded-level1>
                    </ng-template>
                </clr-tree-node>
            </ng-container>
       <!--/span-->
    `
})
export class LazyLoadedLevel1Component{
    @Input() rootDn: string;
    @Output() onChildSelected = new EventEmitter<string>();
    listingObj: any;
//    listingObjObs: Observable<any>;
    listing: string;
    attribs: string;
    selectedDN: string;
    attribsArr: any[];
    listArr:any[];
    len:number;
    loading: boolean;
    private error:any;
    constructor(private utilsService: UtilsService, private vmdirService: VmdirService) {
       console.log("App component - vmdir");
    }
    ngOnInit() {
        this.loading = true;
        // This would be a call to your service that communicates with the server
        console.log('------->oninit-lazyloadedcomponent' + this.rootDn);
        this.getDirListing();
//        console.log(this.listingObjObs);
    }

    onRecChildSelected(childDN:string) {
          console.log("recursively calling emit")
          this.onChildSelected.emit(childDN);
    }
    getAttributes() {
        console.log("getAttributes - component");
        let attribsObj:any;
        this.onChildSelected.emit(this.selectedDN);
    }
    getDirListing() {
        console.log("-------->getting child compoment");
        this.loading  = true;
        this.vmdirService.getDirListing(encodeURIComponent(this.rootDn))
        .subscribe(
            listing => {
                    this.listing = JSON.stringify(listing);
                    this.listingObj = JSON.parse(this.listing);
                    this.utilsService.extractName(this.listingObj.result, false);
                    this.listArr = this.listingObj.result;
                    this.loading = false;
                    },
            error => this.error = <any>error);
    }
}
