#include <QApplication>
#include <QIcon>

#include <sstream>

#include "MonteCarloLineEdit.h"
#include "GenericDistributionDialog.h"

struct NameVisitor : public boost::static_visitor<QString>
{
    QString operator()(const Distribution::UniformReal)       { return QString("UniformReal"); }
    QString operator()(const Distribution::UniformInt)        { return QString("UniformInt"); }
    QString operator()(const Distribution::Binomial)          { return QString("Binomial"); }
    QString operator()(const Distribution::Geometric)         { return QString("Geometric"); }
    QString operator()(const Distribution::NegativeBinomial)  { return QString("NegBinom"); }
    QString operator()(const Distribution::Poisson)           { return QString("Poisson"); }
    QString operator()(const Distribution::Exponential)       { return QString("Exp"); }
    QString operator()(const Distribution::Gamma)             { return QString("Gamma"); }
    QString operator()(const Distribution::Weibull)           { return QString("Weibull"); }
    QString operator()(const Distribution::ExtremeValue)      { return QString("EV"); }
    QString operator()(const Distribution::Beta)              { return QString("Beta"); }
    QString operator()(const Distribution::Laplace)           { return QString("Laplace"); }
    QString operator()(const Distribution::Normal)            { return QString("Norm"); }
    QString operator()(const Distribution::Lognormal)         { return QString("LogNorm"); }
    QString operator()(const Distribution::ChiSquared)        { return QString("ChiSq"); }
    QString operator()(const Distribution::NCChiSquared)      { return QString("NCChiSq"); }
    QString operator()(const Distribution::Cauchy)            { return QString("Cauchy"); }
    QString operator()(const Distribution::FisherF)           { return QString("F"); }
    QString operator()(const Distribution::StudentT)          { return QString("t"); }
    QString operator()(const Distribution::Discrete)          { return QString("Discrete"); }
    QString operator()(const Distribution::PiecewiseConstant) { return QString("PiecewiseConstant"); }
    QString operator()(const Distribution::PiecewiseLinear)   { return QString("PiecewiseLinear"); }
    QString operator()(const Distribution::Triangle)          { return QString("Triangular"); }
    QString operator()(const Distribution::Constant)          { return QString(""); }
};

MonteCarloLineEdit::MonteCarloLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    m_validator = new QDoubleValidator(this);
    m_validator->setNotation(QDoubleValidator::StandardNotation);
    m_previousValue = 0;

    // Default Distribution
    m_distribution = Distribution::Constant();
    m_distributionText = QString("");
    m_distributionSet = false;

    // Actions
    static const QIcon functionIcon = QIcon(":/images/Effects_32x.png");
    m_selectDistributionAct = new QAction(functionIcon, "Define Distribution");
    addAction(m_selectDistributionAct, QLineEdit::TrailingPosition);

    // Connections
    connect(m_selectDistributionAct, &QAction::triggered, this, &MonteCarloLineEdit::onSelectDistribution);
    connect(this, &MonteCarloLineEdit::textEdited, this, &MonteCarloLineEdit::onTextEdited);
    connect(this, &MonteCarloLineEdit::cursorPositionChanged, this, &MonteCarloLineEdit::onCursorPositionChanged);

    init();
}

void MonteCarloLineEdit::init()
{
    clear();
    resetState();

    m_distributionText = getDistributionText();
    setText(m_distributionText);
}

void MonteCarloLineEdit::resetState()
{
    // Reset palette and validator.

    if (m_distributionSet) {
        QPalette palette = this->palette();
        palette.setColor(QPalette::Text, Qt::blue);
        setPalette(palette);
        setValidator(0);

    } else {
        QPalette palette = QApplication::palette(this);
        setPalette(palette);
        setValidator(m_validator);
    }
}

void MonteCarloLineEdit::clearDistribution()
{
    clear();

    m_distribution = Distribution::Constant();
    m_distributionSet = false;
    m_distributionText = QString("");

    resetState();
}

