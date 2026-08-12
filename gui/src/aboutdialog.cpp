#include "aboutdialog.h"

#include "config.h"

#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSysInfo>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace {

QLabel *selectable_value(const QString& text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

} // namespace

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName("aboutDialog");
    setWindowTitle(tr("About %1").arg(PROGRAM_NAME));
    setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
    setModal(true);
    setMinimumWidth(560);
    resize(680, 600);

    auto *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(14);

    auto *header_layout = new QHBoxLayout();
    auto *icon_label = new QLabel(this);
    icon_label->setPixmap(QIcon(":/assets/icon/eeprom_icon.ico").pixmap(72, 72));
    icon_label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    icon_label->setFixedWidth(88);
    header_layout->addWidget(icon_label);

    auto *title_layout = new QVBoxLayout();
    auto *title_label = new QLabel(
        tr("<span style=\"font-size: 18pt; font-weight: bold;\">%1</span><br>"
           "<span style=\"font-size: 11pt;\">Version %2</span>")
            .arg(PROGRAM_NAME, PROGRAM_VERSION),
        this);
    title_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    title_layout->addWidget(title_label);

    auto *description_label = new QLabel(
        tr("Read, program, erase, and verify SST39SF0x0 flash-memory devices "
           "using the Raspberry Pi Pico-based programmer."),
        this);
    description_label->setWordWrap(true);
    title_layout->addWidget(description_label);
    header_layout->addLayout(title_layout, 1);
    main_layout->addLayout(header_layout);

    auto *links_label = new QLabel(
        tr("<a href=\"https://github.com/ifilot/pico-sst39sf0x0-programmer\">Project website</a>"
           " &nbsp;&middot;&nbsp; "
           "<a href=\"https://www.philips-p2000t.nl/tools/pico-sst39sf0x0-programmer.html\">Documentation</a>"
           " &nbsp;&middot;&nbsp; "
           "<a href=\"https://github.com/ifilot/pico-sst39sf0x0-programmer/releases\">Releases</a>"
           " &nbsp;&middot;&nbsp; "
           "<a href=\"https://github.com/ifilot/pico-sst39sf0x0-programmer/issues\">Report an issue</a>"),
        this);
    links_label->setAlignment(Qt::AlignHCenter);
    links_label->setOpenExternalLinks(true);
    links_label->setWordWrap(true);
    main_layout->addWidget(links_label);

    auto *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    main_layout->addWidget(separator);

    auto *build_group = new QGroupBox(tr("Build information"), this);
    auto *build_layout = new QFormLayout(build_group);
    build_layout->addRow(tr("Version:"), selectable_value(PROGRAM_VERSION, build_group));
    build_layout->addRow(tr("Git revision:"), selectable_value(GIT_HASH, build_group));
    build_layout->addRow(tr("Built:"), selectable_value(QStringLiteral(__DATE__), build_group));
    build_layout->addRow(tr("Qt:"), selectable_value(QString::fromLatin1(qVersion()), build_group));
    main_layout->addWidget(build_group);

    auto *credits = new QTextBrowser(this);
    credits->setObjectName("aboutCredits");
    credits->setOpenExternalLinks(true);
    credits->setReadOnly(true);
    credits->setMinimumHeight(160);
    credits->setHtml(
        tr("<p><b>Developed by Ivo Filot</b><br>"
           "<a href=\"mailto:ivo@ivofilot.nl\">ivo@ivofilot.nl</a><br>"
           "Copyright &copy; 2023&ndash;2026 Ivo Filot</p>"
           "<p>The GUI and firmware are free software distributed under the "
           "<a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GNU GPL v3</a>. "
           "The hardware designs are distributed under "
           "<a href=\"https://creativecommons.org/licenses/by-sa/4.0/\">CC BY-SA 4.0</a>. "
           "This application uses the "
           "<a href=\"https://www.qt.io/licensing/open-source-lgpl-obligations\">Qt framework</a> "
           "and third-party icons credited in the source distribution.</p>"
           "<p>Open-source hardware certification: "
           "<a href=\"https://certification.oshwa.org/nl000020.html\">NL000020</a>.</p>"));
    main_layout->addWidget(credits);

    auto *button_box = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *copy_button = button_box->addButton(tr("Copy system information"),
                                               QDialogButtonBox::ActionRole);
    copy_button->setObjectName("copySystemInformationButton");
    connect(copy_button, &QPushButton::clicked, this, [this, copy_button]() {
        QApplication::clipboard()->setText(system_information());
        copy_button->setText(tr("Copied"));
    });
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    main_layout->addWidget(button_box);
}

QString AboutDialog::system_information() const
{
    return tr("%1 %2\n"
              "Git revision: %3\n"
              "Build date: %4\n"
              "Qt version: %5\n"
              "Operating system: %6\n"
              "CPU architecture: %7")
        .arg(PROGRAM_NAME,
             PROGRAM_VERSION,
             GIT_HASH,
             QStringLiteral(__DATE__),
             QString::fromLatin1(qVersion()),
             QSysInfo::prettyProductName(),
             QSysInfo::currentCpuArchitecture());
}
