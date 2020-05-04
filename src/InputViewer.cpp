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

#include "InputViewer.h"
#include "core/Scenario.h"

#include <QApplication>
#include <QScrollBar>
#include <QStringList>
#include <QVBoxLayout>
#include <QtDebug>

#include <boost/algorithm/string.hpp>
#include <boost/spirit/home/x3/string/symbols.hpp>

#include <Scintilla.h>
#include <ScintillaEditBase.h>
#include <SciLexer.h>

#include <sstream>

const uptr_t LINE_MARGIN_INDEX = 0;
const uptr_t FOLD_MARGIN_INDEX = 1;

const sptr_t LINE_MARGIN_WIDTH = 56;
const sptr_t FOLD_MARGIN_WIDTH = 28;

int QColorToCA(const QColor& c)
{
    int r = (c.red() & 0xff);
    int g = (c.green() & 0xff) << 8;
    int b = (c.blue() & 0xff) << 16;
    int ca = r | g | b;
    return ca;
}

InputViewer::InputViewer(Scenario *s, QWidget *parent)
    : QWidget(parent), sPtr(s)
{
    sci = new ScintillaEditBase;
    sci->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sci->setAttribute(Qt::WA_NativeWindow);

    sendMessage(SCI_SETLEXER, SCLEX_CONTAINER);
    sendMessage(SCI_SETMULTIPLESELECTION, 1);
    sendMessage(SCI_SETADDITIONALSELECTIONTYPING, 1);

    setupStyles();
    setupMargins();
    setupConnections();

    sendMessage(SCI_SETREADONLY, true);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(sci);
    setLayout(mainLayout);
}

void InputViewer::setText(const std::string &text)
{
    sendMessage(SCI_SETREADONLY, false);
    sendMessage(SCI_CLEARALL);
    sendMessage(SCI_INSERTTEXT, 0, (sptr_t)text.data());
    sendMessage(SCI_SETREADONLY, true);
    updateFoldState();
}

sptr_t InputViewer::sendMessage(unsigned int message, uintptr_t wParam, intptr_t lParam)
{
    return sci->send(message, wParam, lParam);
}

void InputViewer::defineMarker(int marker, int markerType, int fg, int bg)
{
    sendMessage(SCI_MARKERDEFINE, marker, markerType);
    sendMessage(SCI_MARKERSETFORE, marker, fg);
    sendMessage(SCI_MARKERSETBACK, marker, bg);
}

