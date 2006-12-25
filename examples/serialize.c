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

/*
 ************************************************************************

          HOW TO USE SERIALIZATION / DESERIALIZATION

    Now serialization/deserialization of unsignedByte, unsignedShort,
 unsignedInt, boolean and string built-n XML Schema types implemented.
 It is possible to serialize/deserialize structures, static and dynamic
 size arrays constructed from these types.

   Let's we have the following schema:
   <xsd:element name="SAMPLE">
      <xsd:complexType>
        <xsd:sequence>
          <xsd:element name="STRING" type="xsd:string"/>
          <xsd:element name="BOOL" type="xsd:boolean"/>
          <xsd:element name="BYTE" type="xsd:unsignedByte"/>
          <xsd:element name="SHORT" type="xsd:unsignedShort"/>
          <xsd:element name="INT" type="xsd:unsignedInt"/>
          <xsd:element name="SHORTS" type="xsd:unsignedShort" minOccurs="3" maxOccurs="3"/>
          <xsd:element name="INTS" type="xsd:unsignedInt" minOccurs="0" maxOccurs="5"/>
          <xsd:element ref="FOO"  maxOccurs="unbounded"/>
        </xsd:sequence>
      </xsd:complexType>
   </xsd:element>
   <xsd:element name="FOO">
      <xsd:complexType>
        <xsd:sequence>
          <xsd:element name="FooSTRING" type="xsd:string"/>
          <xsd:element name="FooINT" type="xsd:unsignedInt"/>
          <xsd:element name="FooBOOL" type="xsd:boolean"/>
        </xsd:sequence>
      </xsd:complexType>
   </xsd:element>


   Each complex element is represented in C programm by 2 objects - target
   structure (TS) definition and type description object (TDO) - the null
   terminated array of type XmlSerializerInfo. There is the name convention -
   TDO for Foo is named Foo_TypeInfo. For our example these structures looks:

    target structures:

    typedef struct {
        XML_TYPE_STR     FooString;
        XML_TYPE_UINT32  FooInt;
        XML_TYPE_BOOL    FooBoolean;
    } Foo;

    typedef struct {
        XML_TYPE_STR     String;
        XML_TYPE_BOOL    Boolean;
        XML_TYPE_UINT8   Byte;
        XML_TYPE_UINT16  Short;
        XML_TYPE_UINT32  Int;
        XML_TYPE_UINT16  Shorts[3];
        XML_TYPE_DYN_ARRAY  Ints;
        XML_TYPE_DYN_ARRAY  Foos;
   } Sample;

   Note, that field Shorts in Sample is defined as built-in array because
   the number of elements in schema is strictly determed. Elements Ints
   and Foos are defined as dynamic arrays because the number of these elements
  in document is variable.

   For each TS the TDO is defined. Each TDS is defined by the sequence of
   defines described in wsman-xml-serializer.h.

  SER_START_ITEMS("FOO", Foo)
            // This is the begining of description. The first argument is the
            // name of element in XML schema, the second one is the name
            // of TS type.
     SER_STR("FooSTRING", 1),
     SER_UINT32("FooINT", 1),
     SER_BOOL("FooBOOL", 1),
            // These 3 defines are for string, unsignedInt and boolean XML types
            // accordingly. The first argument is the name of element in XML
            // schema, the second one is the number of elements.
  SER_END_ITEMS("FOO", Foo);
            // This define completes the definition. The arguments are same as
            // for SER_START_ITEMS.

   So we define The TDO for Foo type. It looks like:
       XmlSerializerInfo Foo_TypeInfo[] = {
         ................
       };
   There some defines to define XmlSerializerInfo's for basic types XML_TYPE_UINT8,
   XML_TYPE_UINT16, XML_TYPE_UINT32, XML_TYPE_BOOL and XML_TYPE_STR:
       SER_TYPEINFO_UINT8;
       SER_TYPEINFO_UINT16;
       SER_TYPEINFO_UINT32;
       SER_TYPEINFO_BOOL;
       SER_TYPEINFO_STR;
   If you use dymanic arrays of basic types you must deefine the coorespondent
   XmlSerializerInfo before defining TDO including this dynamic array. You will
   refer to these TDOs in SER_DYN_ARRAY define and use the forth argument for
   these types uint8, uint16, uint32, bool and string as the last argument (see
   below). 

  Let's do the same for SAMPLE XML element and Sample type.

    SER_START_ITEMS("SAMPLE", sample)
       SER_STR("STRING", 1),
       SER_BOOL("BOOL", 1),
       SER_UINT8("BYTE", 1),
       SER_UINT16("SHORT", 1),
       SER_UINT32("INT", 1),
       SER_UINT16("SHORTS", 3),
       SER_DYN_ARRAY("INTS", 0, 5, uint32),
            // This dynamic array describes XML element INTS of type unsignedInt
            // with minOccurs=0 and maxOccurs=5.
       SER_DYN_ARRAY("FOOS", 1, 0, Foo),
            // Dynamic array of Foo type elements. maxOccures=0 means
            // "unbounded" in XML schema
    SER_END_ITEMS("SAMPLE", sample);


   These objects can be used in ws_serialize() and ws_deserialize() API.

   There are 2 sets of defines SER_IN_* and SER_OUT_*. These defines are used
   if you want to skip the elements while deserialization(serialization). If
   define SER_INOUT_* is used the element is skipped always.

  *****************************************************************/




