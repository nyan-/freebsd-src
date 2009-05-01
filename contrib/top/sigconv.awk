# $FreeBSD: src/contrib/top/sigconv.awk,v 1.2.8.1 2009/04/15 03:14:26 kensmith Exp $

BEGIN		{
		    nsig = 0;
		    j = 0;
		    print "/* This file was automatically generated */"
		    print "/* by the awk script \"sigconv.awk\".      */\n"
		    print "struct sigdesc {"
		    print "    char *name;"
		    print "    int  number;"
		    print "};\n"
		    print "struct sigdesc sigdesc[] = {"
		}

/^#define[ \t][ \t]*SIG[A-Z]+[0-9]*[ \t]/	{

				    j = sprintf("%d", $3);
				    str = $2;

				    if (nsig < j) 
					nsig = j;

				    siglist[j] = sprintf("{ \"%s\",\t%2d },", \
						substr(str, 4), j);
				}
/^#[ \t]*define[ \t][ \t]*SIG[A-Z]+[0-9]*[ \t]/	{

				    j = sprintf("%d", $4);
				    str = $3;

				    if (nsig < j)
					nsig = j;

				    siglist[j] = sprintf("{ \"%s\",\t%2d },", \
						substr(str, 4), j);
				}
/^#[ \t]*define[ \t][ \t]*_SIG[A-Z]+[0-9]*[ \t]/	{

				    j = sprintf("%d", $4);
				    str = $3;

				    if (nsig < j)
					nsig = j;

				    siglist[j] = sprintf("{ \"%s\",\t%2d },", \
					    substr(str, 5), j);
				}

END				{
				    for (n = 1; n <= nsig; n++) 
					if (siglist[n] != "")
					    printf("    %s\n", siglist[n]);

				    printf("    { NULL,\t 0 }\n};\n");
				}
