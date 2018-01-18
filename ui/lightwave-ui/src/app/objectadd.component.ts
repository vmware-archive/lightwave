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

import { EventEmitter, Output, ViewChild, Component, Input } from '@angular/core';
import { VmdirService } from './vmdir.service';
import { VmdirSchemaService } from './vmdirschema.service';
import { VmdirUtils } from './vmdir.utils';
import { Wizard, WizardPage } from 'clarity-angular';
import { Observable } from "rxjs/Rx";
import {StringFilter, DatagridFilter } from "clarity-angular";
@Component({
  moduleId: module.id,
  selector: 'objectadd',
  templateUrl: './objectadd.component.html',
})

export class ObjectAddComponent  {
    @Input() rootDn:string;
    @Output() notify: EventEmitter<string> = new EventEmitter<string>();
    @ViewChild("wizard") wizard: Wizard;
    @ViewChild("pageThree") pageMayAttr: WizardPage;
    @ViewChild("pageFive") pageMustAttr: WizardPage;
    @ViewChild("pageFive") pageConfirm: WizardPage;
    display:string;
    isOpen:boolean;
    sel:string[]=[];
    classesArr:any[];
    auxClassMap:any;
    classNameFilter = new ClassNameFilter();
    auxNameFilter = new ClassNameFilter();
    mayAttrMap:any = {};
    mustAttrMap:any = {};
    signPostObj:any[] = [];
    curSchemaMap:any = {};
    mayAttrArr:string[] = [];
    allDone:boolean = false;
    mustAttrArr:string[] = [];
    showCancelConfirm:boolean = false;
    decodedRootDn:string;
    stage:string;
    classMap:any;
    classNames:string[];
    auxClassNames:string[];
    classNamesObs:Observable<string[]>;
    constructor(private vmdirUtils: VmdirUtils, private vmdirSchemaService: VmdirSchemaService, private vmdirService: VmdirService){
       this.isOpen = true;
       this.classesArr = [];
       this.classNames = [];
       this.auxClassNames = [];
       this.classMap = {};
       this.auxClassMap = {};
       this.stage = '';
    }
     ngOnInit(){
        this.decodedRootDn = decodeURIComponent(this.rootDn);
        console.log(this.rootDn);
        this.classNamesObs = this.getAllStructuralObjectClasses()
     }
     handleClassSelection(selClass){
         if(selClass){
            this.mayAttrArr = [];
            this.mustAttrArr = [];
            this.mayAttrMap = {};
            this.mustAttrMap = {};
            this.prepareAuxClasses(selClass);
            this.getAllAttributes(selClass);
            this.wizard.next();
         }
    }
    handleClose(){
        this.showCancelConfirm = false;
        this.wizard.close();
        this.notify.emit('done');
    }
    public doCancel() {
        if (confirm("Are you sure you want to close the add object wizard?")) {
            this.handleClose();
        }
    }

    accumilateAuxClasses(classObj:any){
       if(classObj && classObj.auxClass){
            for(let cls of classObj.auxClass){
                this.auxClassMap[cls] = true;
            }
        }
    }

    prepareAuxClasses(className:string){
        let classObj = this.classMap[className];
        this.accumilateAuxClasses(classObj);
        this.getAllAttributes('top');
        while(classObj && classObj.subClassOf != 'top'){
            classObj = this.classMap[classObj.subClassOf];
            this.getAllAttributes(classObj.subClassOf);
            this.accumilateAuxClasses(classObj);
        }
        this.mustAttrMap['objectClass'] = className;
        this.mustAttrArr.push('objectClass');
        this.auxClassNames = Object.keys(this.auxClassMap);
        this.stage = 'showAuxClass'
    }

    handleMustAttr(){
        let unfilledAttr:boolean = false;
        for(var i = 0;i<this.mustAttrArr.length; i ++){
            if(!this.mustAttrMap[this.mustAttrArr[i]].length){
                unfilledAttr = true;
                break;
            }
        }
        if(!unfilledAttr){
            this.wizard.next();
        }
    }

