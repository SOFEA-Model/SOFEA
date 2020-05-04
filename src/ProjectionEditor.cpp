// Copyright 2020 Dow, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "ProjectionEditor.h"
#include "core/Common.h"
#include "widgets/DoubleLineEdit.h"
#include "widgets/ExtendedComboBox.h"
#include "widgets/ReadOnlyLineEdit.h"
#include "widgets/PlainTextEdit.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QString>

#include <QDebug>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <algorithm>
#include <string>
#include <memory>
#include <vector>

struct ZoneInfo {
    enum ZoneType {
        Local = 1,
        SPCS27,
        SPCS83,
        UTM
    };

    QString code;
    ZoneType type;
    QString description;
};

const std::vector<ZoneInfo> zoneData {
    { "10101", ZoneInfo::SPCS27, QStringLiteral("Alabama East")                                   },
    { "10102", ZoneInfo::SPCS27, QStringLiteral("Alabama West")                                   },
    { "15001", ZoneInfo::SPCS27, QStringLiteral("Alaska 1")                                       },
    { "15002", ZoneInfo::SPCS27, QStringLiteral("Alaska 2")                                       },
    { "15003", ZoneInfo::SPCS27, QStringLiteral("Alaska 3")                                       },
    { "15004", ZoneInfo::SPCS27, QStringLiteral("Alaska 4")                                       },
    { "15005", ZoneInfo::SPCS27, QStringLiteral("Alaska 5")                                       },
    { "15006", ZoneInfo::SPCS27, QStringLiteral("Alaska 6")                                       },
    { "15007", ZoneInfo::SPCS27, QStringLiteral("Alaska 7")                                       },
    { "15008", ZoneInfo::SPCS27, QStringLiteral("Alaska 8")                                       },
    { "15009", ZoneInfo::SPCS27, QStringLiteral("Alaska 9")                                       },
    { "15010", ZoneInfo::SPCS27, QStringLiteral("Alaska 10")                                      },
    { "10301", ZoneInfo::SPCS27, QStringLiteral("Arkansas North")                                 },
    { "10302", ZoneInfo::SPCS27, QStringLiteral("Arkansas South")                                 },
    { "10401", ZoneInfo::SPCS27, QStringLiteral("California 1")                                   },
    { "10402", ZoneInfo::SPCS27, QStringLiteral("California 2")                                   },
    { "10403", ZoneInfo::SPCS27, QStringLiteral("California 3")                                   },
    { "10404", ZoneInfo::SPCS27, QStringLiteral("California 4")                                   },
    { "10405", ZoneInfo::SPCS27, QStringLiteral("California 5")                                   },
    { "10406", ZoneInfo::SPCS27, QStringLiteral("California 6")                                   },
    { "10408", ZoneInfo::SPCS27, QStringLiteral("California 7")                                   },
    { "10502", ZoneInfo::SPCS27, QStringLiteral("Colorado Central")                               },
    { "10501", ZoneInfo::SPCS27, QStringLiteral("Colorado North")                                 },
    { "10503", ZoneInfo::SPCS27, QStringLiteral("Colorado South")                                 },
    { "10600", ZoneInfo::SPCS27, QStringLiteral("Connecticut")                                    },
    { "10700", ZoneInfo::SPCS27, QStringLiteral("Delaware")                                       },
    { "10901", ZoneInfo::SPCS27, QStringLiteral("Florida East")                                   },
    { "10903", ZoneInfo::SPCS27, QStringLiteral("Florida North")                                  },
    { "10902", ZoneInfo::SPCS27, QStringLiteral("Florida West")                                   },
    { "11001", ZoneInfo::SPCS27, QStringLiteral("Georgia East")                                   },
    { "11002", ZoneInfo::SPCS27, QStringLiteral("Georgia West")                                   },
    { "15101", ZoneInfo::SPCS27, QStringLiteral("Hawaii 1")                                       },
    { "15102", ZoneInfo::SPCS27, QStringLiteral("Hawaii 2")                                       },
    { "15103", ZoneInfo::SPCS27, QStringLiteral("Hawaii 3")                                       },
    { "15104", ZoneInfo::SPCS27, QStringLiteral("Hawaii 4")                                       },
    { "15105", ZoneInfo::SPCS27, QStringLiteral("Hawaii 5")                                       },
    { "11102", ZoneInfo::SPCS27, QStringLiteral("Idaho Central")                                  },
    { "11101", ZoneInfo::SPCS27, QStringLiteral("Idaho East")                                     },
    { "11103", ZoneInfo::SPCS27, QStringLiteral("Idaho West")                                     },
    { "11201", ZoneInfo::SPCS27, QStringLiteral("Illinois East")                                  },
    { "11202", ZoneInfo::SPCS27, QStringLiteral("Illinois West")                                  },
    { "11301", ZoneInfo::SPCS27, QStringLiteral("Indiana East")                                   },
    { "11302", ZoneInfo::SPCS27, QStringLiteral("Indiana West")                                   },
    { "11401", ZoneInfo::SPCS27, QStringLiteral("Iowa North")                                     },
    { "11402", ZoneInfo::SPCS27, QStringLiteral("Iowa South")                                     },
    { "11501", ZoneInfo::SPCS27, QStringLiteral("Kansas North")                                   },
    { "11502", ZoneInfo::SPCS27, QStringLiteral("Kansas South")                                   },
    { "11601", ZoneInfo::SPCS27, QStringLiteral("Kentucky North")                                 },
    { "11602", ZoneInfo::SPCS27, QStringLiteral("Kentucky South")                                 },
    { "11701", ZoneInfo::SPCS27, QStringLiteral("Louisiana North")                                },
    { "11703", ZoneInfo::SPCS27, QStringLiteral("Louisiana Offshore")                             },
    { "11702", ZoneInfo::SPCS27, QStringLiteral("Louisiana South")                                },
    { "11801", ZoneInfo::SPCS27, QStringLiteral("Maine East")                                     },
    { "11802", ZoneInfo::SPCS27, QStringLiteral("Maine West")                                     },
    { "11900", ZoneInfo::SPCS27, QStringLiteral("Maryland")                                       },
    { "12002", ZoneInfo::SPCS27, QStringLiteral("Massachusetts Island")                           },
    { "12001", ZoneInfo::SPCS27, QStringLiteral("Massachusetts Mainland")                         },
    { "6198",  ZoneInfo::SPCS27, QStringLiteral("Michigan Central")                               },
    { "6965",  ZoneInfo::SPCS27, QStringLiteral("Michigan North")                                 },
    { "6199",  ZoneInfo::SPCS27, QStringLiteral("Michigan South")                                 },
    { "12202", ZoneInfo::SPCS27, QStringLiteral("Minnesota Central")                              },
    { "12201", ZoneInfo::SPCS27, QStringLiteral("Minnesota North")                                },
    { "12203", ZoneInfo::SPCS27, QStringLiteral("Minnesota South")                                },
    { "12301", ZoneInfo::SPCS27, QStringLiteral("Mississippi East")                               },
    { "12302", ZoneInfo::SPCS27, QStringLiteral("Mississippi West")                               },
    { "12402", ZoneInfo::SPCS27, QStringLiteral("Missouri Central")                               },
    { "12401", ZoneInfo::SPCS27, QStringLiteral("Missouri East")                                  },
    { "12403", ZoneInfo::SPCS27, QStringLiteral("Missouri West")                                  },
    { "12502", ZoneInfo::SPCS27, QStringLiteral("Montana Central")                                },
    { "12501", ZoneInfo::SPCS27, QStringLiteral("Montana North")                                  },
    { "12503", ZoneInfo::SPCS27, QStringLiteral("Montana South")                                  },
    { "12601", ZoneInfo::SPCS27, QStringLiteral("Nebraska North")                                 },
    { "12602", ZoneInfo::SPCS27, QStringLiteral("Nebraska South")                                 },
    { "12702", ZoneInfo::SPCS27, QStringLiteral("Nevada Central")                                 },
    { "12701", ZoneInfo::SPCS27, QStringLiteral("Nevada East")                                    },
    { "12703", ZoneInfo::SPCS27, QStringLiteral("Nevada West")                                    },
    { "12800", ZoneInfo::SPCS27, QStringLiteral("New Hampshire")                                  },
    { "12900", ZoneInfo::SPCS27, QStringLiteral("New Jersey")                                     },
    { "13002", ZoneInfo::SPCS27, QStringLiteral("New Mexico Central")                             },
    { "13001", ZoneInfo::SPCS27, QStringLiteral("New Mexico East")                                },
    { "13003", ZoneInfo::SPCS27, QStringLiteral("New Mexico West")                                },
    { "13102", ZoneInfo::SPCS27, QStringLiteral("New York Central")                               },
    { "13101", ZoneInfo::SPCS27, QStringLiteral("New York East")                                  },
    { "4454",  ZoneInfo::SPCS27, QStringLiteral("New York Long Island")                           },
    { "13103", ZoneInfo::SPCS27, QStringLiteral("New York West")                                  },
    { "13200", ZoneInfo::SPCS27, QStringLiteral("North Carolina")                                 },
    { "13301", ZoneInfo::SPCS27, QStringLiteral("North Dakota North")                             },
    { "13302", ZoneInfo::SPCS27, QStringLiteral("North Dakota South")                             },
    { "13401", ZoneInfo::SPCS27, QStringLiteral("Ohio North")                                     },
    { "13402", ZoneInfo::SPCS27, QStringLiteral("Ohio South")                                     },
    { "13501", ZoneInfo::SPCS27, QStringLiteral("Oklahoma North")                                 },
    { "13502", ZoneInfo::SPCS27, QStringLiteral("Oklahoma South")                                 },
    { "13601", ZoneInfo::SPCS27, QStringLiteral("Oregon North")                                   },
    { "13602", ZoneInfo::SPCS27, QStringLiteral("Oregon South")                                   },
    { "13701", ZoneInfo::SPCS27, QStringLiteral("Pennsylvania North")                             },
    { "4436",  ZoneInfo::SPCS27, QStringLiteral("Pennsylvania South")                             },
    { "15201", ZoneInfo::SPCS27, QStringLiteral("Puerto Rico")                                    },
    { "13800", ZoneInfo::SPCS27, QStringLiteral("Rhode Island")                                   },
    { "13901", ZoneInfo::SPCS27, QStringLiteral("South Carolina North")                           },
    { "13902", ZoneInfo::SPCS27, QStringLiteral("South Carolina South")                           },
    { "14001", ZoneInfo::SPCS27, QStringLiteral("South Dakota North")                             },
    { "14002", ZoneInfo::SPCS27, QStringLiteral("South Dakota South")                             },
    { "15202", ZoneInfo::SPCS27, QStringLiteral("St. Croix")                                      },
    { "15302", ZoneInfo::SPCS27, QStringLiteral("Tennessee")                                      },
    { "14203", ZoneInfo::SPCS27, QStringLiteral("Texas Central")                                  },
    { "14201", ZoneInfo::SPCS27, QStringLiteral("Texas North")                                    },
    { "14202", ZoneInfo::SPCS27, QStringLiteral("Texas North Central")                            },
    { "14205", ZoneInfo::SPCS27, QStringLiteral("Texas South")                                    },
    { "14204", ZoneInfo::SPCS27, QStringLiteral("Texas South Central")                            },
    { "14302", ZoneInfo::SPCS27, QStringLiteral("Utah Central")                                   },
    { "14301", ZoneInfo::SPCS27, QStringLiteral("Utah North")                                     },
    { "14303", ZoneInfo::SPCS27, QStringLiteral("Utah South")                                     },
    { "14400", ZoneInfo::SPCS27, QStringLiteral("Vermont")                                        },
    { "14501", ZoneInfo::SPCS27, QStringLiteral("Virginia North")                                 },
    { "14502", ZoneInfo::SPCS27, QStringLiteral("Virginia South")                                 },
    { "14601", ZoneInfo::SPCS27, QStringLiteral("Washington North")                               },
    { "14602", ZoneInfo::SPCS27, QStringLiteral("Washington South")                               },
    { "14701", ZoneInfo::SPCS27, QStringLiteral("West Virginia North")                            },
    { "14702", ZoneInfo::SPCS27, QStringLiteral("West Virginia South")                            },
    { "14802", ZoneInfo::SPCS27, QStringLiteral("Wisconsin Central")                              },
    { "14801", ZoneInfo::SPCS27, QStringLiteral("Wisconsin North")                                },
    { "14803", ZoneInfo::SPCS27, QStringLiteral("Wisconsin South")                                },
    { "14901", ZoneInfo::SPCS27, QStringLiteral("Wyoming East")                                   },
    { "14902", ZoneInfo::SPCS27, QStringLiteral("Wyoming East Central")                           },
    { "14904", ZoneInfo::SPCS27, QStringLiteral("Wyoming West")                                   },
    { "14903", ZoneInfo::SPCS27, QStringLiteral("Wyoming West Central")                           },
    { "10131", ZoneInfo::SPCS83, QStringLiteral("Alabama East (FIPS 0101)")                       },
    { "10132", ZoneInfo::SPCS83, QStringLiteral("Alabama West (FIPS 0102)")                       },
    { "15031", ZoneInfo::SPCS83, QStringLiteral("Alaska 1 (FIPS 5001)")                           },
    { "15032", ZoneInfo::SPCS83, QStringLiteral("Alaska 2 (FIPS 5002)")                           },
    { "15033", ZoneInfo::SPCS83, QStringLiteral("Alaska 3 (FIPS 5003)")                           },
    { "15034", ZoneInfo::SPCS83, QStringLiteral("Alaska 4 (FIPS 5004)")                           },
    { "15035", ZoneInfo::SPCS83, QStringLiteral("Alaska 5 (FIPS 5005)")                           },
    { "15036", ZoneInfo::SPCS83, QStringLiteral("Alaska 6 (FIPS 5006)")                           },
    { "15037", ZoneInfo::SPCS83, QStringLiteral("Alaska 7 (FIPS 5007)")                           },
    { "15038", ZoneInfo::SPCS83, QStringLiteral("Alaska 8 (FIPS 5008)")                           },
    { "15039", ZoneInfo::SPCS83, QStringLiteral("Alaska 9 (FIPS 5009)")                           },
    { "15040", ZoneInfo::SPCS83, QStringLiteral("Alaska 10 (FIPS 5010)")                          },
    { "10231", ZoneInfo::SPCS83, QStringLiteral("Arizona East (FIPS 0201)")                       },
    { "10232", ZoneInfo::SPCS83, QStringLiteral("Arizona Central (FIPS 0202)")                    },
    { "10233", ZoneInfo::SPCS83, QStringLiteral("Arizona West (FIPS 0203)")                       },
    { "10331", ZoneInfo::SPCS83, QStringLiteral("Arkansas North (FIPS 0301)")                     },
    { "10332", ZoneInfo::SPCS83, QStringLiteral("Arkansas South (FIPS 0302)")                     },
    { "10431", ZoneInfo::SPCS83, QStringLiteral("California 1 (FIPS 0401)")                       },
    { "10432", ZoneInfo::SPCS83, QStringLiteral("California 2 (FIPS 0402)")                       },
    { "10433", ZoneInfo::SPCS83, QStringLiteral("California 3 (FIPS 0403)")                       },
    { "10434", ZoneInfo::SPCS83, QStringLiteral("California 4 (FIPS 0404)")                       },
    { "10435", ZoneInfo::SPCS83, QStringLiteral("California 5 (FIPS 0405)")                       },
    { "10436", ZoneInfo::SPCS83, QStringLiteral("California 6 (FIPS 0406)")                       },
    { "10531", ZoneInfo::SPCS83, QStringLiteral("Colorado North (FIPS 0501)")                     },
    { "10532", ZoneInfo::SPCS83, QStringLiteral("Colorado Central (FIPS 0502)")                   },
    { "10533", ZoneInfo::SPCS83, QStringLiteral("Colorado South (FIPS 0503)")                     },
    { "10630", ZoneInfo::SPCS83, QStringLiteral("Connecticut (FIPS 0600)")                        },
    { "10730", ZoneInfo::SPCS83, QStringLiteral("Delaware (FIPS 0700)")                           },
    { "10931", ZoneInfo::SPCS83, QStringLiteral("Florida East (FIPS 0901)")                       },
    { "10932", ZoneInfo::SPCS83, QStringLiteral("Florida West (FIPS 0902)")                       },
    { "10933", ZoneInfo::SPCS83, QStringLiteral("Florida North (FIPS 0903)")                      },
    { "11031", ZoneInfo::SPCS83, QStringLiteral("Georgia East (FIPS 1001)")                       },
    { "11032", ZoneInfo::SPCS83, QStringLiteral("Georgia West (FIPS 1002)")                       },
    { "15131", ZoneInfo::SPCS83, QStringLiteral("Hawaii 1 (FIPS 5101)")                           },
    { "15132", ZoneInfo::SPCS83, QStringLiteral("Hawaii 2 (FIPS 5102)")                           },
    { "15133", ZoneInfo::SPCS83, QStringLiteral("Hawaii 3 (FIPS 5103)")                           },
    { "15134", ZoneInfo::SPCS83, QStringLiteral("Hawaii 4 (FIPS 5104)")                           },
    { "15135", ZoneInfo::SPCS83, QStringLiteral("Hawaii 5 (FIPS 5105)")                           },
    { "11131", ZoneInfo::SPCS83, QStringLiteral("Idaho East (FIPS 1101)")                         },
    { "11132", ZoneInfo::SPCS83, QStringLiteral("Idaho Central (FIPS 1102)")                      },
    { "11133", ZoneInfo::SPCS83, QStringLiteral("Idaho West (FIPS 1103)")                         },
    { "11231", ZoneInfo::SPCS83, QStringLiteral("Illinois East (FIPS 1201)")                      },
    { "11232", ZoneInfo::SPCS83, QStringLiteral("Illinois West (FIPS 1202)")                      },
    { "11331", ZoneInfo::SPCS83, QStringLiteral("Indiana East (FIPS 1301)")                       },
    { "11332", ZoneInfo::SPCS83, QStringLiteral("Indiana West (FIPS 1302)")                       },
    { "11431", ZoneInfo::SPCS83, QStringLiteral("Iowa North (FIPS 1401)")                         },
    { "11432", ZoneInfo::SPCS83, QStringLiteral("Iowa South (FIPS 1402)")                         },
    { "11531", ZoneInfo::SPCS83, QStringLiteral("Kansas North (FIPS 1501)")                       },
    { "11532", ZoneInfo::SPCS83, QStringLiteral("Kansas South (FIPS 1502)")                       },
    { "11630", ZoneInfo::SPCS83, QStringLiteral("Kentucky One (FIPS 1600)")                       },
    { "15303", ZoneInfo::SPCS83, QStringLiteral("Kentucky North (FIPS 1601)")                     },
    { "11632", ZoneInfo::SPCS83, QStringLiteral("Kentucky South (FIPS 1602)")                     },
    { "11731", ZoneInfo::SPCS83, QStringLiteral("Louisiana North (FIPS 1701)")                    },
    { "11732", ZoneInfo::SPCS83, QStringLiteral("Louisiana South (FIPS 1702)")                    },
    { "11733", ZoneInfo::SPCS83, QStringLiteral("Louisiana Offshore (FIPS 1703)")                 },
    { "11831", ZoneInfo::SPCS83, QStringLiteral("Maine East (FIPS 1801)")                         },
    { "11832", ZoneInfo::SPCS83, QStringLiteral("Maine West (FIPS 1802)")                         },
    { "11930", ZoneInfo::SPCS83, QStringLiteral("Maryland (FIPS 1900)")                           },
    { "12031", ZoneInfo::SPCS83, QStringLiteral("Massachusetts Mainland (FIPS 2001)")             },
    { "12032", ZoneInfo::SPCS83, QStringLiteral("Massachusetts Island (FIPS 2002)")               },
    { "12141", ZoneInfo::SPCS83, QStringLiteral("Michigan North (FIPS 2111)")                     },
    { "12142", ZoneInfo::SPCS83, QStringLiteral("Michigan Central (FIPS 2112)")                   },
    { "12143", ZoneInfo::SPCS83, QStringLiteral("Michigan South (FIPS 2113)")                     },
    { "12231", ZoneInfo::SPCS83, QStringLiteral("Minnesota North (FIPS 2201)")                    },
    { "12232", ZoneInfo::SPCS83, QStringLiteral("Minnesota Central (FIPS 2202)")                  },
    { "12233", ZoneInfo::SPCS83, QStringLiteral("Minnesota South (FIPS 2203)")                    },
    { "12331", ZoneInfo::SPCS83, QStringLiteral("Mississippi East (FIPS 2301)")                   },
    { "12332", ZoneInfo::SPCS83, QStringLiteral("Mississippi West (FIPS 2302)")                   },
    { "12431", ZoneInfo::SPCS83, QStringLiteral("Missouri East (FIPS 2401)")                      },
    { "12432", ZoneInfo::SPCS83, QStringLiteral("Missouri Central (FIPS 2402)")                   },
    { "12433", ZoneInfo::SPCS83, QStringLiteral("Missouri West (FIPS 2403)")                      },
    { "12530", ZoneInfo::SPCS83, QStringLiteral("Montana  (FIPS 2500)")                           },
    { "12630", ZoneInfo::SPCS83, QStringLiteral("Nebraska  (FIPS 2600)")                          },
    { "12731", ZoneInfo::SPCS83, QStringLiteral("Nevada East (FIPS 2701)")                        },
    { "12732", ZoneInfo::SPCS83, QStringLiteral("Nevada Central (FIPS 2702)")                     },
    { "12733", ZoneInfo::SPCS83, QStringLiteral("Nevada West (FIPS 2703)")                        },
    { "12830", ZoneInfo::SPCS83, QStringLiteral("New Hampshire (FIPS 2800)")                      },
    { "12930", ZoneInfo::SPCS83, QStringLiteral("New Jersey (FIPS 2900)")                         },
    { "13031", ZoneInfo::SPCS83, QStringLiteral("New Mexico East (FIPS 3001)")                    },
    { "13032", ZoneInfo::SPCS83, QStringLiteral("New Mexico Central (FIPS 3002)")                 },
    { "13033", ZoneInfo::SPCS83, QStringLiteral("New Mexico West (FIPS 3003)")                    },
    { "13131", ZoneInfo::SPCS83, QStringLiteral("New York East (FIPS 3101)")                      },
    { "13132", ZoneInfo::SPCS83, QStringLiteral("New York Central (FIPS 3102)")                   },
    { "13133", ZoneInfo::SPCS83, QStringLiteral("New York West (FIPS 3103)")                      },
    { "13134", ZoneInfo::SPCS83, QStringLiteral("New York Long Island (FIPS 3104)")               },
    { "13230", ZoneInfo::SPCS83, QStringLiteral("North Carolina (FIPS 3200)")                     },
    { "13331", ZoneInfo::SPCS83, QStringLiteral("North Dakota North (FIPS 3301)")                 },
    { "13332", ZoneInfo::SPCS83, QStringLiteral("North Dakota South (FIPS 3302)")                 },
    { "13431", ZoneInfo::SPCS83, QStringLiteral("Ohio North (FIPS 3401)")                         },
    { "13432", ZoneInfo::SPCS83, QStringLiteral("Ohio South (FIPS 3402)")                         },
    { "13531", ZoneInfo::SPCS83, QStringLiteral("Oklahoma North (FIPS 3501)")                     },
    { "13532", ZoneInfo::SPCS83, QStringLiteral("Oklahoma South (FIPS 3502)")                     },
    { "13631", ZoneInfo::SPCS83, QStringLiteral("Oregon North (FIPS 3601)")                       },
    { "13632", ZoneInfo::SPCS83, QStringLiteral("Oregon South (FIPS 3602)")                       },
    { "13731", ZoneInfo::SPCS83, QStringLiteral("Pennsylvania North (FIPS 3701)")                 },
    { "13732", ZoneInfo::SPCS83, QStringLiteral("Pennsylvania South (FIPS 3702)")                 },
    { "13830", ZoneInfo::SPCS83, QStringLiteral("Rhode Island  (FIPS 3800)")                      },
    { "13930", ZoneInfo::SPCS83, QStringLiteral("South Carolina (FIPS 3900)")                     },
    { "14031", ZoneInfo::SPCS83, QStringLiteral("South Dakota North (FIPS 4001)")                 },
    { "14032", ZoneInfo::SPCS83, QStringLiteral("South Dakota South (FIPS 4002)")                 },
    { "14130", ZoneInfo::SPCS83, QStringLiteral("Tennessee (FIPS 4100)")                          },
    { "14231", ZoneInfo::SPCS83, QStringLiteral("Texas North (FIPS 4201)")                        },
    { "14232", ZoneInfo::SPCS83, QStringLiteral("Texas North Central (FIPS 4202)")                },
    { "14233", ZoneInfo::SPCS83, QStringLiteral("Texas Central (FIPS 4203)")                      },
    { "14234", ZoneInfo::SPCS83, QStringLiteral("Texas South Central (FIPS 4204)")                },
    { "14235", ZoneInfo::SPCS83, QStringLiteral("Texas South (FIPS 4205)")                        },
    { "14331", ZoneInfo::SPCS83, QStringLiteral("Utah North (FIPS 4301)")                         },
    { "14332", ZoneInfo::SPCS83, QStringLiteral("Utah Central (FIPS 4302)")                       },
    { "14333", ZoneInfo::SPCS83, QStringLiteral("Utah South (FIPS 4303)")                         },
    { "14430", ZoneInfo::SPCS83, QStringLiteral("Vermont (FIPS 4400)")                            },
    { "14531", ZoneInfo::SPCS83, QStringLiteral("Virginia North (FIPS 4501)")                     },
    { "14532", ZoneInfo::SPCS83, QStringLiteral("Virginia South (FIPS 4502)")                     },
    { "14631", ZoneInfo::SPCS83, QStringLiteral("Washington North (FIPS 4601)")                   },
    { "14632", ZoneInfo::SPCS83, QStringLiteral("Washington South (FIPS 4602)")                   },
    { "14731", ZoneInfo::SPCS83, QStringLiteral("West Virginia North (FIPS 4701)")                },
    { "14732", ZoneInfo::SPCS83, QStringLiteral("West Virginia South (FIPS 4702)")                },
    { "14831", ZoneInfo::SPCS83, QStringLiteral("Wisconsin North (FIPS 4801)")                    },
    { "14832", ZoneInfo::SPCS83, QStringLiteral("Wisconsin Central (FIPS 4802)")                  },
    { "14833", ZoneInfo::SPCS83, QStringLiteral("Wisconsin South (FIPS 4803)")                    },
    { "14931", ZoneInfo::SPCS83, QStringLiteral("Wyoming East (FIPS 4901)")                       },
    { "14932", ZoneInfo::SPCS83, QStringLiteral("Wyoming East Central (FIPS 4902)")               },
    { "14933", ZoneInfo::SPCS83, QStringLiteral("Wyoming West Central (FIPS 4903)")               },
    { "14934", ZoneInfo::SPCS83, QStringLiteral("Wyoming West (FIPS 4904)")                       },
    { "16001", ZoneInfo::UTM,    QStringLiteral("UTM Zone 1 North (180\u00b0W\u2013174\u00b0W)")  },
    { "16002", ZoneInfo::UTM,    QStringLiteral("UTM Zone 2 North (174\u00b0W\u2013168\u00b0W)")  },
    { "16003", ZoneInfo::UTM,    QStringLiteral("UTM Zone 3 North (168\u00b0W\u2013162\u00b0W)")  },
    { "16004", ZoneInfo::UTM,    QStringLiteral("UTM Zone 4 North (162\u00b0W\u2013156\u00b0W)")  },
    { "16005", ZoneInfo::UTM,    QStringLiteral("UTM Zone 5 North (156\u00b0W\u2013150\u00b0W)")  },
    { "16006", ZoneInfo::UTM,    QStringLiteral("UTM Zone 6 North (150\u00b0W\u2013144\u00b0W)")  },
    { "16007", ZoneInfo::UTM,    QStringLiteral("UTM Zone 7 North (144\u00b0W\u2013138\u00b0W)")  },
    { "16008", ZoneInfo::UTM,    QStringLiteral("UTM Zone 8 North (138\u00b0W\u2013132\u00b0W)")  },
    { "16009", ZoneInfo::UTM,    QStringLiteral("UTM Zone 9 North (132\u00b0W\u2013126\u00b0W)")  },
    { "16010", ZoneInfo::UTM,    QStringLiteral("UTM Zone 10 North (126\u00b0W\u2013120\u00b0W)") },
    { "16011", ZoneInfo::UTM,    QStringLiteral("UTM Zone 11 North (120\u00b0W\u2013114\u00b0W)") },
    { "16012", ZoneInfo::UTM,    QStringLiteral("UTM Zone 12 North (114\u00b0W\u2013108\u00b0W)") },
    { "16013", ZoneInfo::UTM,    QStringLiteral("UTM Zone 13 North (108\u00b0W\u2013102\u00b0W)") },
    { "16014", ZoneInfo::UTM,    QStringLiteral("UTM Zone 14 North (102\u00b0W\u201396\u00b0W)")  },
    { "16015", ZoneInfo::UTM,    QStringLiteral("UTM Zone 15 North (96\u00b0W\u201390\u00b0W)")   },
    { "16016", ZoneInfo::UTM,    QStringLiteral("UTM Zone 16 North (90\u00b0W\u201384\u00b0W)")   },
    { "16017", ZoneInfo::UTM,    QStringLiteral("UTM Zone 17 North (84\u00b0W\u201378\u00b0W)")   },
    { "16018", ZoneInfo::UTM,    QStringLiteral("UTM Zone 18 North (78\u00b0W\u201372\u00b0W)")   },
    { "16019", ZoneInfo::UTM,    QStringLiteral("UTM Zone 19 North (72\u00b0W\u201366\u00b0W)")   },
    { "16020", ZoneInfo::UTM,    QStringLiteral("UTM Zone 20 North (66\u00b0W\u201360\u00b0W)")   },
    { "16021", ZoneInfo::UTM,    QStringLiteral("UTM Zone 21 North (60\u00b0W\u201354\u00b0W)")   },
    { "16022", ZoneInfo::UTM,    QStringLiteral("UTM Zone 22 North (54\u00b0W\u201348\u00b0W)")   },
    { "16023", ZoneInfo::UTM,    QStringLiteral("UTM Zone 23 North (48\u00b0W\u201342\u00b0W)")   },
    { "16024", ZoneInfo::UTM,    QStringLiteral("UTM Zone 24 North (42\u00b0W\u201336\u00b0W)")   },
    { "16025", ZoneInfo::UTM,    QStringLiteral("UTM Zone 25 North (36\u00b0W\u201330\u00b0W)")   },
    { "16026", ZoneInfo::UTM,    QStringLiteral("UTM Zone 26 North (30\u00b0W\u201324\u00b0W)")   },
    { "16027", ZoneInfo::UTM,    QStringLiteral("UTM Zone 27 North (24\u00b0W\u201318\u00b0W)")   },
    { "16028", ZoneInfo::UTM,    QStringLiteral("UTM Zone 28 North (18\u00b0W\u201312\u00b0W)")   },
    { "16029", ZoneInfo::UTM,    QStringLiteral("UTM Zone 29 North (12\u00b0W\u20136\u00b0W)")    },
    { "16030", ZoneInfo::UTM,    QStringLiteral("UTM Zone 30 North (6\u00b0W\u20130\u00b0W)")     },
    { "16031", ZoneInfo::UTM,    QStringLiteral("UTM Zone 31 North (0\u00b0E\u20136\u00b0E)")     },
    { "16032", ZoneInfo::UTM,    QStringLiteral("UTM Zone 32 North (6\u00b0E\u201312\u00b0E)")    },
    { "16033", ZoneInfo::UTM,    QStringLiteral("UTM Zone 33 North (12\u00b0E\u201318\u00b0E)")   },
    { "16034", ZoneInfo::UTM,    QStringLiteral("UTM Zone 34 North (18\u00b0E\u201324\u00b0E)")   },
    { "16035", ZoneInfo::UTM,    QStringLiteral("UTM Zone 35 North (24\u00b0E\u201330\u00b0E)")   },
    { "16036", ZoneInfo::UTM,    QStringLiteral("UTM Zone 36 North (30\u00b0E\u201336\u00b0E)")   },
    { "16037", ZoneInfo::UTM,    QStringLiteral("UTM Zone 37 North (36\u00b0E\u201342\u00b0E)")   },
    { "16038", ZoneInfo::UTM,    QStringLiteral("UTM Zone 38 North (42\u00b0E\u201348\u00b0E)")   },
    { "16039", ZoneInfo::UTM,    QStringLiteral("UTM Zone 39 North (48\u00b0E\u201354\u00b0E)")   },
    { "16040", ZoneInfo::UTM,    QStringLiteral("UTM Zone 40 North (54\u00b0E\u201360\u00b0E)")   },
    { "16041", ZoneInfo::UTM,    QStringLiteral("UTM Zone 41 North (60\u00b0E\u201366\u00b0E)")   },
    { "16042", ZoneInfo::UTM,    QStringLiteral("UTM Zone 42 North (66\u00b0E\u201372\u00b0E)")   },
    { "16043", ZoneInfo::UTM,    QStringLiteral("UTM Zone 43 North (72\u00b0E\u201378\u00b0E)")   },
    { "16044", ZoneInfo::UTM,    QStringLiteral("UTM Zone 44 North (78\u00b0E\u201384\u00b0E)")   },
    { "16045", ZoneInfo::UTM,    QStringLiteral("UTM Zone 45 North (84\u00b0E\u201390\u00b0E)")   },
    { "16046", ZoneInfo::UTM,    QStringLiteral("UTM Zone 46 North (90\u00b0E\u201396\u00b0E)")   },
    { "16047", ZoneInfo::UTM,    QStringLiteral("UTM Zone 47 North (96\u00b0E\u2013102\u00b0E)")  },
    { "16048", ZoneInfo::UTM,    QStringLiteral("UTM Zone 48 North (102\u00b0E\u2013108\u00b0E)") },
    { "16049", ZoneInfo::UTM,    QStringLiteral("UTM Zone 49 North (108\u00b0E\u2013114\u00b0E)") },
    { "16050", ZoneInfo::UTM,    QStringLiteral("UTM Zone 50 North (114\u00b0E\u2013120\u00b0E)") },
    { "16051", ZoneInfo::UTM,    QStringLiteral("UTM Zone 51 North (120\u00b0E\u2013126\u00b0E)") },
    { "16052", ZoneInfo::UTM,    QStringLiteral("UTM Zone 52 North (126\u00b0E\u2013132\u00b0E)") },
    { "16053", ZoneInfo::UTM,    QStringLiteral("UTM Zone 53 North (132\u00b0E\u2013138\u00b0E)") },
    { "16054", ZoneInfo::UTM,    QStringLiteral("UTM Zone 54 North (138\u00b0E\u2013144\u00b0E)") },
    { "16055", ZoneInfo::UTM,    QStringLiteral("UTM Zone 55 North (144\u00b0E\u2013150\u00b0E)") },
    { "16056", ZoneInfo::UTM,    QStringLiteral("UTM Zone 56 North (150\u00b0E\u2013156\u00b0E)") },
    { "16057", ZoneInfo::UTM,    QStringLiteral("UTM Zone 57 North (156\u00b0E\u2013162\u00b0E)") },
    { "16058", ZoneInfo::UTM,    QStringLiteral("UTM Zone 58 North (162\u00b0E\u2013168\u00b0E)") },
    { "16059", ZoneInfo::UTM,    QStringLiteral("UTM Zone 59 North (168\u00b0E\u2013174\u00b0E)") },
    { "16060", ZoneInfo::UTM,    QStringLiteral("UTM Zone 60 North (174\u00b0E\u2013180\u00b0E)") },
    { "16101", ZoneInfo::UTM,    QStringLiteral("UTM Zone 1 South (180\u00b0W\u2013174\u00b0W)")  },
    { "16102", ZoneInfo::UTM,    QStringLiteral("UTM Zone 2 South (174\u00b0W\u2013168\u00b0W)")  },
    { "16103", ZoneInfo::UTM,    QStringLiteral("UTM Zone 3 South (168\u00b0W\u2013162\u00b0W)")  },
    { "16104", ZoneInfo::UTM,    QStringLiteral("UTM Zone 4 South (162\u00b0W\u2013156\u00b0W)")  },
    { "16105", ZoneInfo::UTM,    QStringLiteral("UTM Zone 5 South (156\u00b0W\u2013150\u00b0W)")  },
    { "16106", ZoneInfo::UTM,    QStringLiteral("UTM Zone 6 South (150\u00b0W\u2013144\u00b0W)")  },
    { "16107", ZoneInfo::UTM,    QStringLiteral("UTM Zone 7 South (144\u00b0W\u2013138\u00b0W)")  },
    { "16108", ZoneInfo::UTM,    QStringLiteral("UTM Zone 8 South (138\u00b0W\u2013132\u00b0W)")  },
    { "16109", ZoneInfo::UTM,    QStringLiteral("UTM Zone 9 South (132\u00b0W\u2013126\u00b0W)")  },
    { "16110", ZoneInfo::UTM,    QStringLiteral("UTM Zone 10 South (126\u00b0W\u2013120\u00b0W)") },
    { "16111", ZoneInfo::UTM,    QStringLiteral("UTM Zone 11 South (120\u00b0W\u2013114\u00b0W)") },
    { "16112", ZoneInfo::UTM,    QStringLiteral("UTM Zone 12 South (114\u00b0W\u2013108\u00b0W)") },
    { "16113", ZoneInfo::UTM,    QStringLiteral("UTM Zone 13 South (108\u00b0W\u2013102\u00b0W)") },
    { "16114", ZoneInfo::UTM,    QStringLiteral("UTM Zone 14 South (102\u00b0W\u201396\u00b0W)")  },
    { "16115", ZoneInfo::UTM,    QStringLiteral("UTM Zone 15 South (96\u00b0W\u201390\u00b0W)")   },
    { "16116", ZoneInfo::UTM,    QStringLiteral("UTM Zone 16 South (90\u00b0W\u201384\u00b0W)")   },
    { "16117", ZoneInfo::UTM,    QStringLiteral("UTM Zone 17 South (84\u00b0W\u201378\u00b0W)")   },
    { "16118", ZoneInfo::UTM,    QStringLiteral("UTM Zone 18 South (78\u00b0W\u201372\u00b0W)")   },
    { "16119", ZoneInfo::UTM,    QStringLiteral("UTM Zone 19 South (72\u00b0W\u201366\u00b0W)")   },
    { "16120", ZoneInfo::UTM,    QStringLiteral("UTM Zone 20 South (66\u00b0W\u201360\u00b0W)")   },
    { "16121", ZoneInfo::UTM,    QStringLiteral("UTM Zone 21 South (60\u00b0W\u201354\u00b0W)")   },
    { "16122", ZoneInfo::UTM,    QStringLiteral("UTM Zone 22 South (54\u00b0W\u201348\u00b0W)")   },
    { "16123", ZoneInfo::UTM,    QStringLiteral("UTM Zone 23 South (48\u00b0W\u201342\u00b0W)")   },
    { "16124", ZoneInfo::UTM,    QStringLiteral("UTM Zone 24 South (42\u00b0W\u201336\u00b0W)")   },
    { "16125", ZoneInfo::UTM,    QStringLiteral("UTM Zone 25 South (36\u00b0W\u201330\u00b0W)")   },
    { "16126", ZoneInfo::UTM,    QStringLiteral("UTM Zone 26 South (30\u00b0W\u201324\u00b0W)")   },
    { "16127", ZoneInfo::UTM,    QStringLiteral("UTM Zone 27 South (24\u00b0W\u201318\u00b0W)")   },
    { "16128", ZoneInfo::UTM,    QStringLiteral("UTM Zone 28 South (18\u00b0W\u201312\u00b0W)")   },
    { "16129", ZoneInfo::UTM,    QStringLiteral("UTM Zone 29 South (12\u00b0W\u20136\u00b0W)")    },
    { "16130", ZoneInfo::UTM,    QStringLiteral("UTM Zone 30 South (6\u00b0W\u20130\u00b0W)")     },
    { "16131", ZoneInfo::UTM,    QStringLiteral("UTM Zone 31 South (0\u00b0E\u20136\u00b0E)")     },
    { "16132", ZoneInfo::UTM,    QStringLiteral("UTM Zone 32 South (6\u00b0E\u201312\u00b0E)")    },
    { "16133", ZoneInfo::UTM,    QStringLiteral("UTM Zone 33 South (12\u00b0E\u201318\u00b0E)")   },
    { "16134", ZoneInfo::UTM,    QStringLiteral("UTM Zone 34 South (18\u00b0E\u201324\u00b0E)")   },
    { "16135", ZoneInfo::UTM,    QStringLiteral("UTM Zone 35 South (24\u00b0E\u201330\u00b0E)")   },
    { "16136", ZoneInfo::UTM,    QStringLiteral("UTM Zone 36 South (30\u00b0E\u201336\u00b0E)")   },
    { "16137", ZoneInfo::UTM,    QStringLiteral("UTM Zone 37 South (36\u00b0E\u201342\u00b0E)")   },
    { "16138", ZoneInfo::UTM,    QStringLiteral("UTM Zone 38 South (42\u00b0E\u201348\u00b0E)")   },
    { "16139", ZoneInfo::UTM,    QStringLiteral("UTM Zone 39 South (48\u00b0E\u201354\u00b0E)")   },
    { "16140", ZoneInfo::UTM,    QStringLiteral("UTM Zone 40 South (54\u00b0E\u201360\u00b0E)")   },
    { "16141", ZoneInfo::UTM,    QStringLiteral("UTM Zone 41 South (60\u00b0E\u201366\u00b0E)")   },
    { "16142", ZoneInfo::UTM,    QStringLiteral("UTM Zone 42 South (66\u00b0E\u201372\u00b0E)")   },
    { "16143", ZoneInfo::UTM,    QStringLiteral("UTM Zone 43 South (72\u00b0E\u201378\u00b0E)")   },
    { "16144", ZoneInfo::UTM,    QStringLiteral("UTM Zone 44 South (78\u00b0E\u201384\u00b0E)")   },
    { "16145", ZoneInfo::UTM,    QStringLiteral("UTM Zone 45 South (84\u00b0E\u201390\u00b0E)")   },
    { "16146", ZoneInfo::UTM,    QStringLiteral("UTM Zone 46 South (90\u00b0E\u201396\u00b0E)")   },
    { "16147", ZoneInfo::UTM,    QStringLiteral("UTM Zone 47 South (96\u00b0E\u2013102\u00b0E)")  },
    { "16148", ZoneInfo::UTM,    QStringLiteral("UTM Zone 48 South (102\u00b0E\u2013108\u00b0E)") },
    { "16149", ZoneInfo::UTM,    QStringLiteral("UTM Zone 49 South (108\u00b0E\u2013114\u00b0E)") },
    { "16150", ZoneInfo::UTM,    QStringLiteral("UTM Zone 50 South (114\u00b0E\u2013120\u00b0E)") },
    { "16151", ZoneInfo::UTM,    QStringLiteral("UTM Zone 51 South (120\u00b0E\u2013126\u00b0E)") },
    { "16152", ZoneInfo::UTM,    QStringLiteral("UTM Zone 52 South (126\u00b0E\u2013132\u00b0E)") },
    { "16153", ZoneInfo::UTM,    QStringLiteral("UTM Zone 53 South (132\u00b0E\u2013138\u00b0E)") },
    { "16154", ZoneInfo::UTM,    QStringLiteral("UTM Zone 54 South (138\u00b0E\u2013144\u00b0E)") },
    { "16155", ZoneInfo::UTM,    QStringLiteral("UTM Zone 55 South (144\u00b0E\u2013150\u00b0E)") },
    { "16156", ZoneInfo::UTM,    QStringLiteral("UTM Zone 56 South (150\u00b0E\u2013156\u00b0E)") },
    { "16157", ZoneInfo::UTM,    QStringLiteral("UTM Zone 57 South (156\u00b0E\u2013162\u00b0E)") },
    { "16158", ZoneInfo::UTM,    QStringLiteral("UTM Zone 58 South (162\u00b0E\u2013168\u00b0E)") },
    { "16159", ZoneInfo::UTM,    QStringLiteral("UTM Zone 59 South (168\u00b0E\u2013174\u00b0E)") },
    { "16160", ZoneInfo::UTM,    QStringLiteral("UTM Zone 60 South (174\u00b0E\u2013180\u00b0E)") }
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

    void setType(ZoneInfo::ZoneType type)
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
    ZoneInfo::ZoneType currentType_;
    std::vector<ZoneInfo>::const_iterator lower_;
    std::vector<ZoneInfo>::const_iterator upper_;
};

