#include "ProjectionEditor.h"
#include "widgets/DoubleLineEdit.h"
#include "widgets/ExtendedComboBox.h"
#include "widgets/PlainTextEdit.h"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QString>

#include <QDebug>

#include <algorithm>
#include <string>
#include <memory>

struct ZoneInfo {
    QString code;
    Projection::Type type;
    QString description;
};

const std::vector<ZoneInfo> zoneData {
    { "10131", Projection::Type::SPCS83, QStringLiteral("Alabama East (FIPS 0101)")                       },
    { "10132", Projection::Type::SPCS83, QStringLiteral("Alabama West (FIPS 0102)")                       },
    { "15031", Projection::Type::SPCS83, QStringLiteral("Alaska 1 (FIPS 5001)")                           },
    { "15032", Projection::Type::SPCS83, QStringLiteral("Alaska 2 (FIPS 5002)")                           },
    { "15033", Projection::Type::SPCS83, QStringLiteral("Alaska 3 (FIPS 5003)")                           },
    { "15034", Projection::Type::SPCS83, QStringLiteral("Alaska 4 (FIPS 5004)")                           },
    { "15035", Projection::Type::SPCS83, QStringLiteral("Alaska 5 (FIPS 5005)")                           },
    { "15036", Projection::Type::SPCS83, QStringLiteral("Alaska 6 (FIPS 5006)")                           },
    { "15037", Projection::Type::SPCS83, QStringLiteral("Alaska 7 (FIPS 5007)")                           },
    { "15038", Projection::Type::SPCS83, QStringLiteral("Alaska 8 (FIPS 5008)")                           },
    { "15039", Projection::Type::SPCS83, QStringLiteral("Alaska 9 (FIPS 5009)")                           },
    { "15040", Projection::Type::SPCS83, QStringLiteral("Alaska 10 (FIPS 5010)")                          },
    { "10231", Projection::Type::SPCS83, QStringLiteral("Arizona East (FIPS 0201)")                       },
    { "10232", Projection::Type::SPCS83, QStringLiteral("Arizona Central (FIPS 0202)")                    },
    { "10233", Projection::Type::SPCS83, QStringLiteral("Arizona West (FIPS 0203)")                       },
    { "10331", Projection::Type::SPCS83, QStringLiteral("Arkansas North (FIPS 0301)")                     },
    { "10332", Projection::Type::SPCS83, QStringLiteral("Arkansas South (FIPS 0302)")                     },
    { "10431", Projection::Type::SPCS83, QStringLiteral("California 1 (FIPS 0401)")                       },
    { "10432", Projection::Type::SPCS83, QStringLiteral("California 2 (FIPS 0402)")                       },
    { "10433", Projection::Type::SPCS83, QStringLiteral("California 3 (FIPS 0403)")                       },
    { "10434", Projection::Type::SPCS83, QStringLiteral("California 4 (FIPS 0404)")                       },
    { "10435", Projection::Type::SPCS83, QStringLiteral("California 5 (FIPS 0405)")                       },
    { "10436", Projection::Type::SPCS83, QStringLiteral("California 6 (FIPS 0406)")                       },
    { "10531", Projection::Type::SPCS83, QStringLiteral("Colorado North (FIPS 0501)")                     },
    { "10532", Projection::Type::SPCS83, QStringLiteral("Colorado Central (FIPS 0502)")                   },
    { "10533", Projection::Type::SPCS83, QStringLiteral("Colorado South (FIPS 0503)")                     },
    { "10630", Projection::Type::SPCS83, QStringLiteral("Connecticut (FIPS 0600)")                        },
    { "10730", Projection::Type::SPCS83, QStringLiteral("Delaware (FIPS 0700)")                           },
    { "10931", Projection::Type::SPCS83, QStringLiteral("Florida East (FIPS 0901)")                       },
    { "10932", Projection::Type::SPCS83, QStringLiteral("Florida West (FIPS 0902)")                       },
    { "10933", Projection::Type::SPCS83, QStringLiteral("Florida North (FIPS 0903)")                      },
    { "11031", Projection::Type::SPCS83, QStringLiteral("Georgia East (FIPS 1001)")                       },
    { "11032", Projection::Type::SPCS83, QStringLiteral("Georgia West (FIPS 1002)")                       },
    { "15131", Projection::Type::SPCS83, QStringLiteral("Hawaii 1 (FIPS 5101)")                           },
    { "15132", Projection::Type::SPCS83, QStringLiteral("Hawaii 2 (FIPS 5102)")                           },
    { "15133", Projection::Type::SPCS83, QStringLiteral("Hawaii 3 (FIPS 5103)")                           },
    { "15134", Projection::Type::SPCS83, QStringLiteral("Hawaii 4 (FIPS 5104)")                           },
    { "15135", Projection::Type::SPCS83, QStringLiteral("Hawaii 5 (FIPS 5105)")                           },
    { "11131", Projection::Type::SPCS83, QStringLiteral("Idaho East (FIPS 1101)")                         },
    { "11132", Projection::Type::SPCS83, QStringLiteral("Idaho Central (FIPS 1102)")                      },
    { "11133", Projection::Type::SPCS83, QStringLiteral("Idaho West (FIPS 1103)")                         },
    { "11231", Projection::Type::SPCS83, QStringLiteral("Illinois East (FIPS 1201)")                      },
    { "11232", Projection::Type::SPCS83, QStringLiteral("Illinois West (FIPS 1202)")                      },
    { "11331", Projection::Type::SPCS83, QStringLiteral("Indiana East (FIPS 1301)")                       },
    { "11332", Projection::Type::SPCS83, QStringLiteral("Indiana West (FIPS 1302)")                       },
    { "11431", Projection::Type::SPCS83, QStringLiteral("Iowa North (FIPS 1401)")                         },
    { "11432", Projection::Type::SPCS83, QStringLiteral("Iowa South (FIPS 1402)")                         },
    { "11531", Projection::Type::SPCS83, QStringLiteral("Kansas North (FIPS 1501)")                       },
    { "11532", Projection::Type::SPCS83, QStringLiteral("Kansas South (FIPS 1502)")                       },
    { "11630", Projection::Type::SPCS83, QStringLiteral("Kentucky One (FIPS 1600)")                       },
    { "15303", Projection::Type::SPCS83, QStringLiteral("Kentucky North (FIPS 1601)")                     },
    { "11632", Projection::Type::SPCS83, QStringLiteral("Kentucky South (FIPS 1602)")                     },
    { "11731", Projection::Type::SPCS83, QStringLiteral("Louisiana North (FIPS 1701)")                    },
    { "11732", Projection::Type::SPCS83, QStringLiteral("Louisiana South (FIPS 1702)")                    },
    { "11733", Projection::Type::SPCS83, QStringLiteral("Louisiana Offshore (FIPS 1703)")                 },
    { "11831", Projection::Type::SPCS83, QStringLiteral("Maine East (FIPS 1801)")                         },
    { "11832", Projection::Type::SPCS83, QStringLiteral("Maine West (FIPS 1802)")                         },
    { "11930", Projection::Type::SPCS83, QStringLiteral("Maryland (FIPS 1900)")                           },
    { "12031", Projection::Type::SPCS83, QStringLiteral("Massachusetts Mainland (FIPS 2001)")             },
    { "12032", Projection::Type::SPCS83, QStringLiteral("Massachusetts Island (FIPS 2002)")               },
    { "12141", Projection::Type::SPCS83, QStringLiteral("Michigan North (FIPS 2111)")                     },
    { "12142", Projection::Type::SPCS83, QStringLiteral("Michigan Central (FIPS 2112)")                   },
    { "12143", Projection::Type::SPCS83, QStringLiteral("Michigan South (FIPS 2113)")                     },
    { "12231", Projection::Type::SPCS83, QStringLiteral("Minnesota North (FIPS 2201)")                    },
    { "12232", Projection::Type::SPCS83, QStringLiteral("Minnesota Central (FIPS 2202)")                  },
    { "12233", Projection::Type::SPCS83, QStringLiteral("Minnesota South (FIPS 2203)")                    },
    { "12331", Projection::Type::SPCS83, QStringLiteral("Mississippi East (FIPS 2301)")                   },
    { "12332", Projection::Type::SPCS83, QStringLiteral("Mississippi West (FIPS 2302)")                   },
    { "12431", Projection::Type::SPCS83, QStringLiteral("Missouri East (FIPS 2401)")                      },
    { "12432", Projection::Type::SPCS83, QStringLiteral("Missouri Central (FIPS 2402)")                   },
    { "12433", Projection::Type::SPCS83, QStringLiteral("Missouri West (FIPS 2403)")                      },
    { "12530", Projection::Type::SPCS83, QStringLiteral("Montana  (FIPS 2500)")                           },
    { "12630", Projection::Type::SPCS83, QStringLiteral("Nebraska  (FIPS 2600)")                          },
    { "12731", Projection::Type::SPCS83, QStringLiteral("Nevada East (FIPS 2701)")                        },
    { "12732", Projection::Type::SPCS83, QStringLiteral("Nevada Central (FIPS 2702)")                     },
    { "12733", Projection::Type::SPCS83, QStringLiteral("Nevada West (FIPS 2703)")                        },
    { "12830", Projection::Type::SPCS83, QStringLiteral("New Hampshire (FIPS 2800)")                      },
    { "12930", Projection::Type::SPCS83, QStringLiteral("New Jersey (FIPS 2900)")                         },
    { "13031", Projection::Type::SPCS83, QStringLiteral("New Mexico East (FIPS 3001)")                    },
    { "13032", Projection::Type::SPCS83, QStringLiteral("New Mexico Central (FIPS 3002)")                 },
    { "13033", Projection::Type::SPCS83, QStringLiteral("New Mexico West (FIPS 3003)")                    },
    { "13131", Projection::Type::SPCS83, QStringLiteral("New York East (FIPS 3101)")                      },
    { "13132", Projection::Type::SPCS83, QStringLiteral("New York Central (FIPS 3102)")                   },
    { "13133", Projection::Type::SPCS83, QStringLiteral("New York West (FIPS 3103)")                      },
    { "13134", Projection::Type::SPCS83, QStringLiteral("New York Long Island (FIPS 3104)")               },
    { "13230", Projection::Type::SPCS83, QStringLiteral("North Carolina (FIPS 3200)")                     },
    { "13331", Projection::Type::SPCS83, QStringLiteral("North Dakota North (FIPS 3301)")                 },
    { "13332", Projection::Type::SPCS83, QStringLiteral("North Dakota South (FIPS 3302)")                 },
    { "13431", Projection::Type::SPCS83, QStringLiteral("Ohio North (FIPS 3401)")                         },
    { "13432", Projection::Type::SPCS83, QStringLiteral("Ohio South (FIPS 3402)")                         },
    { "13531", Projection::Type::SPCS83, QStringLiteral("Oklahoma North (FIPS 3501)")                     },
    { "13532", Projection::Type::SPCS83, QStringLiteral("Oklahoma South (FIPS 3502)")                     },
    { "13631", Projection::Type::SPCS83, QStringLiteral("Oregon North (FIPS 3601)")                       },
    { "13632", Projection::Type::SPCS83, QStringLiteral("Oregon South (FIPS 3602)")                       },
    { "13731", Projection::Type::SPCS83, QStringLiteral("Pennsylvania North (FIPS 3701)")                 },
    { "13732", Projection::Type::SPCS83, QStringLiteral("Pennsylvania South (FIPS 3702)")                 },
    { "13830", Projection::Type::SPCS83, QStringLiteral("Rhode Island  (FIPS 3800)")                      },
    { "13930", Projection::Type::SPCS83, QStringLiteral("South Carolina (FIPS 3900)")                     },
    { "14031", Projection::Type::SPCS83, QStringLiteral("South Dakota North (FIPS 4001)")                 },
    { "14032", Projection::Type::SPCS83, QStringLiteral("South Dakota South (FIPS 4002)")                 },
    { "14130", Projection::Type::SPCS83, QStringLiteral("Tennessee (FIPS 4100)")                          },
    { "14231", Projection::Type::SPCS83, QStringLiteral("Texas North (FIPS 4201)")                        },
    { "14232", Projection::Type::SPCS83, QStringLiteral("Texas North Central (FIPS 4202)")                },
    { "14233", Projection::Type::SPCS83, QStringLiteral("Texas Central (FIPS 4203)")                      },
    { "14234", Projection::Type::SPCS83, QStringLiteral("Texas South Central (FIPS 4204)")                },
    { "14235", Projection::Type::SPCS83, QStringLiteral("Texas South (FIPS 4205)")                        },
    { "14331", Projection::Type::SPCS83, QStringLiteral("Utah North (FIPS 4301)")                         },
    { "14332", Projection::Type::SPCS83, QStringLiteral("Utah Central (FIPS 4302)")                       },
    { "14333", Projection::Type::SPCS83, QStringLiteral("Utah South (FIPS 4303)")                         },
    { "14430", Projection::Type::SPCS83, QStringLiteral("Vermont (FIPS 4400)")                            },
    { "14531", Projection::Type::SPCS83, QStringLiteral("Virginia North (FIPS 4501)")                     },
    { "14532", Projection::Type::SPCS83, QStringLiteral("Virginia South (FIPS 4502)")                     },
    { "14631", Projection::Type::SPCS83, QStringLiteral("Washington North (FIPS 4601)")                   },
    { "14632", Projection::Type::SPCS83, QStringLiteral("Washington South (FIPS 4602)")                   },
    { "14731", Projection::Type::SPCS83, QStringLiteral("West Virginia North (FIPS 4701)")                },
    { "14732", Projection::Type::SPCS83, QStringLiteral("West Virginia South (FIPS 4702)")                },
    { "14831", Projection::Type::SPCS83, QStringLiteral("Wisconsin North (FIPS 4801)")                    },
    { "14832", Projection::Type::SPCS83, QStringLiteral("Wisconsin Central (FIPS 4802)")                  },
    { "14833", Projection::Type::SPCS83, QStringLiteral("Wisconsin South (FIPS 4803)")                    },
    { "14931", Projection::Type::SPCS83, QStringLiteral("Wyoming East (FIPS 4901)")                       },
    { "14932", Projection::Type::SPCS83, QStringLiteral("Wyoming East Central (FIPS 4902)")               },
    { "14933", Projection::Type::SPCS83, QStringLiteral("Wyoming West Central (FIPS 4903)")               },
    { "14934", Projection::Type::SPCS83, QStringLiteral("Wyoming West (FIPS 4904)")                       },
    { "16001", Projection::Type::UTM,    QStringLiteral("UTM Zone 1 North (180\u00b0W\u2013174\u00b0W)")  },
    { "16002", Projection::Type::UTM,    QStringLiteral("UTM Zone 2 North (174\u00b0W\u2013168\u00b0W)")  },
    { "16003", Projection::Type::UTM,    QStringLiteral("UTM Zone 3 North (168\u00b0W\u2013162\u00b0W)")  },
    { "16004", Projection::Type::UTM,    QStringLiteral("UTM Zone 4 North (162\u00b0W\u2013156\u00b0W)")  },
    { "16005", Projection::Type::UTM,    QStringLiteral("UTM Zone 5 North (156\u00b0W\u2013150\u00b0W)")  },
    { "16006", Projection::Type::UTM,    QStringLiteral("UTM Zone 6 North (150\u00b0W\u2013144\u00b0W)")  },
    { "16007", Projection::Type::UTM,    QStringLiteral("UTM Zone 7 North (144\u00b0W\u2013138\u00b0W)")  },
    { "16008", Projection::Type::UTM,    QStringLiteral("UTM Zone 8 North (138\u00b0W\u2013132\u00b0W)")  },
    { "16009", Projection::Type::UTM,    QStringLiteral("UTM Zone 9 North (132\u00b0W\u2013126\u00b0W)")  },
    { "16010", Projection::Type::UTM,    QStringLiteral("UTM Zone 10 North (126\u00b0W\u2013120\u00b0W)") },
    { "16011", Projection::Type::UTM,    QStringLiteral("UTM Zone 11 North (120\u00b0W\u2013114\u00b0W)") },
    { "16012", Projection::Type::UTM,    QStringLiteral("UTM Zone 12 North (114\u00b0W\u2013108\u00b0W)") },
    { "16013", Projection::Type::UTM,    QStringLiteral("UTM Zone 13 North (108\u00b0W\u2013102\u00b0W)") },
    { "16014", Projection::Type::UTM,    QStringLiteral("UTM Zone 14 North (102\u00b0W\u201396\u00b0W)")  },
    { "16015", Projection::Type::UTM,    QStringLiteral("UTM Zone 15 North (96\u00b0W\u201390\u00b0W)")   },
    { "16016", Projection::Type::UTM,    QStringLiteral("UTM Zone 16 North (90\u00b0W\u201384\u00b0W)")   },
    { "16017", Projection::Type::UTM,    QStringLiteral("UTM Zone 17 North (84\u00b0W\u201378\u00b0W)")   },
    { "16018", Projection::Type::UTM,    QStringLiteral("UTM Zone 18 North (78\u00b0W\u201372\u00b0W)")   },
    { "16019", Projection::Type::UTM,    QStringLiteral("UTM Zone 19 North (72\u00b0W\u201366\u00b0W)")   },
    { "16020", Projection::Type::UTM,    QStringLiteral("UTM Zone 20 North (66\u00b0W\u201360\u00b0W)")   },
    { "16021", Projection::Type::UTM,    QStringLiteral("UTM Zone 21 North (60\u00b0W\u201354\u00b0W)")   },
    { "16022", Projection::Type::UTM,    QStringLiteral("UTM Zone 22 North (54\u00b0W\u201348\u00b0W)")   },
    { "16023", Projection::Type::UTM,    QStringLiteral("UTM Zone 23 North (48\u00b0W\u201342\u00b0W)")   },
    { "16024", Projection::Type::UTM,    QStringLiteral("UTM Zone 24 North (42\u00b0W\u201336\u00b0W)")   },
    { "16025", Projection::Type::UTM,    QStringLiteral("UTM Zone 25 North (36\u00b0W\u201330\u00b0W)")   },
    { "16026", Projection::Type::UTM,    QStringLiteral("UTM Zone 26 North (30\u00b0W\u201324\u00b0W)")   },
    { "16027", Projection::Type::UTM,    QStringLiteral("UTM Zone 27 North (24\u00b0W\u201318\u00b0W)")   },
    { "16028", Projection::Type::UTM,    QStringLiteral("UTM Zone 28 North (18\u00b0W\u201312\u00b0W)")   },
    { "16029", Projection::Type::UTM,    QStringLiteral("UTM Zone 29 North (12\u00b0W\u20136\u00b0W)")    },
    { "16030", Projection::Type::UTM,    QStringLiteral("UTM Zone 30 North (6\u00b0W\u20130\u00b0W)")     },
    { "16031", Projection::Type::UTM,    QStringLiteral("UTM Zone 31 North (0\u00b0E\u20136\u00b0E)")     },
    { "16032", Projection::Type::UTM,    QStringLiteral("UTM Zone 32 North (6\u00b0E\u201312\u00b0E)")    },
    { "16033", Projection::Type::UTM,    QStringLiteral("UTM Zone 33 North (12\u00b0E\u201318\u00b0E)")   },
    { "16034", Projection::Type::UTM,    QStringLiteral("UTM Zone 34 North (18\u00b0E\u201324\u00b0E)")   },
    { "16035", Projection::Type::UTM,    QStringLiteral("UTM Zone 35 North (24\u00b0E\u201330\u00b0E)")   },
    { "16036", Projection::Type::UTM,    QStringLiteral("UTM Zone 36 North (30\u00b0E\u201336\u00b0E)")   },
    { "16037", Projection::Type::UTM,    QStringLiteral("UTM Zone 37 North (36\u00b0E\u201342\u00b0E)")   },
    { "16038", Projection::Type::UTM,    QStringLiteral("UTM Zone 38 North (42\u00b0E\u201348\u00b0E)")   },
    { "16039", Projection::Type::UTM,    QStringLiteral("UTM Zone 39 North (48\u00b0E\u201354\u00b0E)")   },
    { "16040", Projection::Type::UTM,    QStringLiteral("UTM Zone 40 North (54\u00b0E\u201360\u00b0E)")   },
    { "16041", Projection::Type::UTM,    QStringLiteral("UTM Zone 41 North (60\u00b0E\u201366\u00b0E)")   },
    { "16042", Projection::Type::UTM,    QStringLiteral("UTM Zone 42 North (66\u00b0E\u201372\u00b0E)")   },
    { "16043", Projection::Type::UTM,    QStringLiteral("UTM Zone 43 North (72\u00b0E\u201378\u00b0E)")   },
    { "16044", Projection::Type::UTM,    QStringLiteral("UTM Zone 44 North (78\u00b0E\u201384\u00b0E)")   },
    { "16045", Projection::Type::UTM,    QStringLiteral("UTM Zone 45 North (84\u00b0E\u201390\u00b0E)")   },
    { "16046", Projection::Type::UTM,    QStringLiteral("UTM Zone 46 North (90\u00b0E\u201396\u00b0E)")   },
    { "16047", Projection::Type::UTM,    QStringLiteral("UTM Zone 47 North (96\u00b0E\u2013102\u00b0E)")  },
    { "16048", Projection::Type::UTM,    QStringLiteral("UTM Zone 48 North (102\u00b0E\u2013108\u00b0E)") },
    { "16049", Projection::Type::UTM,    QStringLiteral("UTM Zone 49 North (108\u00b0E\u2013114\u00b0E)") },
    { "16050", Projection::Type::UTM,    QStringLiteral("UTM Zone 50 North (114\u00b0E\u2013120\u00b0E)") },
    { "16051", Projection::Type::UTM,    QStringLiteral("UTM Zone 51 North (120\u00b0E\u2013126\u00b0E)") },
    { "16052", Projection::Type::UTM,    QStringLiteral("UTM Zone 52 North (126\u00b0E\u2013132\u00b0E)") },
    { "16053", Projection::Type::UTM,    QStringLiteral("UTM Zone 53 North (132\u00b0E\u2013138\u00b0E)") },
    { "16054", Projection::Type::UTM,    QStringLiteral("UTM Zone 54 North (138\u00b0E\u2013144\u00b0E)") },
    { "16055", Projection::Type::UTM,    QStringLiteral("UTM Zone 55 North (144\u00b0E\u2013150\u00b0E)") },
    { "16056", Projection::Type::UTM,    QStringLiteral("UTM Zone 56 North (150\u00b0E\u2013156\u00b0E)") },
    { "16057", Projection::Type::UTM,    QStringLiteral("UTM Zone 57 North (156\u00b0E\u2013162\u00b0E)") },
    { "16058", Projection::Type::UTM,    QStringLiteral("UTM Zone 58 North (162\u00b0E\u2013168\u00b0E)") },
    { "16059", Projection::Type::UTM,    QStringLiteral("UTM Zone 59 North (168\u00b0E\u2013174\u00b0E)") },
    { "16060", Projection::Type::UTM,    QStringLiteral("UTM Zone 60 North (174\u00b0E\u2013180\u00b0E)") },
    { "16101", Projection::Type::UTM,    QStringLiteral("UTM Zone 1 South (180\u00b0W\u2013174\u00b0W)")  },
    { "16102", Projection::Type::UTM,    QStringLiteral("UTM Zone 2 South (174\u00b0W\u2013168\u00b0W)")  },
    { "16103", Projection::Type::UTM,    QStringLiteral("UTM Zone 3 South (168\u00b0W\u2013162\u00b0W)")  },
    { "16104", Projection::Type::UTM,    QStringLiteral("UTM Zone 4 South (162\u00b0W\u2013156\u00b0W)")  },
    { "16105", Projection::Type::UTM,    QStringLiteral("UTM Zone 5 South (156\u00b0W\u2013150\u00b0W)")  },
    { "16106", Projection::Type::UTM,    QStringLiteral("UTM Zone 6 South (150\u00b0W\u2013144\u00b0W)")  },
    { "16107", Projection::Type::UTM,    QStringLiteral("UTM Zone 7 South (144\u00b0W\u2013138\u00b0W)")  },
    { "16108", Projection::Type::UTM,    QStringLiteral("UTM Zone 8 South (138\u00b0W\u2013132\u00b0W)")  },
    { "16109", Projection::Type::UTM,    QStringLiteral("UTM Zone 9 South (132\u00b0W\u2013126\u00b0W)")  },
    { "16110", Projection::Type::UTM,    QStringLiteral("UTM Zone 10 South (126\u00b0W\u2013120\u00b0W)") },
    { "16111", Projection::Type::UTM,    QStringLiteral("UTM Zone 11 South (120\u00b0W\u2013114\u00b0W)") },
    { "16112", Projection::Type::UTM,    QStringLiteral("UTM Zone 12 South (114\u00b0W\u2013108\u00b0W)") },
    { "16113", Projection::Type::UTM,    QStringLiteral("UTM Zone 13 South (108\u00b0W\u2013102\u00b0W)") },
    { "16114", Projection::Type::UTM,    QStringLiteral("UTM Zone 14 South (102\u00b0W\u201396\u00b0W)")  },
    { "16115", Projection::Type::UTM,    QStringLiteral("UTM Zone 15 South (96\u00b0W\u201390\u00b0W)")   },
    { "16116", Projection::Type::UTM,    QStringLiteral("UTM Zone 16 South (90\u00b0W\u201384\u00b0W)")   },
    { "16117", Projection::Type::UTM,    QStringLiteral("UTM Zone 17 South (84\u00b0W\u201378\u00b0W)")   },
    { "16118", Projection::Type::UTM,    QStringLiteral("UTM Zone 18 South (78\u00b0W\u201372\u00b0W)")   },
    { "16119", Projection::Type::UTM,    QStringLiteral("UTM Zone 19 South (72\u00b0W\u201366\u00b0W)")   },
    { "16120", Projection::Type::UTM,    QStringLiteral("UTM Zone 20 South (66\u00b0W\u201360\u00b0W)")   },
    { "16121", Projection::Type::UTM,    QStringLiteral("UTM Zone 21 South (60\u00b0W\u201354\u00b0W)")   },
    { "16122", Projection::Type::UTM,    QStringLiteral("UTM Zone 22 South (54\u00b0W\u201348\u00b0W)")   },
    { "16123", Projection::Type::UTM,    QStringLiteral("UTM Zone 23 South (48\u00b0W\u201342\u00b0W)")   },
    { "16124", Projection::Type::UTM,    QStringLiteral("UTM Zone 24 South (42\u00b0W\u201336\u00b0W)")   },
    { "16125", Projection::Type::UTM,    QStringLiteral("UTM Zone 25 South (36\u00b0W\u201330\u00b0W)")   },
    { "16126", Projection::Type::UTM,    QStringLiteral("UTM Zone 26 South (30\u00b0W\u201324\u00b0W)")   },
    { "16127", Projection::Type::UTM,    QStringLiteral("UTM Zone 27 South (24\u00b0W\u201318\u00b0W)")   },
    { "16128", Projection::Type::UTM,    QStringLiteral("UTM Zone 28 South (18\u00b0W\u201312\u00b0W)")   },
    { "16129", Projection::Type::UTM,    QStringLiteral("UTM Zone 29 South (12\u00b0W\u20136\u00b0W)")    },
    { "16130", Projection::Type::UTM,    QStringLiteral("UTM Zone 30 South (6\u00b0W\u20130\u00b0W)")     },
    { "16131", Projection::Type::UTM,    QStringLiteral("UTM Zone 31 South (0\u00b0E\u20136\u00b0E)")     },
    { "16132", Projection::Type::UTM,    QStringLiteral("UTM Zone 32 South (6\u00b0E\u201312\u00b0E)")    },
    { "16133", Projection::Type::UTM,    QStringLiteral("UTM Zone 33 South (12\u00b0E\u201318\u00b0E)")   },
    { "16134", Projection::Type::UTM,    QStringLiteral("UTM Zone 34 South (18\u00b0E\u201324\u00b0E)")   },
    { "16135", Projection::Type::UTM,    QStringLiteral("UTM Zone 35 South (24\u00b0E\u201330\u00b0E)")   },
    { "16136", Projection::Type::UTM,    QStringLiteral("UTM Zone 36 South (30\u00b0E\u201336\u00b0E)")   },
    { "16137", Projection::Type::UTM,    QStringLiteral("UTM Zone 37 South (36\u00b0E\u201342\u00b0E)")   },
    { "16138", Projection::Type::UTM,    QStringLiteral("UTM Zone 38 South (42\u00b0E\u201348\u00b0E)")   },
    { "16139", Projection::Type::UTM,    QStringLiteral("UTM Zone 39 South (48\u00b0E\u201354\u00b0E)")   },
    { "16140", Projection::Type::UTM,    QStringLiteral("UTM Zone 40 South (54\u00b0E\u201360\u00b0E)")   },
    { "16141", Projection::Type::UTM,    QStringLiteral("UTM Zone 41 South (60\u00b0E\u201366\u00b0E)")   },
    { "16142", Projection::Type::UTM,    QStringLiteral("UTM Zone 42 South (66\u00b0E\u201372\u00b0E)")   },
    { "16143", Projection::Type::UTM,    QStringLiteral("UTM Zone 43 South (72\u00b0E\u201378\u00b0E)")   },
    { "16144", Projection::Type::UTM,    QStringLiteral("UTM Zone 44 South (78\u00b0E\u201384\u00b0E)")   },
    { "16145", Projection::Type::UTM,    QStringLiteral("UTM Zone 45 South (84\u00b0E\u201390\u00b0E)")   },
    { "16146", Projection::Type::UTM,    QStringLiteral("UTM Zone 46 South (90\u00b0E\u201396\u00b0E)")   },
    { "16147", Projection::Type::UTM,    QStringLiteral("UTM Zone 47 South (96\u00b0E\u2013102\u00b0E)")  },
    { "16148", Projection::Type::UTM,    QStringLiteral("UTM Zone 48 South (102\u00b0E\u2013108\u00b0E)") },
    { "16149", Projection::Type::UTM,    QStringLiteral("UTM Zone 49 South (108\u00b0E\u2013114\u00b0E)") },
    { "16150", Projection::Type::UTM,    QStringLiteral("UTM Zone 50 South (114\u00b0E\u2013120\u00b0E)") },
    { "16151", Projection::Type::UTM,    QStringLiteral("UTM Zone 51 South (120\u00b0E\u2013126\u00b0E)") },
    { "16152", Projection::Type::UTM,    QStringLiteral("UTM Zone 52 South (126\u00b0E\u2013132\u00b0E)") },
    { "16153", Projection::Type::UTM,    QStringLiteral("UTM Zone 53 South (132\u00b0E\u2013138\u00b0E)") },
    { "16154", Projection::Type::UTM,    QStringLiteral("UTM Zone 54 South (138\u00b0E\u2013144\u00b0E)") },
    { "16155", Projection::Type::UTM,    QStringLiteral("UTM Zone 55 South (144\u00b0E\u2013150\u00b0E)") },
    { "16156", Projection::Type::UTM,    QStringLiteral("UTM Zone 56 South (150\u00b0E\u2013156\u00b0E)") },
    { "16157", Projection::Type::UTM,    QStringLiteral("UTM Zone 57 South (156\u00b0E\u2013162\u00b0E)") },
    { "16158", Projection::Type::UTM,    QStringLiteral("UTM Zone 58 South (162\u00b0E\u2013168\u00b0E)") },
    { "16159", Projection::Type::UTM,    QStringLiteral("UTM Zone 59 South (168\u00b0E\u2013174\u00b0E)") },
    { "16160", Projection::Type::UTM,    QStringLiteral("UTM Zone 60 South (174\u00b0E\u2013180\u00b0E)") }
};

