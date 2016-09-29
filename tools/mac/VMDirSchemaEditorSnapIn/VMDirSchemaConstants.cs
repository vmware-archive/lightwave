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

using System;

/*
 * VMDirSchema Constants using in the UI
 *
 * @author Sumalatha Abhishek
 */
namespace VMDirSchema
{
    public static class VMDirSchemaConstants
    {
        /*dialog result codes */
        public const int DIALOGOK = 1;
        public const int DIALOGCANCEL = -1;

        /* Welcome Screen Constants */
        public const string WELCOME_TITLE_DESCRIPTION = "Welcome to Lightwave Directory Schema Editor.";
        public const string WELCOME_DESCRIPTION1 = "Browse Directory Schema";
        public const string WELCOME_DESCRIPTION2 = "Manage/Edit Schema";
        public const string WELCOME_DESCRIPTION3 = "Compare Federation and Replication Metadata";

        /* Application Constants */
        public const string VMDIRSCHEMA_APPNAME = "Lightwave Schema Management";
        public const string VMDIRSCHEMA_DATA_FILENAME = "VMDirSchemaData.xml";
        public const string VMDIRSCHEMA_SNAPIN = "VMware Directory Schema";
        public const string VMDIRSCHEMA_DESCRIPTION = "VMware Domain Name Service SnapIn";
        public const string VMDIRSCHEMA_SCHEMA_VIEW = "Schema View";
        public const string VMDIRSCHEMA_FEDERATION_VIEW = "Federation View";
        public const string VMDIRSCHEMA_SEARCH = "Search";
        public const string FEDERATION_TOOLBAR = "FederationToolBar";

        /* UI Constants*/
        public const string VMDIRSCHEMA_ADDCLASS = "New ObjectClass";
        public const string VMDIRSCHEMA_CLASSES = "Classes";
        public const string VMDIRSCHEMA_ADDATTRIBUTE = "New Attribute";
        public const string VMDIRSCHEMA_PROPERTIES = "Properties";
        public const string VMDIRSCHEMA_ATTRIBUTES = "Attributes";
        public const string VMDIRSCHEMA_NAME = "Name";
        public  const string VMDIRSCHEMA_ATTR_SYNTAX = "Syntax";
        public const string VMDIRSCHEMA_DESC = "Description";
        public const string VMDIRSCHEMA_ATTR_STATUS = "Status";
        public const string VMDIRSCHMEA_ATTR_SOURCECLASS = "Source Class";
        public const string VMDIRSCHEMA_CLASS_TYPE = "Type";
        public const string VMDIRSCHEMA_SOURCECLASS = "Source Class";
        public const string VMDIRSCHEMA_OPTIONAL_ATTR = "Is Optional";
        public const string VMDIRSCHEMA_ITEMS = "Items";
        public const string VMDIRSCHEMA_STRUCTURAL = "Structural";
        public const string VMDIRSCHEMA_AUXILIARY = "Auxiliary";
        public const string VMDIRSCHEMA_ABSTRACT = "Abstract";
        public const string BASE_TITLE = "Base :";
        public const string CURRENT_TITLE = "Current :";
        public const string MISING_METADATA = "MetaData Missing.";
        public const string MISING_OBJECTCLASS = "ObjectClass Missing.";
        public const string MISSING_ATTRIBUTETYPE = "AttributeType Missing.";
        public const string DIFF_OBJECTCLASS = "View ObjectClass Diff";
        public const string DIFF_METADATA = "View MetaData Diff";
        public const string DIFF_ATTRIBUTETYPE = "View AttributeType Diff";
        public const string NO_DATA_FOUND = "No Data Found";
        public const string NO_CHANGE_DETECTED = "No change in values is detected";
        public const string NODE1 = "Node1";
        public const string NODE2 = "Node2";
        public const string NODES = "Nodes";
        public const string SYNC_STATUS = "Sync Status";

        public const int VMDIRSERVER_TIMEOUT = 20000;

        public const string ATTR_MODIFY_MESSAGE = "Successfully modified AttributeType";
        public const string ATTR_ADD_MESSAGE = "Successfully added AttributeType";
        public const string CLASS_MODIFY_MESSAGE = "Successfully modified Object Class";
        public const string CLASS_ADD_MESSAGE = "Successfully added Object Class";
        public const string NO_DIFF_FOUND = "No Diff Found";



    }
}