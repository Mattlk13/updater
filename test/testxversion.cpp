#include <stdlib.h>
#include <QString>

#include <stdio.h>

#include "xversion.h"

char *formattests[] = {
  "",
  "1",
  "1.",
  "1.0",
  "1.2",
  "1.2wip",
  "1.2alpha",
  "1.2beta",
  "1.2gamma",
  "1.2rc",
  "1.23",
  "1.23.0",
  "0.0.0rc2",
  "1.2.3",
  "1.2.3wip",
  "1.2.3wip1",
  "1.2.3alpha",
  "1.2.3alpha1",
  "1.2.3alpha2",
  "1.2.3beta",
  "1.2.3beta0",
  "1.2.3beta1",
  "1.2.3beta2",
  "1.2.3rc-1",
  "1.2.3rc",
  "1.2.3rc0",
  "1.2.3rc4",
  "1.2.3rc4invalid",
  "1.2.3rcinvalid",
  "invalid",
  "1.2Wip",
  "1.2aLpha",
  "1.2beTa",
  "1.2gamMa",
  "1.2Rc",
  "1.2.3betA2",
  "1.2.3Rc-1",
  "1.2.3rciNvalid",
  "invaLid",
  "prefix1.2.3rc1suffix",

  "v1",
  "v1.",
  "v1.0",
  "v1.2",
  "v1.2-wip",
  "v1.2-alpha",
  "v1.2-beta",
  "v1.2-gamma",
  "v1.2-rc",
  "v1.23",
  "v1.23.0",
  "v0.0.0-rc2",
  "v1.2.3",
  "v1.2.3-wip",
  "v1.2.3-wip1",
  "v1.2.3-alpha",
  "v1.2.3-alpha1",
  "v1.2.3-alpha2",
  "v1.2.3-beta",
  "v1.2.3-beta0",
  "v1.2.3-beta1",
  "v1.2.3-beta2",
  "v1.2.3-rc-1",
  "v1.2.3-rc",
  "v1.2.3-rc0",
  "v1.2.3-rc4",
  "v1.2.3-rc4invalid",
  "v1.2.3-rcinvalid",
  "vinvalid",
  "v1.2-Wip",
  "v1.2-aLpha",
  "v1.2-beTa",
  "v1.2-gamMa",
  "v1.2-Rc",
  "v1.2.3-betA2",
  "v1.2.3-Rc-1",
  "v1.2.3-rciNvalid",
  "v1.2.3-rc+isvalid",
  "vinvaLid",
  "vprefix1.2.3-rc1suffix"
};

char *comparetests[][2] = {
  { "1",            "0"           },
  { "2",            "1"           },
  { "1.2",          "1.2"         },
  { "1.2",          "1.3"         },
  { "1.3",          "1.2"         },
  { "1.2.0",        "1.3.0"       },
  { "1.2.0",        "1.2.1"       },
  { "1.2",          "1.2wip"      },
  { "1.2wip",       "1.2beta"     },
  { "1.2beta",      "1.2rc",      },
  { "1.2rc",        "1.2"         },
  { "1.2beta",      "1.2beta1"    },
  { "1.2beta1",     "1.2beta"     },
  { "1.2.1beta1",   "1.2beta"     },
  { "1.2beta10",    "1.2beta1"    },
  { "1.2beta10",    "1.2beta2"    },
  { "2.3",          "1.4"         },
  { "2.3",          "2.3.0"       },
  { "2.3",          "2.3.1"       },
  { "2.3.10",       "2.3.10"      },
  { "2.3.10wip",    "2.3.10beta"  },
  { "2.3.10beta",   "2.3.10beta0" },
  { "2.3.10beta0",  "2.3.10beta"  },
  { "2.3.10beta10", "2.3.10beta2" },
  { "2.3.10beta2",  "2.3.10beta10"},
  { "2.3.10beta2",  "2.3.10rc1"   },
  { "2.3.10rc2",    "2.3.10rc1"   },
  { "2.3.10rc2",    "2.3.10"      },
  { "2.3.10",       "2.3.10rc2"   },
  { "10.2",         "2.10"        },
  { "2.10",         "10.2"        },

  { "v1",            "0"             },
  { "2",             "v1"            },
  { "v1.2",          "1.2"           },
  { "1.2",           "v1.3"          },
  { "1.3",           "v1.2"          },
  { "v1.2.0",        "1.3.0"         },
  { "1.2.0",         "v1.2.1"        },
  { "v1.2",          "1.2-wip"       },
  { "1.2wip",        "v1.2-beta"     },
  { "v1.2beta",      "1.2-rc",       },
  { "1.2rc",         "v1.2"          },
  { "v1.2beta",      "1.2-beta1"     },
  { "1.2beta1",      "v1.2-beta"     },
  { "v1.2.1beta1",   "1.2-beta"      },
  { "1.2beta10",     "v1.2-beta1"    },
  { "v1.2beta10",    "1.2-beta2"     },
  { "2.3",           "v1.4"          },
  { "v2.3",          "2.3.0"         },
  { "2.3",           "v2.3.1"        },
  { "v2.3.10",       "2.3.10"        },
  { "2.3.10wip",     "v2.3.10-beta"  },
  { "v2.3.10beta",   "2.3.10-beta0"  },
  { "2.3.10beta0",   "v2.3.10-beta"  },
  { "v2.3.10beta10", "2.3.10-beta2"  },
  { "2.3.10beta2",   "v2.3.10-beta10"},
  { "v2.3.10beta2",  "2.3.10-rc1"    },
  { "2.3.10rc2",     "v2.3.10-rc1"   },
  { "v2.3.10rc2",    "2.3.10"        },
  { "2.3.10",        "v2.3.10-rc2"   },
  { "v10.2",         "2.10"          },
  { "2.10",          "v10.2"         },
  { "1.2.3-rc",      "v1.2.3-rc+isvalid" }
};