//-----------------------------------------------------------------------------
// ZoneModel
//-----------------------------------------------------------------------------

class ZoneModel : public QAbstractListModel
{
public:
    explicit ZoneModel(QObject *parent = nullptr)
        : QAbstractListModel(parent)
    {}

    void setType(Projection::Type type)
    {
        beginResetModel();
        currentType_ = type;
        lower_ = std::lower_bound(zoneData.begin(), zoneData.end(), type,
            [&](const auto& item, const auto& x) { return item.type < x; });
        upper_ = std::upper_bound(zoneData.begin(), zoneData.end(), type,
            [&](const auto& x, const auto& item) { return x < item.type; });
        endResetModel();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return static_cast<int>(std::distance(lower_, upper_));
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return 1;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        auto it = std::next(lower_, index.row());
        if (it != zoneData.end()) {
            if (role == Qt::DisplayRole)
                return it->description;
            else if (role == Qt::UserRole)
                return it->code;
        }
        return QVariant();
    }

private:
    Projection::Type currentType_;
    std::vector<ZoneInfo>::const_iterator lower_;
    std::vector<ZoneInfo>::const_iterator upper_;
};

//-----------------------------------------------------------------------------
// ProjectionEditor
//-----------------------------------------------------------------------------

ProjectionEditor::ProjectionEditor(QWidget *parent)
    : QWidget(parent)
{
    // Horizontal
    cboProjection = new QComboBox;
    cboProjection->addItem("Local", static_cast<int>(Projection::Type::Local));
    cboProjection->addItem("State Plane Coordinate System", static_cast<int>(Projection::Type::SPCS83));
    cboProjection->addItem("Universal Transverse Mercator", static_cast<int>(Projection::Type::UTM));

    zoneModel = new ZoneModel;
    cboZone = new ExtendedComboBox;
    cboZone->setPopupHeight(300);
    cboZone->setModel(zoneModel);

    cboHDatum = new QComboBox;
    cboHDatum->addItem("NAD83(2011)",        "6318");
    cboHDatum->addItem("NAD83(NSRS2007)",    "4759");
    cboHDatum->addItem("NAD83(FBN)",         "8860");
    cboHDatum->addItem("NAD83(HARN)",        "4152");
    cboHDatum->addItem("NAD83(1986)",        "4269");

    cboHUnits = new QComboBox;
    cboHUnits->addItem("Meter",              "9001");
    //cboHUnits->addItem("International Foot", "9002");
    //cboHUnits->addItem("US Survey Foot",     "9003");

    // Vertical
    cboVDatum = new QComboBox;
    cboVDatum->addItem("NAVD88",             "5103");

    cboVUnits = new QComboBox;
    cboVUnits->addItem("Meter",              "9001");
    //cboVUnits->addItem("International Foot", "9002");
    //cboVUnits->addItem("US Survey Foot",     "9003");

    // Preview
    textPreview = new PlainTextEdit;
    textPreview->setReadOnly(true);
    textPreview->setLineCount(4);

    // Horizontal Layout
    QGroupBox *hGroupBox = new QGroupBox(tr("Horizontal"));
    QGridLayout *hLayout = new QGridLayout;
    hLayout->setColumnMinimumWidth(0, 125);
    hLayout->setColumnStretch(0, 0);
    hLayout->setColumnStretch(1, 1);
    hLayout->addWidget(new QLabel(tr("Projection:")), 0, 0);
    hLayout->addWidget(cboProjection, 0, 1);
    hLayout->addWidget(new QLabel(tr("Zone:")), 1, 0);
    hLayout->addWidget(cboZone, 1, 1);
    hLayout->addWidget(new QLabel(tr("Datum:")), 2, 0);
    hLayout->addWidget(cboHDatum, 2, 1);
    hLayout->addWidget(new QLabel(tr("Units:")), 3, 0);
    hLayout->addWidget(cboHUnits, 3, 1);
    hGroupBox->setLayout(hLayout);

    // Vertical Layout
    QGroupBox *vGroupBox = new QGroupBox(tr("Vertical"));
    QGridLayout *vLayout = new QGridLayout;
    vLayout->setColumnMinimumWidth(0, 125);
    vLayout->setColumnStretch(0, 0);
    vLayout->setColumnStretch(1, 1);
    vLayout->addWidget(new QLabel(tr("Datum:")), 0, 0);
    vLayout->addWidget(cboVDatum, 0, 1);
    vLayout->addWidget(new QLabel(tr("Units:")), 1, 0);
    vLayout->addWidget(cboVUnits, 1, 1);
    vGroupBox->setLayout(vLayout);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(hGroupBox);
    mainLayout->addWidget(vGroupBox);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(new QLabel(tr("Projection String")));
    mainLayout->addWidget(textPreview);
    mainLayout->addStretch(1);

    setLayout(mainLayout);

    cboHUnits->setCurrentIndex(0);
    cboVUnits->setCurrentIndex(0);
    cboHDatum->setCurrentIndex(-1);
    cboHDatum->setDisabled(true);

    setupConnections();
}