//-----------------------------------------------------------------------------
// ProjectionEditor
//-----------------------------------------------------------------------------

ProjectionEditor::ProjectionEditor(QWidget *parent)
    : QWidget(parent)
{
    cboProjection = new QComboBox;
    cboProjection->addItem("Local", ZoneInfo::Local);
    cboProjection->addItem("State Plane Coordinate System of 1983", ZoneInfo::SPCS83);
    cboProjection->addItem("State Plane Coordinate System of 1927", ZoneInfo::SPCS27);
    cboProjection->addItem("Universal Transverse Mercator", ZoneInfo::UTM);

    zoneModel = new ZoneModel;
    cboZone = new ExtendedComboBox;
    cboZone->setPopupHeight(300);
    cboZone->setModel(zoneModel);

    cboHDatum = new QComboBox;

    cboHUnits = new QComboBox;
    cboHUnits->addItem("Meter",              EPSG_UOM_METER);
    cboHUnits->addItem("International Foot", EPSG_UOM_IFT);
    cboHUnits->addItem("US Survey Foot",     EPSG_UOM_USFT);

    cboVDatum = new QComboBox;

    cboVUnits = new QComboBox;
    cboVUnits->addItem("Meter",              EPSG_UOM_METER);
    cboVUnits->addItem("International Foot", EPSG_UOM_IFT);
    cboVUnits->addItem("US Survey Foot",     EPSG_UOM_USFT);

    // Bounding Box
    leYMax = new ReadOnlyLineEdit;
    leXMin = new ReadOnlyLineEdit;
    leYMin = new ReadOnlyLineEdit;
    leXMax = new ReadOnlyLineEdit;

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

    // Area Layout
    QFormLayout *areaLayoutL = new QFormLayout;
    areaLayoutL->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    areaLayoutL->setSpacing(10);
    areaLayoutL->addRow(tr("West:"), leXMin);

    QFormLayout *areaLayoutC = new QFormLayout;
    areaLayoutC->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    areaLayoutC->setSpacing(10);
    areaLayoutC->addRow(tr("North:"), leYMax);
    areaLayoutC->addRow(tr("South:"), leYMin);

    QFormLayout *areaLayoutR = new QFormLayout;
    areaLayoutR->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    areaLayoutR->setSpacing(10);
    areaLayoutR->addRow(tr("East:"), leXMax);

    QGroupBox *areaGroupBox = new QGroupBox(tr("Valid Area"));
    QHBoxLayout *areaLayout = new QHBoxLayout;
    areaLayout->addLayout(areaLayoutL);
    areaLayout->addLayout(areaLayoutC);
    areaLayout->addLayout(areaLayoutR);
    areaGroupBox->setLayout(areaLayout);

    // Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(hGroupBox);
    mainLayout->addWidget(vGroupBox);
    mainLayout->addWidget(areaGroupBox);
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
            [=](int) { updateProjectionInfo(); });

    connect(cboHDatum, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int) { updateProjectionInfo(); });

    connect(cboHUnits, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int) { updateProjectionInfo(); });

    connect(cboVDatum, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int) { updateProjectionInfo(); });

    connect(cboVUnits, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int) { updateProjectionInfo(); });
}