static void
example1()
{
struct __Sample_Servie
{
  XML_TYPE_BOOL AcceptPause;
  XML_TYPE_BOOL AcceptStop;
  XML_TYPE_STR Caption;
  XML_TYPE_UINT32 CheckPoint;
  XML_TYPE_STR CreationClassName;
  XML_TYPE_STR Description;
  XML_TYPE_BOOL DesktopInteract;
  XML_TYPE_STR  DisplayName;
  XML_TYPE_STR ErrorControl;
  XML_TYPE_UINT32 ExitCode;
  XML_TYPE_STR InstallDate;
  XML_TYPE_STR Name;
  XML_TYPE_STR PathName;
  XML_TYPE_UINT32 ProcessId;
  XML_TYPE_UINT32 ServiceSpecificExitCode;
  XML_TYPE_STR ServiceType;
  XML_TYPE_BOOL Started;
  XML_TYPE_STR StartMode;
  XML_TYPE_STR StartName;
  XML_TYPE_STR State;
  XML_TYPE_STR Status;
  XML_TYPE_STR SystemCreationClassName;
  XML_TYPE_STR SystemName;
  XML_TYPE_UINT32 TagId;
  XML_TYPE_UINT32 WaitHint;
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
SER_BOOL("AcceptPause", 1),
SER_BOOL("AcceptStop", 1),
SER_STR("Caption", 1),
SER_UINT32("CheckPoint", 1),
SER_STR("CreationClassName", 1),
SER_STR("Description", 1),
SER_BOOL("DesktopInteract", 1),
SER_STR("DisplayName", 1),
SER_STR("ErrorControl", 1),
SER_UINT32("ExitCode", 1),
SER_STR("InstallDate", 1),
SER_STR("Name", 1),
SER_STR("PathName", 1),
SER_UINT32("ProcessId", 1),
SER_UINT32("ServiceSpecificExitCode", 1),
SER_STR("ServiceType", 1),
SER_BOOL("Started", 1),
SER_STR("StartMode", 1),
SER_STR("StartName", 1),
SER_STR("State", 1),
SER_STR("Status", 1),
SER_STR("SystemCreationClassName", 1),
SER_STR("SystemName", 1),
SER_UINT32("TagId", 1),
SER_UINT32("WaitHint", 1),
SER_END_ITEMS("Sample_Servie", Sample_Servie);

WsContextH cntx;
WsXmlDocH doc;
WsXmlNodeH node;
int retval;

    printf("\n\n   ********   example1. Basic types  ********\n");

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
   }
}




static void
example2()
{

typedef struct {
    XML_TYPE_UINT8 byte1;
    XML_TYPE_UINT32 int1;
    XML_TYPE_UINT8 byte2;
} Foo;

typedef struct {
  XML_TYPE_UINT8 byte1;
  XML_TYPE_UINT16 short1;
  XML_TYPE_UINT32 int1;
  char *string1;
  Foo  foo;
} Sample;


Sample sample = { 1, 2, 4, "string", {5, 196, 8} };

SER_START_ITEMS("Foo", Foo)
  SER_UINT8("FOOBYTE1", 1),
  SER_UINT32("FOOINT32", 1),
  SER_UINT8("FOOBYTE2", 1),
SER_END_ITEMS("Foo", Foo);


SER_START_ITEMS("Sample", Sample)
  SER_UINT8("BYTE", 1),
  SER_UINT16("SHORT", 1),
  SER_UINT32("INT32", 1),
  SER_STR("STRING", 1),
  SER_STRUCT("FOO", 1, Foo),
SER_END_ITEMS("Sample", Sample);

WsContextH cntx;
WsXmlDocH doc;
WsXmlNodeH node;
int retval;

    printf("\n\n   ********   example2. Structure with pads.  ********\n");

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

    printf("\n      initial and deserialized structures\n");
    printf("     byte1    =    %d : %d\n", sample.byte1, cs->byte1);
    printf("     short1   =    %d : %d\n", sample.short1, cs->short1);
    printf("     int1     =    %d : %d\n", sample.int1, cs->int1);
    printf("     string1  =    <%s> : <%s>\n", sample.string1, cs->string1);
    printf("     foo :\n");
    printf("        byte1    =    %d : %d\n", sample.foo.byte1, cs->foo.byte1);
    printf("        int1     =    %d : %d\n", sample.foo.int1, cs->foo.int1);
    printf("        byte2    =    %d : %d\n", sample.foo.byte2, cs->foo.byte2);
}



