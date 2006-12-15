/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of Intel Corp. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/** 
 * @author Vadim Revyakin
 */




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "u/libu.h"

#include "wsman-client-api.h"
#include "wsman-client-transport.h"
#include "wsman-xml-serializer.h"
#include "wsman-debug.h"

#define CLASSNAME "Sample"



static void
example1()
{
struct __Sample_Servie
{
  int AcceptPause;
  int AcceptStop;
  char* Caption;
  unsigned long CheckPoint;
  char* CreationClassName;
  char* Description;
  int DesktopInteract;
  char*  DisplayName;
  char* ErrorControl;
  unsigned long ExitCode;
  char* InstallDate;
  char* Name;
  char* PathName;
  unsigned long ProcessId;
  unsigned long ServiceSpecificExitCode;
  char* ServiceType;
  int Started;
  char* StartMode;
  char* StartName;
  char* State;
  char* Status;
  char* SystemCreationClassName;
  char* SystemName;
  unsigned long TagId;
  unsigned long WaitHint;
};
typedef struct __Sample_Servie Sample_Servie;

Sample_Servie servie = {
        0,
        1,
        "Caption",
        30,
        "CreationClassName",
        "Description",
        1,
        "DisplayName",
        "ErrorControl",
        50,
        "InstallDate",
        "Name",
        "PathName",
        60,
        70,
        "ServiceType",
        0,
        "StartMode",
        "StartName",
        "State",
        "Status",
        "SystemCreationClassName",
        "SystemName",
        90,
        100
};

SER_START_ITEMS("Sample_Servie", Sample_Servie)
SER_BOOL("AcceptPause",0,1),
SER_BOOL("AcceptStop",0,1),
SER_STR("Caption",0,1),
SER_UINT32("CheckPoint", 1 ,1 ),
SER_STR("CreationClassName",0,1),
SER_STR("Description",0,1),
SER_BOOL("DesktopInteract",0,1),
SER_STR("DisplayName",0,1),
SER_STR("ErrorControl",0,1),
SER_UINT32("ExitCode",0,1),
SER_STR("InstallDate",0,1),
SER_STR("Name",0,1),
SER_STR("PathName",0,1),
SER_UINT32("ProcessId",0,1),
SER_UINT32("ServiceSpecificExitCode",0,1),
SER_STR("ServiceType",0,1),
SER_BOOL("Started",0,1),
SER_STR("StartMode",0,1),
SER_STR("StartName",0,1),
SER_STR("State",0,1),
SER_STR("Status",0,1),
SER_STR("SystemCreationClassName",0,1),
SER_STR("SystemName",0,1),
SER_UINT32("TagId",0,1),
SER_UINT32("WaitHint",0,1),
SER_END_ITEMS("Sample_Servie", Sample_Servie);

WsContextH cntx;
WsXmlDocH doc;
WsXmlNodeH node;
int retval;

    printf("\n\n   ********   example1()  ********\n");

    cntx = wsman_create_runtime();
    if (cntx == NULL) {
        printf("Error ws_create_runtime\n");
        return;
    }
    doc = wsman_create_doc(cntx, "example");
    node = ws_xml_get_doc_root(doc);

    retval = ws_serialize(cntx, node, &servie, Sample_Servie_TypeInfo,
               CLASSNAME, NULL, NULL, 0);
    printf("ws_serialize: %d\n", retval);
    ws_xml_dump_node_tree(stdout, node);
    node = ws_xml_get_doc_root(doc);
    Sample_Servie *cs = (Sample_Servie *)ws_deserialize(cntx,
                                     node,
                                     Sample_Servie_TypeInfo,
                                     CLASSNAME, NULL, NULL,
                                     0, 0);
    if (cs == NULL) {
        printf("Errror ws_serialize\n");
        return;
    }
    retval = memcmp(cs, &servie, sizeof (&servie));
    if (retval) {
            printf("Not compared (%d)   -    FAILED\n", retval);
            printf("%d   :  %d\n", servie.AcceptPause, cs->AcceptPause);
            printf("%d   :  %d\n", servie.AcceptStop, cs->AcceptStop);
            printf("%s   :  %s\n", servie.Caption, cs->Caption);
   } else {
        printf("           PASS\n");
   }
}




