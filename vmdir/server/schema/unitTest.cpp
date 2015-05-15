#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "includes.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

// Workaround to link common/libcommon.a (logging.c)
int  ldap_syslog_level = 0;
int  ldap_syslog = 0;
int  slap_debug = 0;

using namespace CppUnit;
using namespace std;


class SchemaTests : public TestFixture
{
private:
    // member variables for fixture tests
    string schemaFilePath;

public:
    SchemaTests()
    {
    schemaFilePath = "./vmdirschema.ldif";
    }

    void setUp()
    {
    // This runs before every test
    }

    void tearDown()
    {
    // This runs after every test
    }

    void TestSchemaLibrary()
    {
        PVDIR_SCHEMA_CTX        pCtx = NULL;
        cout << "TestSchemaLibrary -----------------------------------" << endl;
        {
            cout << endl << "case 001 - VdirSchemaInitializeViaFile" << endl;
            CPPUNIT_ASSERT(0 == VmDirSchemaLibInit());
            CPPUNIT_ASSERT(0 == VmDirSchemaInitializeViaFile(schemaFilePath.c_str()));

            CPPUNIT_ASSERT(NULL != (pCtx = VmDirSchemaCtxAcquire()));
            CPPUNIT_ASSERT(3 == VmDirSchemaAttrNameToId(pCtx, "ldapSyntaxes"));
            //CPPUNIT_ASSERT(100 == VmDirSchemaAttrNameToId(pCtx, "knowledgeInformation"));
            //CPPUNIT_ASSERT(NULL != VmDirSchemaAttrNameToDesc(pCtx, "knowledgeInformation"));
            CPPUNIT_ASSERT(NULL != VmDirSchemaAttrIdToDesc(pCtx, 3));

            if (pCtx) VmDirSchemaCtxRelease(pCtx);
            VmDirSchemaLibShutdown();
        }

    }

    void TestSchemaParseAT()
    {
        cout << "TestSchemaParseAT -----------------------------------" << endl;

        {
            cout << endl << "case 001 - no attribute name" << endl;

            VDIR_SCHEMA_AT_DESC    ATDesc = {0};
            char pAt[] =
            "AttributeTypes: ( 1.3 NAME () DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )";

            CPPUNIT_ASSERT(0 != VmDirSchemaParseStrToATDesc(pAt, &ATDesc));
        }
        {
            cout << endl << "case 002 - no attribute name" << endl;

            VDIR_SCHEMA_AT_DESC    ATDesc = {0};
            char pAt[] =
            "AttributeTypes: ( 1.3 NAME ( ) DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )";

            CPPUNIT_ASSERT(0 != VmDirSchemaParseStrToATDesc(pAt, &ATDesc));
        }
    }

    void TestSchemaParseOC()
    {
        cout << "TestSchemaParseOC -----------------------------------" << endl;

        {
            cout << endl << "case 1" << endl;

            VDIR_SCHEMA_OC_DESC OCDesc;
            char pOc[2048] =
                    "ObjectClasses: ( 1.2 NAME ( ) DESC 'ocdesc1' SUP top STRUCTURAL MAY ( atname1 ) )";

            memset(&OCDesc, 0, sizeof(OCDesc));
            // no oc name, pass parser but fail in verification later
            CPPUNIT_ASSERT(0 == VmDirSchemaParseStrToOCDesc(pOc, &OCDesc));
            VmDirSchemaOCDescContentFree(&OCDesc);
        }

    }

    void TestSchemaValidation()
    {
        PVDIR_SCHEMA_INSTANCE    pSchema = NULL;

        cout << "TestSchemaValidation --------------------------------" << endl;

        CPPUNIT_ASSERT(0 == VmDirSchemaLibInit());

        {
            cout << endl << "case 001 - no attribute defined" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.2 NAME 'ocname1' DESC 'ocdesc1' SUP top STRUCTURAL MAY ( atname1 ) )"
            };

            CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, 1, &pSchema));
        }

        {
            cout << endl << "case 002 - no objectclasses defined" << endl;
            char* ppdata[] = {
                    "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
                    "AttributeTypes: ( 1.5 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )"
            };
/// no longer valid, we always need top
//            CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]), &pSchema));
        }

        {
            cout << endl << "case 003 - attribute name atname1 not unique" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.2 NAME 'ocname1' DESC 'ocdesc1' SUP top STRUCTURAL MAY ( atname1 ) )",
            "AttributeTypes: ( 1.3 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.5 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )"
            };

            CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]), &pSchema));
        }

        {
            cout << endl << "case 004  - attribute name atname1 not unique" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.2 NAME 'ocname1' DESC 'ocdesc1' SUP top STRUCTURAL MAY ( atname1 ) )",
            "AttributeTypes: ( 1.3 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.5 NAME ('twonames atname1') DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )"
            };

            CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
        }

        {
            cout << endl << "case 005  - attribute name atname1 not unique" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.2 NAME 'ocname1' DESC 'ocdesc1' SUP top STRUCTURAL MAY ( atname1 $ onename) )",
            "AttributeTypes: ( 1.3 NAME ('onename atname1') DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.5 NAME ('twonames atname1') DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )"
            };

            CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
        }

        {
            cout << endl << "case 006 - ocname2 SUP not defined" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.2 NAME 'ocname1' DESC 'ocdesc1' SUP top STRUCTURAL MAY ( atname1 ) )",
            "ObjectClasses: ( 1.3 NAME 'ocname2' DESC 'ocdesc2' SUP nonexists STRUCTURAL MAY atname1 )",
            "AttributeTypes: ( 1.5 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )"
            };

            CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
            if (pSchema)
            {
                VdirSchemaInstanceFree(pSchema);
                pSchema = NULL;
            }
        }

        {
            cout << endl << "case 007 - objectclass ocname2 SUP ocname1" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.2 NAME 'ocname1' DESC 'ocdesc1' SUP top STRUCTURAL MUST ( atname2 ) )",
            "ObjectClasses: ( 1.3 NAME 'ocname2' DESC 'ocdesc2' SUP ocname1 STRUCTURAL MUST atname1 )",
            "AttributeTypes: ( 1.5 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.3 NAME 'atname2' DESC 'atdesc2' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )"
            };

            CPPUNIT_ASSERT(0 == UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
            if (pSchema)
            {
                VdirSchemaInstanceFree(pSchema);
                pSchema = NULL;
            }
        }

        {
            cout << endl << "case 008 - vdirSchemaLoadOCDescAllATs must case" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.1 NAME 'ocname1' DESC 'ocdesc2' SUP top STRUCTURAL MUST ( atname5 $ atname6 $ atname7 $ atname8 $ atname9 )",
            "ObjectClasses: ( 1.2 NAME 'ocname2' DESC 'ocdesc2' SUP ocname1 STRUCTURAL MUST ( atname2 $ atname3 $ atname4 )",
            "ObjectClasses: ( 1.3 NAME 'ocname3' DESC 'ocdesc3' SUP ocname2 STRUCTURAL MUST atname1 )",
            "AttributeTypes: ( 1.4 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.5 NAME 'atname2' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.6 NAME 'atname3' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.7 NAME 'atname4' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.8 NAME 'atname5' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.9 NAME 'atname6' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.10 NAME 'atname7' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.11 NAME 'atname8' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.12 NAME 'atname9' DESC 'atdesc2' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )"
            };

            CPPUNIT_ASSERT(0 == UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
            if (pSchema)
            {
                VdirSchemaInstanceFree(pSchema);
                pSchema = NULL;
            }
        }

        {
            cout << endl << "case 009 - vdirSchemaLoadATDescSup sup found" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.3 NAME 'ocname3' DESC 'ocdesc3' STRUCTURAL MUST atname1 )",
            "AttributeTypes: ( 1.4 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.5 NAME 'atname2' DESC 'atdesc1' sup atname1 )"
            };

            CPPUNIT_ASSERT(0 == UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
            if (pSchema)
            {
                VdirSchemaInstanceFree(pSchema);
                pSchema = NULL;
            }
        }

        {
            cout << endl << "case 010 - vdirSchemaLoadATDescSup sup not found" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.3 NAME 'ocname3' DESC 'ocdesc3' STRUCTURAL MUST atname1 )",
            "AttributeTypes: ( 1.4 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.5 NAME 'atname2' DESC 'atdesc1' sup atname0 )"
            };

            CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
        }

        {
            cout << endl << "case 011 - vdirSchemaVerifyATDesc no attribute oid" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.3 NAME 'ocname3' DESC 'ocdesc3' STRUCTURAL MUST atname1 )",
            "AttributeTypes: ( NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )"
            };

            CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
        }

        {
            cout << endl << "case 012 - vdirSchemaVerifyATDesc no syntax" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.3 NAME 'ocname3' DESC 'ocdesc3' STRUCTURAL MUST atname1 )",
            "AttributeTypes: ( 1.4 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch  )"
            };

            CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
        }

        {
            cout << endl << "case 013 - vdirSchemaLoadATDescResolve resolve syntax" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.3 NAME 'ocname3' DESC 'ocdesc3' STRUCTURAL MUST atname1 )",
            "AttributeTypes: ( 1.4 NAME 'atname0' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )",
            "AttributeTypes: ( 1.5 NAME 'atname1' DESC 'atdesc1' SUP atname0  )"
            };

            CPPUNIT_ASSERT(0 == UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
            if (pSchema)
            {
                VdirSchemaInstanceFree(pSchema);
                pSchema = NULL;
            }
        }

        {
            cout << endl << "case 014 - vdirSchemaLoadOCDescSupOCs sup must be same type" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.3 NAME 'ocname1' DESC 'ocdesc3' STRUCTURAL MUST atname1 )",
            "ObjectClasses: ( 1.4 NAME 'ocname2' DESC 'ocdesc3' SUP ocname1 AUXILIARY MUST atname1 )",
            "AttributeTypes: ( 1.5 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{32768} )"
            };

            //CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
            // relax this check for now to load full AD schema
            CPPUNIT_ASSERT(0 == UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
            if (pSchema)
            {
                VdirSchemaInstanceFree(pSchema);
                pSchema = NULL;
            }
        }

        {
            cout << endl << "case 015 - vdirSchemaLoadATDescResolveSyntax syntax not supported" << endl;
            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.3 NAME 'ocname3' DESC 'ocdesc3' STRUCTURAL MUST atname1 )",
            "AttributeTypes: ( 1.4 NAME 'atname1' DESC 'atdesc1' EQUALITY caseIgnoreMatch SYNTAX 10.3.6.1.4.1.1466.115.121.1.6 )",
            "AttributeTypes: ( 1.5 NAME 'atname2' DESC 'atdesc1' sup atname1 )"
            };

            //CPPUNIT_ASSERT(0 != UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
            // relax this check for now to load full AD schema
            CPPUNIT_ASSERT(0 == UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));
            if (pSchema)
            {
                VdirSchemaInstanceFree(pSchema);
                pSchema = NULL;
            }
        }

        VmDirSchemaLibShutdown();

    }

    void TestSyntax()
    {
        PVDIR_SCHEMA_INSTANCE    pSchema = NULL;
        PVDIR_SCHEMA_CTX        pCtx = NULL;
        PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;
        VDIR_BERVALUE                BerVal = {0};

        cout << "TestSyntax" << endl;
        {

            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.3 NAME 'ocname3' DESC 'ocdesc3' STRUCTURAL MUST numericstr )",
            "AttributeTypes: ( 1.4 NAME 'numericstr' SYNTAX 1.3.6.1.4.1.1466.115.121.1.36 )",
            "AttributeTypes: ( 1.5 NAME 'dirstr'     SYNTAX 1.3.6.1.4.1.1466.115.121.1.15 )",
            "AttributeTypes: ( 1.6 NAME 'GeneralizedTime'     SYNTAX 1.3.6.1.4.1.1466.115.121.1.24 )",
            "AttributeTypes: ( 1.7 NAME 'INTEGER'     SYNTAX 1.3.6.1.4.1.1466.115.121.1.27 )"
            };

            CPPUNIT_ASSERT(0 == VmDirSchemaLibInit());

            CPPUNIT_ASSERT(0 == UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));

            gVdirSchemaGlobals.pSchemas[1] = pSchema;
            gVdirSchemaGlobals.usLiveSchema = 1;

            CPPUNIT_ASSERT(NULL != (pCtx = VmDirSchemaCtxAcquire()));

            cout << endl << "case 001 -  INTEGER" << endl;
            // INTEGER syntax
            CPPUNIT_ASSERT(NULL != (pATDesc = VmDirSchemaAttrNameToDesc(pCtx, "INTEGER")));

            BerVal.bv_len = 3;
            BerVal.bv_val = "123";
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            BerVal.bv_len = 3;
            BerVal.bv_val = "1A2";
            CPPUNIT_ASSERT(0 != VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            BerVal.bv_len = 1;
            BerVal.bv_val = "-";
            CPPUNIT_ASSERT(0 != VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            BerVal.bv_len = 2;
            BerVal.bv_val = "+0";
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            BerVal.bv_len = 4;
            BerVal.bv_val = "-100";
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            cout << endl << "case 002 -  numeric string" << endl;
            // numeric string syntax
            CPPUNIT_ASSERT(NULL != (pATDesc = VmDirSchemaAttrNameToDesc(pCtx, "numericstr")));

            BerVal.bv_len = 5;
            BerVal.bv_val = "12345";
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            BerVal.bv_len = 5;
            BerVal.bv_val = "123A5";
            CPPUNIT_ASSERT(0 != VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            cout << endl << "case 003 -  directory string" << endl;
            // directory string syntax
            CPPUNIT_ASSERT(NULL != (pATDesc = VmDirSchemaAttrNameToDesc(pCtx, "dirstr")));
            unsigned char ucValidUTF8_1[] = { 0xc0, 0x80};
            BerVal.bv_len = 2;
            BerVal.bv_val = (char*)ucValidUTF8_1;
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            unsigned char ucInvalidUTF8_1[] = { 0xc0, 0xc0};
            BerVal.bv_len = 2;
            BerVal.bv_val = (char*)ucInvalidUTF8_1;
            CPPUNIT_ASSERT(0 != VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            unsigned char ucValidUTF8_2[] = { 0xe0, 0x80, 0x80};
            BerVal.bv_len = 3;
            BerVal.bv_val = (char*)ucValidUTF8_2;
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            unsigned char ucInvalidUTF8_2[] = { 0xe0, 0x80, 0xc0};
            BerVal.bv_len = 3;
            BerVal.bv_val = (char*)ucInvalidUTF8_2;
            CPPUNIT_ASSERT(0 != VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            unsigned char ucValidUTF8_3[] = { 0xfc, 0x80, 0x80, 0x80, 0x80, 0x80};
            BerVal.bv_len = 6;
            BerVal.bv_val = (char*)ucValidUTF8_3;
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            unsigned char ucInvalidUTF8_3[] = { 0xfe, 0x80, 0x80, 0x80, 0x80, 0x80};
            BerVal.bv_len = 6;
            BerVal.bv_val = (char*)ucInvalidUTF8_3;
            CPPUNIT_ASSERT(0 != VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            cout << endl << "case 004 -  generalized time" << endl;
            // generalized time syntax
            CPPUNIT_ASSERT(NULL != (pATDesc = VmDirSchemaAttrNameToDesc(pCtx, "GeneralizedTime")));
            BerVal.bv_len = 15;
            BerVal.bv_val = "20070508200557Z";
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));
            BerVal.bv_len = 13;
            BerVal.bv_val = "200705082005Z";
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));
            BerVal.bv_len = 11;
            BerVal.bv_val = "2007050820Z";
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalSyntaxCheck(pCtx, pATDesc, &BerVal));

            if (pCtx) VmDirSchemaCtxRelease(pCtx);

            VmDirSchemaLibShutdown();
        }
    }

    void TestMatchingRule()
    {
        PVDIR_SCHEMA_INSTANCE    pSchema = NULL;
        PVDIR_SCHEMA_CTX        pCtx = NULL;
        PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;
        VDIR_BERVALUE                BerVal = {0};
        VDIR_BERVALUE                AssertBerv = {0};

        cout << "TestMatchingRule" << endl;
        {

            char* ppdata[] = {
            "ObjectClasses: ( 1.1 NAME 'top' DESC 'top' ABSTRACT )",
            "ObjectClasses: ( 1.3 NAME 'ocname3' DESC 'ocdesc3' STRUCTURAL MUST numericstr )",
            "AttributeTypes: ( 1.9 NAME 'dnstr' EQUALITY distinguishedNameMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.12 )",
            "AttributeTypes: ( 1.4 NAME 'numericstr' SYNTAX 1.3.6.1.4.1.1466.115.121.1.36 )",
            "AttributeTypes: ( 1.5 NAME 'dirstrCaseExact' EQUALITY caseExactMatch ORDERING caseExactOrderingMatch SUBSTR caseExactSubstringsMatch  SYNTAX 1.3.6.1.4.1.1466.115.121.1.15 )",
            "AttributeTypes: ( 1.6 NAME 'dirstrCaseIgnore' EQUALITY caseIgnoreMatch  SYNTAX 1.3.6.1.4.1.1466.115.121.1.15 )",
            "AttributeTypes: ( 1.7 NAME 'GeneralizedTime'     SYNTAX 1.3.6.1.4.1.1466.115.121.1.24 )",
            "AttributeTypes: ( 1.8 NAME 'INTEGER' EQUALITY integerMatch ORDERING integerOrderingMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.27 )"
            };

            CPPUNIT_ASSERT(0 == VmDirSchemaLibInit());

            CPPUNIT_ASSERT(0 == UnitTestSchemaInstanceInit(ppdata, sizeof(ppdata)/sizeof(ppdata[0]),&pSchema));

            gVdirSchemaGlobals.pSchemas[1] = pSchema;
            gVdirSchemaGlobals.usLiveSchema = 1;

            CPPUNIT_ASSERT(NULL != (pCtx = VmDirSchemaCtxAcquire()));

            cout << endl << "case 001 -  directory string - caseExactMath" << endl;
            // directory string - caseExactMath
            CPPUNIT_ASSERT(NULL != (pATDesc = VmDirSchemaAttrNameToDesc(pCtx, "dirstrCaseExact")));

            BerVal.bv_val = "a";
            BerVal.bv_len = strlen(BerVal.bv_val);
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            CPPUNIT_ASSERT(BerVal.bv_val == BerVal.bvnorm_val);

            BerVal.bv_val = "A";
            BerVal.bv_len = strlen(BerVal.bv_val);
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            CPPUNIT_ASSERT(BerVal.bv_val == BerVal.bvnorm_val);

            BerVal.bv_val = " a ";
            BerVal.bv_len = strlen(BerVal.bv_val);
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            CPPUNIT_ASSERT(BerVal.bv_val == BerVal.bvnorm_val);

            BerVal.bv_val = "  a ";
            BerVal.bv_len = strlen(BerVal.bv_val);
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            CPPUNIT_ASSERT(strncmp(BerVal.bvnorm_val, " a ", strlen(" a ")) == 0);
            CPPUNIT_ASSERT(BerVal.bv_val != BerVal.bvnorm_val);
            VMDIR_SAFE_FREE_MEMORY(BerVal.bvnorm_val);
            BerVal.bvnorm_len=0;

            BerVal.bv_val = "  a  ";
            BerVal.bv_len = strlen(BerVal.bv_val);
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            CPPUNIT_ASSERT(strncmp(BerVal.bvnorm_val, " a ", strlen(" a ")) == 0);
            CPPUNIT_ASSERT(BerVal.bv_val != BerVal.bvnorm_val);
            VMDIR_SAFE_FREE_MEMORY(BerVal.bvnorm_val);
            BerVal.bvnorm_len=0;


            BerVal.bv_val = "  a  b  ";
            BerVal.bv_len = strlen(BerVal.bv_val);
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            CPPUNIT_ASSERT(strncmp(BerVal.bvnorm_val, " a b ", strlen(" a b ")) == 0);
            CPPUNIT_ASSERT(BerVal.bv_val != BerVal.bvnorm_val);
            AssertBerv.bv_val = " a b ";
            AssertBerv.bv_len = strlen(AssertBerv.bv_val);
            AssertBerv.bvnorm_val = AssertBerv.bv_val;
            AssertBerv.bvnorm_len = AssertBerv.bv_len;
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_EQUAL, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_GE, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_LE, &AssertBerv, &BerVal));
            AssertBerv.bv_val = " a b";
            AssertBerv.bv_len = strlen(AssertBerv.bv_val);
            AssertBerv.bvnorm_val = AssertBerv.bv_val;
            AssertBerv.bvnorm_len = AssertBerv.bv_len;
            CPPUNIT_ASSERT(false == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_EQUAL, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(false == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_GE, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_LE, &AssertBerv, &BerVal));
            AssertBerv.bv_val = " a";
            AssertBerv.bv_len = strlen(AssertBerv.bv_val);
            AssertBerv.bvnorm_val = AssertBerv.bv_val;
            AssertBerv.bvnorm_len = AssertBerv.bv_len;
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_SUBSTR_INIT, &AssertBerv, &BerVal));
            VMDIR_SAFE_FREE_MEMORY(BerVal.bvnorm_val);
            BerVal.bvnorm_len=0;

            cout << endl << "case 002 -  directory string - caseIgnoreMath" << endl;
            // directory string - caseIgnoreMath
            CPPUNIT_ASSERT(NULL != (pATDesc = VmDirSchemaAttrNameToDesc(pCtx, "dirstrCaseIgnore")));

            BerVal.bv_val = "a";
            BerVal.bv_len = strlen(BerVal.bv_val);
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            CPPUNIT_ASSERT(BerVal.bv_val == BerVal.bvnorm_val);

            BerVal.bv_val = " a b ";
            BerVal.bv_len = strlen(BerVal.bv_val);
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            CPPUNIT_ASSERT(BerVal.bv_val == BerVal.bvnorm_val);

            BerVal.bv_val = "A";
            BerVal.bv_len = strlen(BerVal.bv_val);
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            CPPUNIT_ASSERT(strncmp(BerVal.bvnorm_val, "a", strlen("a")) == 0);
            CPPUNIT_ASSERT(BerVal.bv_val != BerVal.bvnorm_val);
            VMDIR_SAFE_FREE_MEMORY(BerVal.bvnorm_val);
            BerVal.bvnorm_len=0;

            BerVal.bv_val = " A  B   ";
            BerVal.bv_len = strlen(BerVal.bv_val);
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            CPPUNIT_ASSERT(strncmp(BerVal.bvnorm_val, " a b ", strlen(" a b ")) == 0);
            CPPUNIT_ASSERT(strcmp(BerVal.bvnorm_val, " a b ") == 0);
            CPPUNIT_ASSERT(BerVal.bv_val != BerVal.bvnorm_val);
            VMDIR_SAFE_FREE_MEMORY(BerVal.bvnorm_val);
            BerVal.bvnorm_len=0;

            cout << endl << "case 003 -  integer string - integerMatch" << endl;
            CPPUNIT_ASSERT(NULL != (pATDesc = VmDirSchemaAttrNameToDesc(pCtx, "INTEGER")));

            AssertBerv.bv_val = "-0";
            AssertBerv.bv_len = strlen(AssertBerv.bv_val);
            AssertBerv.bvnorm_val = AssertBerv.bv_val;
            AssertBerv.bvnorm_len = AssertBerv.bv_len;
            BerVal.bv_val = "0";
            BerVal.bv_len = strlen(BerVal.bv_val);
            BerVal.bvnorm_val = BerVal.bv_val;
            BerVal.bvnorm_len = BerVal.bv_len;
            // -0 vs 0
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_EQUAL, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_GE, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_LE, &AssertBerv, &BerVal));

            AssertBerv.bv_val = "-1";
            AssertBerv.bv_len = strlen(AssertBerv.bv_val);
            AssertBerv.bvnorm_val = AssertBerv.bv_val;
            AssertBerv.bvnorm_len = AssertBerv.bv_len;
            BerVal.bv_val = "1";
            BerVal.bv_len = strlen(BerVal.bv_val);
            BerVal.bvnorm_val = BerVal.bv_val;
            BerVal.bvnorm_len = BerVal.bv_len;
            // -1 vs 1
            CPPUNIT_ASSERT(false == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_EQUAL, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(false == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_GE, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_LE, &AssertBerv, &BerVal));

            AssertBerv.bv_val = "1";
            AssertBerv.bv_len = strlen(AssertBerv.bv_val);
            AssertBerv.bvnorm_val = AssertBerv.bv_val;
            AssertBerv.bvnorm_len = AssertBerv.bv_len;
            BerVal.bv_val = "1";
            BerVal.bv_len = strlen(BerVal.bv_val);
            BerVal.bvnorm_val = BerVal.bv_val;
            BerVal.bvnorm_len = BerVal.bv_len;
            // 1 vs 1
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_EQUAL, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_GE, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_LE, &AssertBerv, &BerVal));

            AssertBerv.bv_val = "11";
            AssertBerv.bv_len = strlen(AssertBerv.bv_val);
            AssertBerv.bvnorm_val = AssertBerv.bv_val;
            AssertBerv.bvnorm_len = AssertBerv.bv_len;
            BerVal.bv_val = "2";
            BerVal.bv_len = strlen(BerVal.bv_val);
            BerVal.bvnorm_val = BerVal.bv_val;
            BerVal.bvnorm_len = BerVal.bv_len;
            // 11 vs 2
            CPPUNIT_ASSERT(false == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_EQUAL, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_GE, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(false == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_LE, &AssertBerv, &BerVal));

            AssertBerv.bv_val = "-11";
            AssertBerv.bv_len = strlen(AssertBerv.bv_val);
            AssertBerv.bvnorm_val = AssertBerv.bv_val;
            AssertBerv.bvnorm_len = AssertBerv.bv_len;
            BerVal.bv_val = "2";
            BerVal.bv_len = strlen(BerVal.bv_val);
            BerVal.bvnorm_val = BerVal.bv_val;
            BerVal.bvnorm_len = BerVal.bv_len;
            // -11 vs 2
            CPPUNIT_ASSERT(false == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_EQUAL, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(false == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_GE, &AssertBerv, &BerVal));
            CPPUNIT_ASSERT(true == VmDirSchemaBervalCompare(pCtx, pATDesc, VDIR_SCHEMA_MATCH_LE, &AssertBerv, &BerVal));


            cout << endl << "case 004 -  DN Normalization" << endl;
            // DN normalize
            CPPUNIT_ASSERT(NULL != (pATDesc = VmDirSchemaAttrNameToDesc(pCtx, "dnstr")));

            BerVal.bv_val = "INTEGER=99";
            BerVal.bv_len = strlen(BerVal.bv_val);
            BerVal.bvnorm_val=NULL;
            BerVal.bvnorm_len=0;
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            // 1. all attribute name are lower cased
            CPPUNIT_ASSERT(0 == strcmp(BerVal.bvnorm_val, "integer=99"));
            CPPUNIT_ASSERT(BerVal.bv_val != BerVal.bvnorm_val);
            VMDIR_SAFE_FREE_MEMORY(BerVal.bvnorm_val);

            BerVal.bv_val = " dirstrCaseIgnore   =   myCaseIgnoreDirStr   ";
            BerVal.bv_len = strlen(BerVal.bv_val);
            BerVal.bvnorm_val=NULL;
            BerVal.bvnorm_len=0;
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            // 1. all attribute name are lower cased
            // 2. all leading/trailing spaces are removed
            // 3. value are normalized based on its syntax
            CPPUNIT_ASSERT(0 == strcmp(BerVal.bvnorm_val, "dirstrcaseignore=mycaseignoredirstr"));
            CPPUNIT_ASSERT(BerVal.bv_val != BerVal.bvnorm_val);
            VMDIR_SAFE_FREE_MEMORY(BerVal.bvnorm_val);

            BerVal.bv_val = " dirstrCaseExact=   myCaseExactDirStr   ";
            BerVal.bv_len = strlen(BerVal.bv_val);
            BerVal.bvnorm_val=NULL;
            BerVal.bvnorm_len=0;
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            // 1. all attribute name are lower cased
            // 2. all leading/trailing spaces are removed
            // 3. value are normalized based on its syntax
            CPPUNIT_ASSERT(0 == strcmp(BerVal.bvnorm_val, "dirstrcaseexact=myCaseExactDirStr"));
            CPPUNIT_ASSERT(BerVal.bv_val != BerVal.bvnorm_val);
            VMDIR_SAFE_FREE_MEMORY(BerVal.bvnorm_val);

            BerVal.bv_val = "INTEGER  =  99 ,   dirstrCaseIgnore =myCaseIgnoreDirStr   , dirstrCaseExact=   myCaseExactDirStr   ";
            BerVal.bv_len = strlen(BerVal.bv_val);
            BerVal.bvnorm_val=NULL;
            BerVal.bvnorm_len=0;
            CPPUNIT_ASSERT(0 == VmDirSchemaBervalNormalize(pCtx, pATDesc, &BerVal));
            // 1. all attribute name are lower cased
            // 2. all leading/trailing spaces are removed
            // 3. value are normalized based on its syntax
            CPPUNIT_ASSERT(0 == strcmp(BerVal.bvnorm_val, "integer=99,dirstrcaseignore=mycaseignoredirstr,dirstrcaseexact=myCaseExactDirStr"));
            CPPUNIT_ASSERT(BerVal.bv_val != BerVal.bvnorm_val);
            VMDIR_SAFE_FREE_MEMORY(BerVal.bvnorm_val);

            if (pCtx) VmDirSchemaCtxRelease(pCtx);

            VmDirSchemaLibShutdown();
        }
    }
    void TestSingle()
    {
        PVDIR_SCHEMA_INSTANCE    pSchema = NULL;
        PVDIR_SCHEMA_CTX        pCtx = NULL;
        PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;
        VDIR_BERVALUE                BerVal = {0};

        cout << "TestSingle" << endl;
        {

        }
    }

    CPPUNIT_TEST_SUITE(SchemaTests);

    //CPPUNIT_TEST(TestSingle);
    CPPUNIT_TEST(TestSchemaLibrary);

    CPPUNIT_TEST(TestSchemaParseAT);
    CPPUNIT_TEST(TestSchemaParseOC);
    CPPUNIT_TEST(TestSchemaValidation);
    CPPUNIT_TEST(TestSyntax);
    CPPUNIT_TEST(TestMatchingRule);

    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SchemaTests);


int main( int argc, char **argv)
{
    TextUi::TestRunner runner;
    TestFactoryRegistry& registry = TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());

    bool wasSuccessful = runner.run("", false);

    return wasSuccessful;
}