void ProjectionEditor::updateProjectionInfo()
{
    BOOST_LOG_SCOPED_THREAD_TAG("Source", "Projection");

    textPreview->clear();
    leYMax->clear();
    leXMin->clear();
    leYMin->clear();
    leXMax->clear();

    if (cboProjection->currentIndex() >= 0 &&
        cboZone->currentIndex() >= 0 &&
        cboHDatum->currentIndex() >= 0 &&
        cboHUnits->currentIndex() >= 0 &&
        cboVDatum->currentIndex() >= 0 &&
        cboVUnits->currentIndex() >= 0)
    {
        auto conv = Projection::createConversion(conversionCode());
        auto gcrs = Projection::createGeodeticCRS(hDatumCode());
        auto pcrs = Projection::createProjectedCRS(gcrs, conv, hUnitsCode());
        auto area = Projection::getObjectAreaOfUse(conv);

        std::string text = Projection::exportToPROJString(pcrs);
        textPreview->setPlainText(text);

        leYMax->setText(QLatin1String("%1\x00b0 %2").arg(QString::number(qAbs(area.northLatitude)))
                                                    .arg(area.northLatitude > 0 ? 'N' : 'S'));
        leXMin->setText(QLatin1String("%1\x00b0 %2").arg(QString::number(qAbs(area.westLongitude)))
                                                    .arg(area.westLongitude > 0 ? 'E' : 'W'));
        leYMin->setText(QLatin1String("%1\x00b0 %2").arg(QString::number(qAbs(area.southLatitude)))
                                                    .arg(area.southLatitude > 0 ? 'N' : 'S'));
        leXMax->setText(QLatin1String("%1\x00b0 %2").arg(QString::number(qAbs(area.eastLongitude)))
                                                    .arg(area.eastLongitude > 0 ? 'E' : 'W'));
    }
}