void ProjectionEditor::setConversionCode(const std::string& code)
{
    QString str = QString::fromStdString(code);

    auto it = std::find_if(zoneData.begin(), zoneData.end(),
        [&](const auto& item) { return item.code == str; });

    if (it != zoneData.end())
    {
        const ZoneInfo& info = *it;
        int typeindex = cboProjection->findData(static_cast<int>(info.type));
        if (typeindex >= 0)
            cboProjection->setCurrentIndex(typeindex);

        auto lower = std::lower_bound(zoneData.begin(), zoneData.end(), info.type,
            [&](const auto& item, const auto& x) { return item.type < x; });

        std::size_t zoneindex = std::distance(lower, it);
        if (zoneindex >= 0)
            cboZone->setCurrentIndex(static_cast<int>(zoneindex));
    }
}

void ProjectionEditor::setHDatumCode(const std::string& code)
{
    QString str = QString::fromStdString(code);
    int index = cboHDatum->findData(str);
    if (index >= 0)
        cboHDatum->setCurrentIndex(index);
}

void ProjectionEditor::setHUnitsCode(const std::string& code)
{
    QString str = QString::fromStdString(code);
    int index = cboHUnits->findData(str);
    if (index >= 0)
        cboHUnits->setCurrentIndex(index);
}