int main(int argc, char *argv[])
{
  unsigned int maxlen=5;
  for (unsigned int i = 0; i < sizeof(formattests) / sizeof(*formattests); i++)
  {
    if (strlen(formattests[i]) > maxlen)
      maxlen = strlen(formattests[i]);
  }

  char *formatheaderfmt = "\n\nTesting Parsing of Version Number String\n"
                          "%*s %5s %5s %5s %5s %7s (#  ok) %8s %s\n";
  char *formattestfmt   = "%*s %5s %5d %5d %5d %7s (%2d %2s) %8d %s\n";
  printf(formatheaderfmt,
         maxlen, "input",
         "valid",
         "major",
         "minor",
         "point",
         "stage",
         "substage",
         "toString");
  for (unsigned int i = 0; i < sizeof(formattests) / sizeof(*formattests); i++)
  {
    XVersion version(formattests[i]);
    char *stagestr;
    bool unusedok;
    bool stageok;
    switch (version.stage(stageok))
    {
      case XVersion::WIP:     stagestr = "WIP";     break;
      case XVersion::ALPHA:   stagestr = "ALPHA";   break;
      case XVersion::BETA:    stagestr = "BETA";    break;
      case XVersion::RC:      stagestr = "RC";      break;
      case XVersion::FINAL:   stagestr = "FINAL";   break;
      case XVersion::UNKNOWN: stagestr = "UNKNOWN"; break;
      default:                stagestr = "default"; break;
    }

    printf(formattestfmt,
           maxlen, formattests[i],
           (version.isValid() ? "T" : "F"),
           version.majorNumber(unusedok),
           version.minorNumber(unusedok),
           version.pointNumber(unusedok),
           stagestr,
           version.stage(unusedok),
           (stageok ? "Y" : "N"),
           version.substageNumber(unusedok),
           qPrintable(version.toString()));
  }

  maxlen = 5;
  for (unsigned int i = 0; i < sizeof(comparetests) / sizeof(*comparetests); i++)
  {
    if (strlen(comparetests[i][0]) > maxlen)
      maxlen = strlen(comparetests[i][0]);
    if (strlen(comparetests[i][1]) > maxlen)
      maxlen = strlen(comparetests[i][1]);
  }
  printf("\n\nTesting Comparison Operators\n");
  char *compareheaderfmt = "\n%*s %*s | %*s %*s  ==   >  >=   <  <=  !=\n";
  char *comparetestfmt   = "%*s %*s | %*s %*s %3s %3s %3s %3s %3s %3s\n";

  for (unsigned int i = 0; i < sizeof(comparetests) / sizeof(*comparetests); i++)
  {
    if (i % 20 == 0)
      printf(compareheaderfmt,
             maxlen, "left",
             maxlen, "right",
             maxlen, "left str",
             maxlen, "right str");
    XVersion left(comparetests[i][0]);
    XVersion right(comparetests[i][1]);
    printf(comparetestfmt,
         maxlen, comparetests[i][0],
         maxlen, comparetests[i][1],
         maxlen, qPrintable(left.toString()),
         maxlen, qPrintable(right.toString()),
         (left == right ? "T" : "F"),
         (left >  right ? "T" : "F"),
         (left >= right ? "T" : "F"),
         (left <  right ? "T" : "F"),
         (left <= right ? "T" : "F"),
         (left != right ? "T" : "F"));
  }
}