static void
example2()
{

char *strings[] = {
    "string 1", "string 2", "string 3", NULL,
};

struct __XmlSerializerInfo strings_TypeInfo[] =
{
      SER_STR("string", 1, 1)
};

typedef struct {
  int AcceptPause;
  char* Caption;
  char **strings;
  unsigned long CheckPoint;
  int AcceptStop;
} Sample;


Sample sample = {
        0,
        "Caption",
        strings,
        30,
        1,

};


SER_START_ITEMS("Sample", Sample)
SER_BOOL("AcceptPause",0,1),
SER_STR("Caption",0,1),
SER_STR_PTR("strings", 1, 1),
SER_UINT32("CheckPoint", 1 ,1 ),
SER_BOOL("AcceptStop",0,1),
SER_END_ITEMS("Sample", Sample);

WsContextH cntx;
WsXmlDocH doc;
WsXmlNodeH node;
int retval;

    printf("\n\n   ********   example2()  ********\n");

    cntx = wsman_create_runtime();
    if (cntx == NULL) {
        printf("Error ws_create_runtime\n");
        return;
    }
    doc = wsman_create_doc(cntx, "example");
    node = ws_xml_get_doc_root(doc);

    retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
               CLASSNAME, NULL, NULL, 0);
    printf("ws_serialize: %d\n", retval);
    ws_xml_dump_node_tree(stdout, node);

    node = ws_xml_get_doc_root(doc);
    Sample *cs = (Sample *)ws_deserialize(cntx,
                                     node,
                                     Sample_TypeInfo,
                                     CLASSNAME, NULL, NULL,
                                     0, 0);
    if (cs == NULL) {
        printf("Errror ws_deserialize\n");
        return;
    }
    if (cs->strings == NULL) {
        printf("No strings\n");
        return;
    }
    printf("\n       deserialized strings:\n");
    char **p = cs->strings;
    while (*p != NULL) { printf("%s\n", *p); p++; }
}



static void
example3()
{
typedef struct {
  unsigned char a;
  unsigned char b;
  unsigned char c;
  unsigned char pad;
  char* string;
} Sample;



SER_START_ITEMS("Sample", Sample)
SER_UINT8("a",0,1),
SER_UINT8("b",0,1),
SER_UINT8("c", 0 ,1 ),
SER_INOUT_UINT8("pad"),
SER_STR("string",0,1),
SER_END_ITEMS("Sample", Sample);

Sample sample = {'a', 'b', 'c', 'x', "simple string"};
Sample *p = NULL;


WsContextH cntx;
WsXmlDocH doc;
WsXmlNodeH node;
int retval;

    printf("\n\n   ********   example3()  ********\n");

    cntx = wsman_create_runtime();
    if (cntx == NULL) {
        printf("Error ws_create_runtime\n");
        return;
    }
    doc = wsman_create_doc(cntx, "example");
    node = ws_xml_get_doc_root(doc);

    retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
               CLASSNAME, NULL, NULL, 0);
    printf("ws_serialize: %d\n", retval);
    ws_xml_dump_node_tree(stdout, node);

    printf("\n\nws_deserialize (prints original : result):\n");

    node = ws_xml_get_doc_root(doc);
    Sample *cs = (Sample *)ws_deserialize(cntx,
                                     node,
                                     Sample_TypeInfo,
                                     CLASSNAME, NULL, NULL,
                                     0, 0);
    if (cs == NULL) {
        printf("Errror ws_serialize\n");
        return;
    }

    printf("a   = %c   :  %c\n", sample.a, cs->a);
    printf("b   = %c   :  %c\n", sample.b, cs->b);
    printf("c   = %c   :  %c\n", sample.c, cs->c);
    printf("pad = %c   :  %c\n", sample.pad, cs->pad);
    printf("string = <%s>   :  <%s>\n", sample.string, cs->string);
}


static void
example4()
{

typedef struct {
  int a;
  char* string;
  int b;
} Embed;

typedef struct {
    int A;
    Embed EMBED[2];
    char *STRING;
} Sample;


Sample sample = {
        10,
        {{1, "string 1", 2}, {3, "string 2", 4},},
        "STRING",
};

SER_START_ITEMS("Embed", Embed)
SER_BOOL("a",0,1),
SER_STR("string",0,1),
SER_BOOL("b",0,1),
SER_END_ITEMS("Embed", Embed);

SER_START_ITEMS("Sample", Sample)
SER_BOOL("A",0,1),
SER_STRUCT("EMBED", 0, 2, Embed),
SER_STR("STRING",0,1),
SER_END_ITEMS("Sample", Sample);

WsContextH cntx;
WsXmlDocH doc;
WsXmlNodeH node;
int retval;

    printf("\n\n   ********   example4()  ********\n");
    cntx = wsman_create_runtime();
    if (cntx == NULL) {
        printf("Error ws_create_runtime\n");
        return;
    }
    doc = wsman_create_doc(cntx, "example");
    node = ws_xml_get_doc_root(doc);

    retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
               CLASSNAME, NULL, NULL, 0);
    printf("ws_serialize: %d\n", retval);
    ws_xml_dump_node_tree(stdout, node);
}