void ProjectionEditor::setVDatumCode(const std::string& code)
{
    QString str = QString::fromStdString(code);
    int index = cboVDatum->findData(str);
    if (index >= 0)
        cboVDatum->setCurrentIndex(index);
}

void ProjectionEditor::setVUnitsCode(const std::string& code)
{
    QString str = QString::fromStdString(code);
    int index = cboVUnits->findData(str);
    if (index >= 0)
        cboVUnits->setCurrentIndex(index);
}

std::string ProjectionEditor::conversionCode() const
{
    return qvariant_cast<QString>(cboZone->currentData()).toStdString();
}

std::string ProjectionEditor::hDatumCode() const
{
    return qvariant_cast<QString>(cboHDatum->currentData()).toStdString();
}

std::string ProjectionEditor::hUnitsCode() const
{
    return qvariant_cast<QString>(cboHUnits->currentData()).toStdString();
}

std::string ProjectionEditor::vDatumCode() const
{
    return qvariant_cast<QString>(cboVDatum->currentData()).toStdString();
}

std::string ProjectionEditor::vUnitsCode() const
{
    return qvariant_cast<QString>(cboVUnits->currentData()).toStdString();
}

void ProjectionEditor::setupConnections()
{
    connect(cboProjection, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProjectionEditor::onProjectionChanged);

    connect(cboZone, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int) { updatePreview(); });

    connect(cboHDatum, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int) { updatePreview(); });

    connect(cboHUnits, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int) { updatePreview(); });

    connect(cboVDatum, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int) { updatePreview(); });

    connect(cboVUnits, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int) { updatePreview(); });
}

