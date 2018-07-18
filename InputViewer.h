#ifndef INPUTVIEWER_H
#define INPUTVIEWER_H

#include "Scenario.h"

#include <Scintilla.h>
#include <ScintillaEditBase.h>

class InputViewer : public QWidget
{
    Q_OBJECT

public:
    explicit InputViewer(Scenario *s, QWidget *parent = nullptr);
    void setText(const std::string& text);

public slots:
    void refresh();

private:
    sptr_t sendMessage(unsigned int iMessage, uptr_t wParam = 0, sptr_t lParam = 0);
    void defineMarker(int marker, int markerType, int fg, int bg);
    void setupStyles();
    void setupMargins();
    void setupConnections();

private slots:
    void onMarginClicked(int position, int modifiers, int margin);
    void onStyleNeeded(int position);
    void onNotify(SCNotification *pscn);
    void onCommand(uptr_t wParam, sptr_t lParam);

private:
    Scenario *sPtr;
    ScintillaEditBase *sci;
};

#endif // INPUTVIEWER_H