QString MonteCarloLineEdit::getDistributionText() const
{
    NameVisitor nv;
    QString name = boost::apply_visitor(nv, m_distribution);

    // Use ostream operator to get parameter text
    std::ostringstream oss;
    oss << m_distribution;
    QString paramText = QString::fromStdString(oss.str());
    paramText.replace(QString(" "), QString(","));

    QString distText;
    if (name.isEmpty()) {
        bool ok;
        double val = paramText.toDouble(&ok);
        if (ok && (val == 0))
            distText = QString("");
        else
            distText = paramText;
    }
    else {
        distText = name + "(" + paramText + ")";
    }

    return distText;
}

void MonteCarloLineEdit::onSelectDistribution()
{
    GenericDistributionDialog dialog(m_distribution, this);

    if (QDialog::Accepted == dialog.exec())
    {
        QString oldText = m_distributionText;

        m_distribution = dialog.getDistribution();
        m_distributionText = getDistributionText();
        m_distributionSet = true;

        resetState();
        setText(m_distributionText); // resets modified flag to false

        // Check whether the distribution actually changed
        if (m_distributionText.compare(oldText) == 0)
            setModified(false);
        else
            setModified(true);
    }
}

void MonteCarloLineEdit::onCursorPositionChanged(int oldPos, int newPos)
{
    // If the distribution is set, make sure the cursor position can
    // only be at the start or end of the text.

    if (m_distributionSet) {
        if (newPos >= oldPos)
            setCursorPosition(this->displayText().length());
        else
            setCursorPosition(0);
    }
}

void MonteCarloLineEdit::onTextEdited(const QString &text)
{
    // If the distribution is set, ignore changes to the text unless
    // it is cleared manually. If the distribution is not set, set the
    // modified flag only if the value actually changed.

    if (m_distributionSet) {
        setText(m_distributionText); // resets modified flag to false
    }
    else {
        bool ok;
        double val = text.toDouble(&ok);
        if (ok && (m_previousValue == val))
            setModified(false); // value did not change
        else
            setModified(true); // value changed
    }
}

void MonteCarloLineEdit::focusInEvent(QFocusEvent *event)
{
    // If the distribution is set, select all text. We also ensure the
    // cursor can only be at the start or end.

    if (m_distributionSet) {
        selectAll();
    }
    else {
        bool ok;
        double val = text().toDouble(&ok);
        if (ok)
            m_previousValue = val; // Check after textEdited signal is emitted
    }

    QLineEdit::focusInEvent(event);
}

void MonteCarloLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (m_distributionSet) {
        if ((event->key() == Qt::Key_Delete && cursorPosition() == 0) ||
            (event->key() == Qt::Key_Backspace && cursorPosition() > 0)) {
            clearDistribution();
            setModified(true);
        }
    }

    QLineEdit::keyPressEvent(event);
}

// Public

void MonteCarloLineEdit::setMinimum(double min)
{
    m_validator->setBottom(min);
}

void MonteCarloLineEdit::setMaximum(double max)
{
    m_validator->setTop(max);
}

void MonteCarloLineEdit::setDecimals(int decimals)
{
    m_validator->setDecimals(decimals);
}

void MonteCarloLineEdit::setRange(double min, double max)
{
    m_validator->setBottom(min);
    m_validator->setTop(max);
}

GenericDistribution MonteCarloLineEdit::getDistribution() const
{
    GenericDistribution d;

    if (m_distributionSet) {
        d = m_distribution;
        return d;
    }
    else {
        // Use placeholder distribution with single value
        bool ok;
        double value = this->text().toDouble(&ok);
        if (!ok)
            value = 0;
        d = Distribution::Constant(value);
        return d;
    }
}

void MonteCarloLineEdit::setDistribution(const GenericDistribution &d)
{
    m_distribution = d;

    // Check for placeholder distribution
    if (d.type() == typeid(Distribution::Constant))
        m_distributionSet = false;
    else
        m_distributionSet = true;

    init();
}