static void
example3()
{
typedef struct {
  XML_TYPE_UINT8 a;
  XML_TYPE_UINT8 b;
  XML_TYPE_UINT8 c;
  XML_TYPE_UINT8 pad;
  XML_TYPE_STR string;
} Sample;



SER_START_ITEMS("Sample", Sample)
SER_UINT8("a", 1),
SER_UINT8("b", 1),
SER_UINT8("c", 1),
SER_IN_UINT8("pad", 1),
SER_STR("string", 1),
SER_END_ITEMS("Sample", Sample);

Sample sample = {'a', 'b', 'c', 'x', "simple string"};
Sample *p = NULL;


WsContextH cntx;
WsXmlDocH doc;
WsXmlNodeH node;
int retval;

    printf("\n\n   ********   example3. Skip elements.  ********\n");

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
    printf("pad = %c(%d)   :  %c(%d)\n", sample.pad, sample.pad,
                                cs->pad, cs->pad);
    printf("string = <%s>   :  <%s>\n", sample.string, cs->string);
}


static void
example4()
{

typedef struct {
  XML_TYPE_BOOL a;
  XML_TYPE_STR string;
  XML_TYPE_BOOL b;
} Embed;

typedef struct {
    XML_TYPE_UINT32 A;
    Embed EMBED[2];
    XML_TYPE_STR STRING;
} Sample;


Sample sample = {
        10,
        {{1, "string 1", 0}, {0, "string 2", 1},},
        "STRING",
};

SER_START_ITEMS("Embed", Embed)
SER_BOOL("a", 1),
SER_STR("string", 1),
SER_BOOL("b", 1),
SER_END_ITEMS("Embed", Embed);

SER_START_ITEMS("Sample", Sample)
SER_UINT32("A", 1),
SER_STRUCT("EMBED", 2, Embed),
SER_STR("STRING", 1),
SER_END_ITEMS("Sample", Sample);

WsContextH cntx;
WsXmlDocH doc;
WsXmlNodeH node;
int retval;

    printf("\n\n   ********   example4. Static structure array  ********\n");
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


typedef struct {
    XML_TYPE_BOOL AcceptPause;
    XML_TYPE_STR Caption;
} Foo;

Foo foos[] = {
        {1, "Caption 1"},
        {0, "Caption 2"},
        {1, "Caption 1",},
        {0, "Caption 2",},};


SER_START_ITEMS("Foo", Foo)
SER_BOOL("AcceptPause", 1),
SER_STR("Caption", 1),
SER_END_ITEMS("Foo", Foo);

XML_TYPE_UINT16 myshorts[] = {5, 11, 14,19, 27, 36};
SER_TYPEINFO_UINT16;

typedef struct {
        XML_TYPE_STR city;
        XML_TYPE_DYN_ARRAY shorts;
        XML_TYPE_DYN_ARRAY foos;
        XML_TYPE_UINT16 tag;
} Sample;

Sample sample = { "Moscow", {6, myshorts}, {2, foos}, 99};




SER_START_ITEMS("Sample", Sample)
SER_STR("city", 1),
SER_DYN_ARRAY("shorts", 0, 1000, uint16),
SER_DYN_ARRAY("foos", 0, 1000, Foo),
SER_UINT16("tag", 1),
SER_END_ITEMS("Sample", Sample);

WsContextH cntx;
WsXmlDocH doc;
WsXmlNodeH node;
int retval;

    printf("\n\n   ********   example5. Dynamic arrays  ********\n");

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
        printf("   ====   End of Foo %d =====\n", i);
        newfoos++;
    }
AFTER_FOOS:
    printf("    tag = %d\n", cs->tag);
}

/*
static void
debug_message_handler(const char *str, debug_level_e level, void *user_data)
{
    if (wsman_debug_level_debugged(level)) {
        struct tm      *tm;
        time_t          now;
        char            timestr[128];

        time(&now);
        tm = localtime(&now);
        strftime(timestr, 128, "%b %e %T", tm);
        fprintf(stderr, "%s  %s\n", timestr, str);
    }
}
*/
static void
initialize_logging(void)
{
    debug_add_handler(wsman_debug_message_handler, DEBUG_LEVEL_ALWAYS, NULL);
}

int debug_level = 0;
int
main(int argc, char **argv)
{
    int num;
    int i;
    actionOptions options;
    char retval = 0;
    u_error_t *error = NULL;
    u_option_entry_t opt[] = {
        { "debug",  'd',    U_OPTION_ARG_INT,   &debug_level,
               "Set the verbosity of debugging output.",   "1-6" }
    };
    u_option_context_t *opt_ctx; 
    opt_ctx = u_option_context_new("");
    u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    u_option_context_add_main_entries(opt_ctx, opt, "wsmid_identify");
    retval = u_option_context_parse(opt_ctx, &argc, &argv, &error);

    u_option_context_free(opt_ctx);

    if (error) {
      if (error->message)
        printf ("%s\n", error->message);
      u_error_free(error);
      return 1;
    }
    u_error_free(error);

    if (debug_level) {
        initialize_logging();
        wsman_debug_set_level(debug_level);
    }

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