void ProjectionEditor::updatePreview()
{
    textPreview->clear();

    if (cboProjection->currentIndex() >= 0 &&
        cboZone->currentIndex() >= 0 &&
        cboHDatum->currentIndex() >= 0 &&
        cboHUnits->currentIndex() >= 0 &&
        cboVDatum->currentIndex() >= 0 &&
        cboVUnits->currentIndex() >= 0)
    {
        auto p = Projection::Generic(
            conversionCode(), hDatumCode(), hUnitsCode(), vDatumCode(), vUnitsCode());

        std::string crs = p.compoundCRS();
        textPreview->setPlainText(crs);
    }
}

void ProjectionEditor::onProjectionChanged(int index)
{
    switch (index) {
    case 0: // Local
        cboZone->setCurrentIndex(-1);
        cboZone->setEnabled(false);
        cboHDatum->setCurrentIndex(-1);
        cboHDatum->setEnabled(false);
        break;
    case 1: // SPCS83
        zoneModel->setType(Projection::Type::SPCS83);
        cboZone->setCurrentIndex(0);
        cboZone->setEnabled(true);
        cboHDatum->setCurrentIndex(0);
        cboHDatum->setEnabled(true);
        break;
    case 2: // UTM
        zoneModel->setType(Projection::Type::UTM);
        cboZone->setCurrentIndex(0);
        cboZone->setEnabled(true);
        cboHDatum->setCurrentIndex(0);
        cboHDatum->setEnabled(true);
        break;
    }

    updatePreview();
}
