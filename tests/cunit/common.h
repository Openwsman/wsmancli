
#ifndef _COMMON_H
#define _COMMON_H

#include <CUnit/Basic.h> 
#include "wsman-client-api.h"

typedef struct {
  const char *server;
  int port;
  const char *path;
  const char *scheme;
  const char *username;
  const char *password;
} ServerData;

typedef struct {						
  /* Explanation of what you should see */
  const char *explanation;

  /* Resource UR to test against */
  const char *resource_uri;

  const char *method;

  /* Selectors in the form of a URI query   key=value&key2=value2 */
  char *selectors;
  /* Properties in the form of a URI query   key=value&key2=value2 */
  char *properties;

  char* expr1;
  char* value1;

  char *expr2;
  char *value2;


  /* What the final status code should be. */
  unsigned int final_status;		

  unsigned char       flags;

  unsigned int		max_elements;

  /* pairs of string filter/value. The last pair is NULL/NULL */
  char **filters;
  char **common_filters;

} TestData;

extern char *host;
extern int verbose;

int init_test(void);
int clean_test(void);
int add_enumeration_tests(CU_pSuite ps);
int add_identify_tests(CU_pSuite ps);
int add_transfer_get_tests(CU_pSuite ps);
int add_transfer_put_tests(CU_pSuite ps);
int add_invoke_tests(CU_pSuite ps);
int add_pull_tests(CU_pSuite ps);

void check_response_header(WsXmlDocH doc, long resp_code, char *action);
void handle_filters(WsXmlDocH doc, char *f[]);

#endif
