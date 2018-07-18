#include "InputViewer.h"

#include <QScrollBar>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>

#include <SciLexer.h>

const int LINE_MARGIN_INDEX = 0;
const int FOLD_MARGIN_INDEX = 1;

QSet<QString> pathways;
QSet<QString> keywords;

InputViewer::InputViewer(Scenario *s, QWidget *parent)
    : QWidget(parent), sPtr(s)
{
    sci = new ScintillaEditBase;
    sci->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sci->setAttribute(Qt::WA_NativeWindow);

    pathways << "CO" << "SO" << "RE" << "ME" << "EV" << "OU" << "STARTING" << "FINISHED";
    keywords << "TITLEONE" << "TITLETWO" << "MODELOPT" << "AVERTIME" << "POLLUTID" <<
                "HALFLIFE" << "DCAYCOEF" << "DEBUGOPT" << "ELEVUNIT" << "FLAGPOLE" <<
                "RUNORNOT" << "EVENTFIL" << "SAVEFILE" << "INITFILE" << "MULTYEAR" <<
                "ERRORFIL" << "GASDEPDF" << "GDSEASON" << "GDLANUSE" << "GASDEPVD" <<
                "URBANOPT" << "NO2EQUIL" << "NO2STACK" << "OZONEVAL" << "OZONEFIL" <<
                "O3VALUES" << "OZONUNIT" << "O3SECTOR" << "LOW_WIND" << "ARMRATIO" <<
                "XBADJ"    << "YBADJ"    <<
                "LOCATION" << "SRCPARAM" << "BUILDHGT" << "BUILDWID" << "BUILDLEN" <<
                "EMISFACT" << "EMISUNIT" << "PARTDIAM" << "MASSFRAX" << "PARTDENS" <<
                "METHOD_2" << "CONCUNIT" << "DEPOUNIT" << "GASDEPOS" << "HOUREMIS" <<
                "NO2RATIO" << "AREAVERT" << "URBANSRC" << "SRCGROUP" << "OLMGROUP" <<
                "PSDGROUP" << "BACKGRND" << "BACKUNIT" << "BGSECTOR" << "BLPINPUT" <<
                "BLPGROUP" << "BUFRZONE" << "INCLUDED" << "EVENTPER" << "EVENTLOC" <<
                "GRIDCART" << "GRIDPOLR" << "DISCCART" << "DISCPOLR" << "EVALCART" <<
                "SURFFILE" << "PROFFILE" << "PROFBASE" << "SURFDATA" << "UAIRDATA" <<
                "SITEDATA" << "STARTEND" << "DAYRANGE" << "SCIMBYHR" << "WDROTATE" <<
                "WINDCATS" << "NUMYEARS" << "RECTABLE" << "MAXTABLE" << "DAYTABLE" <<
                "SUMMFILE" << "MAXIFILE" << "POSTFILE" << "PLOTFILE" << "TOXXFILE" <<
                "SEASONHR" << "EVENTOUT" << "RANKFILE" << "EVALFILE" << "FILEFORM" <<
                "MAXDAILY" << "MXDYBYYR" << "MAXDCONT" << "NOHEADER";

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
}

sptr_t InputViewer::sendMessage(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
    return sci->send(iMessage, wParam, lParam);
}

void InputViewer::defineMarker(int marker, int markerType, int fg, int bg)
{
    sendMessage(SCI_MARKERDEFINE, marker, markerType);
    sendMessage(SCI_MARKERSETFORE, marker, fg);
    sendMessage(SCI_MARKERSETBACK, marker, bg);
}