void ProjectionEditor::onProjectionChanged(int index)
{
    QSignalBlocker cboZoneBlocker(cboZone);
    QSignalBlocker cboHDatumBlocker(cboHDatum);
    QSignalBlocker cboVDatumBlocker(cboVDatum);

    cboHDatum->clear();
    cboVDatum->clear();

    switch (index) {
    case 0: // Local
        cboZone->setCurrentIndex(-1);
        cboZone->setEnabled(false);
        cboHDatum->setEnabled(false);
        cboVDatum->setEnabled(false);
        break;
    case 1: // SPCS27
        zoneModel->setType(ZoneInfo::SPCS27);
        cboZone->setCurrentIndex(0);
        cboZone->setEnabled(true);
        cboHDatum->addItem("NAD27",              EPSG_HDATUM_NAD27);
        cboHDatum->setCurrentIndex(0);
        cboHDatum->setEnabled(true);
        cboVDatum->addItem("NAVD88",             EPSG_VDATUM_NAVD88);
        cboVDatum->addItem("NGVD29",             EPSG_VDATUM_NGVD29);
        cboVDatum->setCurrentIndex(1);
        cboVDatum->setEnabled(true);
        break;
    case 2: // SPCS83
        zoneModel->setType(ZoneInfo::SPCS83);
        cboZone->setCurrentIndex(0);
        cboZone->setEnabled(true);
        cboHDatum->addItem("NAD83(2011)",        EPSG_HDATUM_NAD83_2011);
        cboHDatum->addItem("NAD83(NSRS2007)",    EPSG_HDATUM_NAD83_NSRS2007);
        cboHDatum->addItem("NAD83(HARN)",        EPSG_HDATUM_NAD83_HARN);
        cboHDatum->addItem("NAD83(FBN)",         EPSG_HDATUM_NAD83_FBN);
        cboHDatum->addItem("NAD83(1986)",        EPSG_HDATUM_NAD83_1986);
        cboHDatum->setCurrentIndex(0);
        cboHDatum->setEnabled(true);
        cboVDatum->addItem("NAVD88",             EPSG_VDATUM_NAVD88);
        cboVDatum->addItem("NGVD29",             EPSG_VDATUM_NGVD29);
        cboVDatum->setCurrentIndex(0);
        cboVDatum->setEnabled(true);
        break;
    case 3: // UTM
        zoneModel->setType(ZoneInfo::UTM);
        cboZone->setCurrentIndex(0);
        cboZone->setEnabled(true);
        cboHDatum->addItem("WGS84",              EPSG_HDATUM_WGS84);
        cboHDatum->addItem("NAD83(2011)",        EPSG_HDATUM_NAD83_2011);
        cboHDatum->addItem("NAD83(NSRS2007)",    EPSG_HDATUM_NAD83_NSRS2007);
        cboHDatum->addItem("NAD83(HARN)",        EPSG_HDATUM_NAD83_HARN);
        cboHDatum->addItem("NAD83(FBN)",         EPSG_HDATUM_NAD83_FBN);
        cboHDatum->addItem("NAD83(1986)",        EPSG_HDATUM_NAD83_1986);
        cboHDatum->addItem("NAD27",              EPSG_HDATUM_NAD27);
        cboHDatum->setCurrentIndex(0);
        cboHDatum->setEnabled(true);
        cboVDatum->addItem("NAVD88",             EPSG_VDATUM_NAVD88);
        cboVDatum->addItem("NGVD29",             EPSG_VDATUM_NGVD29);
        cboVDatum->addItem("EGM2008",            EPSG_VDATUM_EGM2008);
        cboVDatum->addItem("EGM96",              EPSG_VDATUM_EGM96);
        cboVDatum->addItem("EGM84",              EPSG_VDATUM_EGM84);
        cboVDatum->setCurrentIndex(0);
        cboVDatum->setEnabled(true);
        break;
    }

    updateProjectionInfo();
}