    createObject(){
        let res:any ;
        let cn:string = this.mustAttrMap['cn'].length?this.mustAttrMap['cn']:this.mayAttrMap['cn'];
        if(cn.length){
            let newRootDn:string = 'cn='+cn + ',' + this.decodedRootDn ;
            this.vmdirService.addNewObject(newRootDn, this.mustAttrMap, this.mayAttrMap)
            .subscribe(
            result => {
               res = result;
               this.handleClose();
            },
            error => {
               console.log(error);
            });
        }
    }
    handleAuxClassSel(auxClasses:any[]){
       for(let aux of auxClasses){
           this.getAllAttributes(aux);
           this.mustAttrMap['objectClass'] += ',' + (aux);
       }
       this.wizard.next();
   }

   populateAttribMap(schemaObj:any){
       let attrArr:any[] = schemaObj.result[0].attributes;
       for(let attr of attrArr){
           if(attr.type == 'systemMayContain' || attr.type == 'mayContain'){
               for(let may of attr.value){
                   if(!(may in this.mayAttrMap)){
                       this.mayAttrArr.push(may)
                       this.mayAttrMap[may] = '';
                   }
               }
           }
           if(attr.type == 'systemMustContain' || attr.type == 'mustContain'){
               for(let must of attr.value){
                   if(!(must in this.mustAttrMap)){
                       this.mustAttrMap[must] = '';
                       if(must != 'nTSecurityDescriptor'){
                           this.mustAttrArr.push(must);
                       }
                   }
               }
           }
       }
   }

   displayProperties(propName:string){
      this.signPostObj = [];
      let schemaObj:any
      this.vmdirSchemaService.getSchema(propName)
        .subscribe(
            schema => {
                    schemaObj = schema;
                    this.signPostObj = schemaObj.result[0].attributes;
                        /* remove stuff that need not be displayed */
                        this.signPostObj.splice(0, 2);
                        this.signPostObj.pop();
                        this.signPostObj.pop();
                        this.vmdirUtils.setAttributeDataType(this.signPostObj);
                        for(let schema of this.signPostObj){
                            this.curSchemaMap[schema.type] = schema.value;
                        }
                        console.log(this.curSchemaMap);
            },
            error => {
            });
   }

   getAllAttributes(className:string) {
        let schemaObj:any
        this.vmdirSchemaService.getSchema(className)
        .subscribe(
            schema => {
                    schemaObj = schema;
                    this.populateAttribMap(schemaObj);
                    console.log(className);
                    console.log(schemaObj);
            },
            error => {
            });
   }
   constructClassMap():void{
        let cn:string;
        for(let cls of this.classesArr){
            let obj:any = {};
            for(let attr of cls.attributes){
                if(attr.type == 'cn'){
                    cn = attr.value[0];
                }else if(attr.type == 'objectClassCategory'){
                    obj.objectClassCategory = attr.value[0];
                }else if(attr.type == 'auxiliaryClass'){
                    obj.auxClass = attr.value;
                }else if(attr.type == 'subClassOf'){
                    obj.subClassOf = attr.value[0];
                }else if(attr.type == 'systemMayContain'){
                    obj.sysMayContain = attr.value;
                }else if(attr.type == 'systemMustContain'){
                    obj.sysMustContain = attr.value;
                }else if(attr.type == 'mayContain'){
                    obj.mayContain = attr.value;
                }else if(attr.type == 'mustContain'){
                    obj.mustContain = attr.value;
                }
            }
            this.classMap[cn] = obj;
            if(obj.objectClassCategory == '1'){
                this.classNames.push(cn);
            }
        }
        console.log(this.classMap);
        console.log(this.classNames);
    }

    getAllStructuralObjectClasses():Observable<string[]>{
         let res:any;
         return this.vmdirService.getAllStrObjectClasses()
            .map(
                result => {
                    console.log(result);
                    res = result;
                    this.classesArr = result.result;
                    this.constructClassMap();
                    return this.classNames;
                    }).first();
    }
}


export class ClassNameFilter implements StringFilter<any> {
    accepts(className: string, search: string):boolean {
        return className.toLowerCase().indexOf(search) >= 0;
    }
}