void InputViewer::setupStyles()
{
    // Fonts
    sendMessage(SCI_STYLESETFONT, STYLE_DEFAULT, (uptr_t)("Consolas"));
    sendMessage(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    sendMessage(SCI_STYLECLEARALL);

    // Default Document Width
    sendMessage(SCI_SETSCROLLWIDTH, 200, 0);
    sendMessage(SCI_SETSCROLLWIDTHTRACKING, 1, 0);

    // Syntax Highlighting
    sendMessage(SCI_STYLESETFORE, SCE_C_COMMENT,     0x008000);
    sendMessage(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x008000);
    sendMessage(SCI_STYLESETFORE, SCE_C_NUMBER,      0x808000);
    sendMessage(SCI_STYLESETFORE, SCE_C_STRING,      0x800080);
    sendMessage(SCI_STYLESETFORE, SCE_C_WORD,        0xff0000);
    sendMessage(SCI_STYLESETBOLD, SCE_C_WORD,        1);
    sendMessage(SCI_STYLESETFORE, SCE_C_WORD2,       0xff0000);

    // Caret Line
    sendMessage(SCI_SETCARETLINEBACK, 0xe1ffff);
    sendMessage(SCI_SETCARETLINEVISIBLE, 1);

    // Line Numbers
    sendMessage(SCI_STYLESETBACK, STYLE_LINENUMBER, 0xf0f0f0);
    sendMessage(SCI_STYLESETFORE, STYLE_LINENUMBER, 0xaf912b);

    // Markers
    defineMarker(SC_MARKNUM_FOLDEROPEN,    SC_MARK_BOXMINUS,          0xffffff, 0x808080);
    defineMarker(SC_MARKNUM_FOLDER,        SC_MARK_BOXPLUS,           0xffffff, 0x808080);
    defineMarker(SC_MARKNUM_FOLDERSUB,     SC_MARK_VLINE,             0xffffff, 0x808080);
    defineMarker(SC_MARKNUM_FOLDERTAIL,    SC_MARK_LCORNER,           0xffffff, 0x808080);
    defineMarker(SC_MARKNUM_FOLDEREND,     SC_MARK_BOXPLUSCONNECTED,  0xffffff, 0x808080);
    defineMarker(SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED, 0xffffff, 0x808080);
    defineMarker(SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER,           0xffffff, 0x808080);
}

void InputViewer::setupMargins()
{
    // Line Number
    sendMessage(SCI_SETMARGINTYPEN,        LINE_MARGIN_INDEX, SC_MARGIN_NUMBER);
    sendMessage(SCI_SETMARGINWIDTHN,       LINE_MARGIN_INDEX, 56);

    // Folding
    sendMessage(SCI_SETMARGINTYPEN,        FOLD_MARGIN_INDEX, SC_MARGIN_SYMBOL);
    sendMessage(SCI_SETMARGINWIDTHN,       FOLD_MARGIN_INDEX, 28);
    sendMessage(SCI_SETMARGINMASKN,        FOLD_MARGIN_INDEX, SC_MASK_FOLDERS);
    sendMessage(SCI_SETMARGINSENSITIVEN,   FOLD_MARGIN_INDEX, 1);
    sendMessage(SCI_SETFOLDMARGINCOLOUR,   FOLD_MARGIN_INDEX, 0xfafafa);
    sendMessage(SCI_SETFOLDMARGINHICOLOUR, FOLD_MARGIN_INDEX, 0xfafafa);

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

void InputViewer::onMarginClicked(int position, int modifiers, int margin)
{
    Q_UNUSED(modifiers);

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
    // Get the styling positions.
    int startPos = sendMessage(SCI_GETENDSTYLED);
    int endPos = position;

    // Get the start and end line numbers.
    const int startLineNo = sendMessage(SCI_LINEFROMPOSITION, startPos);
    const int endLineNo = sendMessage(SCI_LINEFROMPOSITION, endPos);

    // Set the start position to beginning of line.
    startPos = sendMessage(SCI_POSITIONFROMLINE, startLineNo);

    // Start styling.
    sendMessage(SCI_STARTSTYLING, startPos, 0x1f);

    // Iterate over the lines.
    for (int i = startLineNo; i < endLineNo; ++i)
    {
        // Read the line text into a buffer.
        int lineLen = sendMessage(SCI_LINELENGTH, i);
        QByteArray lineData;
        lineData.resize(lineLen + 1);
        sendMessage(SCI_GETLINE, i, (uptr_t)lineData.data());
        QString lineText = QString(lineData).toUpper();

        // Truncate the string to reported line length.
        lineText.truncate(lineLen);

        if (lineText.left(2) == "**") {
            // Style comments. Must be at start of line.
            sendMessage(SCI_SETSTYLING, lineLen, SCE_C_COMMENTLINE);
        }
        else {
            // Style keywords. Split the line into tokens at word boundaries.
            QStringList tokens = lineText.split(QRegExp("\\b"));
            for (int i = 0; i < tokens.size(); ++i) {
                const QString& token = tokens.at(i);
                if (pathways.contains(token) && i < 4) {
                    sendMessage(SCI_SETSTYLING, token.length(), SCE_C_WORD);
                }
                else if (keywords.contains(token) && i < 4) {
                    sendMessage(SCI_SETSTYLING, token.length(), SCE_C_WORD2);
                }
                else {
                    sendMessage(SCI_SETSTYLING, token.length(), SCE_C_DEFAULT);
                }
            }
        }
    }

    // Initialize the current fold state.
    int levelPrev = sendMessage(SCI_GETFOLDLEVEL, 0) & SC_FOLDLEVELNUMBERMASK;
    int levelCurrent = levelPrev;

    // Set folding, iterating over all lines in the document.
    int lineCount = sendMessage(SCI_GETLINECOUNT);
    for (int i = 0; i < lineCount; ++i)
    {
        int lineLen = sendMessage(SCI_LINELENGTH, i);
        QByteArray lineData;
        lineData.resize(lineLen + 1);
        sendMessage(SCI_GETLINE, i, (uptr_t)lineData.data());
        QString lineText = QString(lineData).toUpper().trimmed();

        if (lineText.left(2) != "**") {
            if (lineText.indexOf("STARTING") == 3)
                levelCurrent++;
            else if (lineText.indexOf("FINISHED") == 3)
                levelCurrent--;
        }

        int lev = levelPrev;
        if (levelCurrent > levelPrev)
            lev |= SC_FOLDLEVELHEADERFLAG;
        if (lev != sendMessage(SCI_GETFOLDLEVEL, i))
            sendMessage(SCI_SETFOLDLEVEL, i, lev);

        levelPrev = levelCurrent;
    }
}

void InputViewer::onNotify(SCNotification *pscn)
{
    Q_UNUSED(pscn);
}

void InputViewer::onCommand(uptr_t wParam, sptr_t lParam)
{
    Q_UNUSED(wParam);
    Q_UNUSED(lParam);
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
