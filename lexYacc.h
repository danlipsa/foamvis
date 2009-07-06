#ifndef _LEXYACC_H_
#define _LEXYACC_H_

typedef struct { long i;
                 long double r;
              } yystype;
#define YYSTYPE yystype

int keywordId (char* keyword);
void flexDebugging (int debugging);

#endif
