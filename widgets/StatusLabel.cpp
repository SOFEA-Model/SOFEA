#include "StatusLabel.h"

#include <QFont>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QSize>
#include <QWindow>

#include <algorithm>
#include <vector>

StatusLabel::StatusLabel(QWidget *parent)
    : QWidget(parent)
{
    m_iconLabel = new QLabel;
    m_iconLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    m_iconLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_textLabel = new QLabel;
    m_textLabel->setWordWrap(true);

    // Align the icon with the text baseline.
    QFontMetrics fm = m_textLabel->fontMetrics();
    m_iconLabel->setMinimumSize(fm.ascent(), fm.lineSpacing() + 1);
    m_iconLabel->setContentsMargins(0, 0, 0, fm.descent());

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignLeft | Qt::AlignTop);
    mainLayout->addWidget(m_textLabel, 1, Qt::AlignTop);
    setLayout(mainLayout);
}

void StatusLabel::setSeverity(const int severity)
{
    // Severity corresponds to enum severity_level in boost::log::trivial.

    static const std::vector<QIcon> icons = {
        QIcon(":/images/StatusAnnotations_Information_16xLG.png"),       // 0, trace
        QIcon(":/images/StatusAnnotations_Information_16xLG.png"),       // 1, debug
        QIcon(":/images/StatusAnnotations_Information_16xLG_color.png"), // 2, info
        QIcon(":/images/StatusAnnotations_Warning_16xLG_color.png"),     // 3, warning
        QIcon(":/images/StatusAnnotations_Invalid_16xLG_color.png"),     // 4, error
        QIcon(":/images/StatusAnnotations_Critical_16xLG_color.png")     // 5, fatal
    };
    
    int index = std::clamp(severity, 0, 5);
    const QIcon& icon = icons[index];

    QWindow *window = nullptr;
    if (const QWidget *nativeParent = nativeParentWidget())
        window = nativeParent->windowHandle();

    const QPixmap pixmap = icon.pixmap(window, QSize{16, 16});
    m_iconLabel->setPixmap(pixmap);
}

void StatusLabel::setText(const QString& text)
{
    m_textLabel->setText(text);
}