static void
example5()
{

char *strings1[] = {
    "string 11", "string 12", "string 13", NULL,
};
char *strings2[] = {
    "string 21", "string 22", "string 23", NULL,
};


typedef struct {
  int AcceptPause;
  char* Caption;
  char **strings;
} Foo;

Foo foos[] = {
        {1, "Caption 1", strings1},
        {0, "Caption 2", strings2},
        {1, "Caption 1",},
        {0, "Caption 2",},};


SER_START_ITEMS("Foo", Foo)
SER_BOOL("AcceptPause",0,1),
SER_STR("Caption",0,1),
SER_STR_PTR("strings", 1, 1),
SER_END_ITEMS("Foo", Foo);

unsigned short shorts[] = {5, 11, 14,19, 27, 36};
SER_TYPEINFO_UINT16;

typedef struct {
        char *city;
        XmlSerialiseDynamicSizeData shorts;
        XmlSerialiseDynamicSizeData foos;
        short tag;
} Sample;

Sample sample = { "Moscow", {6, shorts}, {2, foos}, 99};




SER_START_ITEMS("Sample", Sample)
SER_STR("city", 0, 1),
SER_DYN_ARRAY("shorts", uint16),
SER_DYN_ARRAY("foos", Foo),
SER_UINT16("tag", 0, 1),
SER_END_ITEMS("Sample", Sample);

WsContextH cntx;
WsXmlDocH doc;
WsXmlNodeH node;
int retval;

    printf("\n\n   ********   example5()  ********\n");

    cntx = wsman_create_runtime();
    if (cntx == NULL) {
        printf("Error ws_create_runtime\n");
        return;
    }
    doc = wsman_create_doc(cntx, "example");
    node = ws_xml_get_doc_root(doc);

    retval = ws_serialize(cntx, node, &sample, Sample_TypeInfo,
               CLASSNAME, NULL, NULL, 0);
    printf("\n\nws_serialize: %d\n", retval);
    ws_xml_dump_node_tree(stdout, node);

    node = ws_xml_get_doc_root(doc);

    printf("\n\nws_deserialize:\n");
    Sample *cs = (Sample *)ws_deserialize(cntx,
                                     node,
                                     Sample_TypeInfo,
                                     CLASSNAME, NULL, NULL,
                                     0, 0);
    if (cs == NULL) {
        printf("Errror ws_deserialize\n");
        return;
    }
    int i;
    printf("shorts count = %d\n", cs->shorts.count);
    printf("foos count   = %d\n", cs->foos.count);
    printf("\n");
    printf("    city = <%s>\n", cs->city);
    if (cs->shorts.data == NULL) {
        printf("No uint16 objects\n");
        goto AFTER_SHORTS;
    }
    unsigned short *newuints = (unsigned short *)cs->shorts.data;
    printf("    shorts = {");
    for (i = 0; i < cs->shorts.count; i++) {
        printf("%u, ", *newuints);
        newuints++;
    }
    printf("}\n");
AFTER_SHORTS:
    if (cs->foos.data == NULL) {
        printf("No foo objects\n");
        goto AFTER_FOOS;
    }
    Foo *newfoos = cs->foos.data;
    for (i = 0; i < cs->foos.count; i++) {
        printf("   ====   Foo %d =====\n", i);
        printf("    AcceptPause  =   %d\n",  newfoos->AcceptPause);
        printf("    Caption       =   <%s>\n", newfoos->Caption);

        char **p = newfoos->strings;
        printf("    strings :\n");
        while (*p) {
            printf("         <%s>\n", *p);
            p++;
        }
        printf("   ====   End of Foo %d =====\n", i);
        newfoos++;
    }
AFTER_FOOS:
    printf("    tag = %d\n", cs->tag);
}

int
main(int argc, char **argv)
{
    int num;
    int i;

    if (argc == 1) {
        // execute all
        example1();
        example2();
        example3();
        example4();
        example5();
        return 0;
    }

    for (i = 1; i < argc; i++) {
        num = atoi(argv[i]);
        switch (num) {
        case 1: example1(); break;
        case 2: example2(); break;
        case 3: example3(); break;
        case 4: example4(); break;
        case 5: example5(); break;
        default:
            printf("\n    No example%d()\n", num);
            break;
        }
    }
    return 0;
}