void InputViewer::setupStyles()
{
    QColor baseColor = QApplication::palette().base().color();
    QColor caretColor = baseColor.lighter(110);
    QColor marginColor = QApplication::palette().window().color();
    QColor markerColor = QApplication::palette().shadow().color();
    QColor textColor = QApplication::palette().windowText().color();
    QColor commentColor = QColor(106, 153, 85);
    QColor keywordColor = QColor(86, 156, 214);

    sendMessage(SCI_STYLESETBACK, STYLE_DEFAULT, QColorToCA(baseColor));

    // Fonts
    sendMessage(SCI_STYLESETFONT, STYLE_DEFAULT, (uptr_t)("Consolas"));
    sendMessage(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    sendMessage(SCI_STYLECLEARALL);

    // Default Document Width
    sendMessage(SCI_SETSCROLLWIDTH, 200, 0);
    sendMessage(SCI_SETSCROLLWIDTHTRACKING, 1, 0);

    // Syntax Highlighting
    sendMessage(SCI_STYLESETFORE, SCE_C_DEFAULT,     QColorToCA(textColor));
    sendMessage(SCI_STYLESETFORE, SCE_C_COMMENT,     QColorToCA(commentColor)); // 0x008000
    sendMessage(SCI_STYLESETFORE, SCE_C_COMMENTLINE, QColorToCA(commentColor)); // 0x008000
    sendMessage(SCI_STYLESETFORE, SCE_C_NUMBER,      QColorToCA(textColor));    // 0x808000
    sendMessage(SCI_STYLESETFORE, SCE_C_STRING,      QColorToCA(textColor));    // 0x800080
    sendMessage(SCI_STYLESETFORE, SCE_C_WORD,        QColorToCA(keywordColor)); // 0xff0000
    sendMessage(SCI_STYLESETBOLD, SCE_C_WORD,        1);
    sendMessage(SCI_STYLESETFORE, SCE_C_WORD2,       QColorToCA(keywordColor)); // 0xff0000

    // Caret Line
    sendMessage(SCI_SETCARETLINEBACK, QColorToCA(caretColor)); // 0xe1ffff
    sendMessage(SCI_SETCARETLINEVISIBLE, 1);

    // Line Numbers
    sendMessage(SCI_STYLESETBACK, STYLE_LINENUMBER, QColorToCA(marginColor)); // 0xf0f0f0
    sendMessage(SCI_STYLESETFORE, STYLE_LINENUMBER, 0xaf912b);

    // Markers
    defineMarker(SC_MARKNUM_FOLDEROPEN,    SC_MARK_BOXMINUS,          0xffffff, QColorToCA(markerColor)); // 0x808080
    defineMarker(SC_MARKNUM_FOLDER,        SC_MARK_BOXPLUS,           0xffffff, QColorToCA(markerColor)); // 0x808080
    defineMarker(SC_MARKNUM_FOLDERSUB,     SC_MARK_VLINE,             0xffffff, QColorToCA(markerColor)); // 0x808080
    defineMarker(SC_MARKNUM_FOLDERTAIL,    SC_MARK_LCORNER,           0xffffff, QColorToCA(markerColor)); // 0x808080
    defineMarker(SC_MARKNUM_FOLDEREND,     SC_MARK_BOXPLUSCONNECTED,  0xffffff, QColorToCA(markerColor)); // 0x808080
    defineMarker(SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED, 0xffffff, QColorToCA(markerColor)); // 0x808080
    defineMarker(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER,           0xffffff, QColorToCA(markerColor)); // 0x808080
}

void InputViewer::setupMargins()
{
    QColor marginColor = QApplication::palette().window().color().lighter(104);

    // Line Number
    sendMessage(SCI_SETMARGINTYPEN,        LINE_MARGIN_INDEX, SC_MARGIN_NUMBER);
    sendMessage(SCI_SETMARGINWIDTHN,       LINE_MARGIN_INDEX, LINE_MARGIN_WIDTH);

    // Folding
    sendMessage(SCI_SETMARGINTYPEN,        FOLD_MARGIN_INDEX, SC_MARGIN_SYMBOL);
    sendMessage(SCI_SETMARGINWIDTHN,       FOLD_MARGIN_INDEX, FOLD_MARGIN_WIDTH);
    sendMessage(SCI_SETMARGINMASKN,        FOLD_MARGIN_INDEX, SC_MASK_FOLDERS);
    sendMessage(SCI_SETMARGINSENSITIVEN,   FOLD_MARGIN_INDEX, 1);
    sendMessage(SCI_SETFOLDMARGINCOLOUR,   FOLD_MARGIN_INDEX, QColorToCA(marginColor)); // 0xfafafa
    sendMessage(SCI_SETFOLDMARGINHICOLOUR, FOLD_MARGIN_INDEX, QColorToCA(marginColor)); // 0xfafafa

    sendMessage(SCI_SETFOLDFLAGS, SC_FOLDFLAG_LINEAFTER_CONTRACTED);

    sendMessage(SCI_SETPROPERTY, (uptr_t)("fold"), (sptr_t)("1"));
    sendMessage(SCI_SETPROPERTY, (uptr_t)("fold.compact"), (sptr_t)("0"));
}

void InputViewer::setupConnections()
{
    connect(sci, &ScintillaEditBase::command, this, &InputViewer::onCommand);
    connect(sci, &ScintillaEditBase::notify, this, &InputViewer::onNotify);
    connect(sci, &ScintillaEditBase::marginClicked, this, &InputViewer::onMarginClicked);
    connect(sci, &ScintillaEditBase::styleNeeded, this, &InputViewer::onStyleNeeded);
}

void InputViewer::updateFoldState()
{
    // Initialize the current fold state.
    int levelPrev = sendMessage(SCI_GETFOLDLEVEL, 0) & SC_FOLDLEVELNUMBERMASK;
    int levelCurrent = levelPrev;

    // Set folding, iterating over all lines in the document.
    int lineCount = sendMessage(SCI_GETLINECOUNT);
    for (std::size_t i = 0; i < lineCount; ++i)
    {
        std::size_t lineLen = static_cast<std::size_t>(sendMessage(SCI_LINELENGTH, i));
        std::string line(lineLen, '\0');
        sendMessage(SCI_GETLINE, i, (uptr_t)line.data());

        auto it = boost::make_split_iterator(line, boost::token_finder(boost::is_space(), boost::token_compress_on));
        auto end = boost::algorithm::split_iterator<std::string::iterator>();
        if (it != end && !boost::starts_with(*it, "**")) {
            std::advance(it, 1);
            if (it != end) {
                auto keyword = *it;
                if (boost::iequals(keyword, "STARTING"))
                    levelCurrent++;
                else if (boost::iequals(keyword, "FINISHED"))
                    levelCurrent--;
            }
        }

        int lev = levelPrev;
        if (levelCurrent > levelPrev)
            lev |= SC_FOLDLEVELHEADERFLAG;
        if (lev != sendMessage(SCI_GETFOLDLEVEL, i))
            sendMessage(SCI_SETFOLDLEVEL, i, lev);

        levelPrev = levelCurrent;
    }
}

void InputViewer::onMarginClicked(int position, int modifiers, int margin)
{
    Q_UNUSED(modifiers)

    const int lineNumber = sendMessage(SCI_LINEFROMPOSITION, position, 0);

    switch (margin) {
    case FOLD_MARGIN_INDEX:
        sendMessage(SCI_TOGGLEFOLD, lineNumber, 0);
        break;
    default:
        break;
    }
}

void InputViewer::onStyleNeeded(int position)
{
    static const boost::spirit::x3::symbols<> pathways{
        "CO", "SO", "RE", "ME", "EV", "OU"
    };

    static const boost::spirit::x3::symbols<> keywords{
        "TITLEONE", "TITLETWO", "MODELOPT", "AVERTIME", "POLLUTID",
        "HALFLIFE", "DCAYCOEF", "DEBUGOPT", "ELEVUNIT", "FLAGPOLE",
        "RUNORNOT", "EVENTFIL", "SAVEFILE", "INITFILE", "MULTYEAR",
        "ERRORFIL", "GASDEPDF", "GDSEASON", "GDLANUSE", "GASDEPVD",
        "URBANOPT", "NO2EQUIL", "NO2STACK", "OZONEVAL", "OZONEFIL",
        "O3VALUES", "OZONUNIT", "O3SECTOR", "LOW_WIND", "ARMRATIO",
        "XBADJ",    "YBADJ",
        "LOCATION", "SRCPARAM", "BUILDHGT", "BUILDWID", "BUILDLEN",
        "EMISFACT", "EMISUNIT", "PARTDIAM", "MASSFRAX", "PARTDENS",
        "METHOD_2", "CONCUNIT", "DEPOUNIT", "GASDEPOS", "HOUREMIS",
        "NO2RATIO", "AREAVERT", "URBANSRC", "SRCGROUP", "OLMGROUP",
        "PSDGROUP", "BACKGRND", "BACKUNIT", "BGSECTOR", "BLPINPUT",
        "BLPGROUP", "BUFRZONE", "INCLUDED", "EVENTPER", "EVENTLOC",
        "GRIDCART", "GRIDPOLR", "DISCCART", "DISCPOLR", "EVALCART",
        "SURFFILE", "PROFFILE", "PROFBASE", "SURFDATA", "UAIRDATA",
        "SITEDATA", "STARTEND", "DAYRANGE", "SCIMBYHR", "WDROTATE",
        "WINDCATS", "NUMYEARS", "RECTABLE", "MAXTABLE", "DAYTABLE",
        "SUMMFILE", "MAXIFILE", "POSTFILE", "PLOTFILE", "TOXXFILE",
        "SEASONHR", "EVENTOUT", "RANKFILE", "EVALFILE", "FILEFORM",
        "MAXDAILY", "MXDYBYYR", "MAXDCONT", "NOHEADER"
    };

    // Get the styling positions.
    std::size_t startPos = static_cast<std::size_t>(sendMessage(SCI_GETENDSTYLED));
    std::size_t endPos = static_cast<std::size_t>(position);

    // Get the start and end line numbers.
    const std::size_t startLineNo = static_cast<std::size_t>(sendMessage(SCI_LINEFROMPOSITION, startPos));
    const std::size_t endLineNo = static_cast<std::size_t>(sendMessage(SCI_LINEFROMPOSITION, endPos));

    // Iterate over the lines.
    for (std::size_t i = startLineNo; i < endLineNo; ++i)
    {
        // Set the start position to beginning of line.
        startPos = static_cast<std::size_t>(sendMessage(SCI_POSITIONFROMLINE, i));
        sendMessage(SCI_STARTSTYLING, startPos, 0x1f);

        // Read the line text into a buffer.
        std::size_t lineLen = static_cast<std::size_t>(sendMessage(SCI_LINELENGTH, i));
        std::string line(lineLen, '\0');
        sendMessage(SCI_GETLINE, i, (uptr_t)line.data());
        boost::to_upper(line);

        // Apply styling to pathway and keyword.
        std::istringstream ss(line);
        std::string token;
        ss >> std::skipws >> token;
        if (boost::starts_with(token, "**")) {
            // Style comments. Must be at start of line.
            sendMessage(SCI_SETSTYLING, lineLen, SCE_C_COMMENTLINE);
        }
        else if (pathways.find(token)) {
            ss >> token;
            std::streamoff sspos = ss.tellg();
            if (boost::iequals(token, "STARTING") ||
                boost::iequals(token, "FINISHED")) {
                sendMessage(SCI_SETSTYLING, sspos, SCE_C_WORD);
                sendMessage(SCI_SETSTYLING, lineLen - sspos, SCE_C_DEFAULT);
            }
            else if (keywords.find(token)) {
                sendMessage(SCI_SETSTYLING, sspos, SCE_C_WORD2);
                sendMessage(SCI_SETSTYLING, lineLen - sspos, SCE_C_DEFAULT);
            }
            else {
                sendMessage(SCI_SETSTYLING, lineLen, SCE_C_DEFAULT);
            }
        }
        else if (keywords.find(token)) {
            std::streamoff sspos = ss.tellg();
            sendMessage(SCI_SETSTYLING, sspos, SCE_C_WORD2);
            sendMessage(SCI_SETSTYLING, lineLen - sspos, SCE_C_DEFAULT);
        }
    }
}

void InputViewer::onNotify(SCNotification *pscn)
{
    Q_UNUSED(pscn)
}

void InputViewer::onCommand(uintptr_t wParam, intptr_t lParam)
{
    Q_UNUSED(wParam)
    Q_UNUSED(lParam)
}

void InputViewer::refresh()
{
    if (sPtr == nullptr)
        return;

    // Capture the current scroll state
    QScrollBar *vsb = sci->verticalScrollBar();
    int vsbval = vsb->value();

    std::string buffer = sPtr->writeInput();
    setText(buffer);

    // Restore scroll state
    vsb->setValue(vsbval);
}
