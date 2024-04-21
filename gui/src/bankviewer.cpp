#include "bankviewer.h"

BankViewer::BankViewer(unsigned int nrbanks, const QByteArray& _data, QWidget *parent)
    : QDialog{parent} {

    // set data
    this->data = _data;

    // set icon
    this->setWindowIcon(QIcon(":/assets/icon/eeprom_icon.ico"));
    this->setWindowTitle("Overview banks on EEPROM");

    this->build_widget(nrbanks);
}

void BankViewer::build_widget(unsigned int nrbanks) {
    // build layout
    QVBoxLayout* layout = new QVBoxLayout();
    this->setLayout(layout);
    QString chipname;
    switch(nrbanks) {
        case 8:
            chipname = "SST39SF010";
        break;
        case 16:
            chipname = "SST39SF020";
        break;
        case 32:
            chipname = "SST39SF040";
        break;
        default:
            chipname = "Unknown";
        break;
    }

    layout->addWidget(new QLabel(tr("Overview banks on %1 EEPROM").arg(chipname)));

    QWidget *container = new QWidget(this);
    layout->addWidget(container);

    QHBoxLayout* hlayout = new QHBoxLayout();
    container->setLayout(hlayout);

    for(unsigned int i=0; i<nrbanks / 8; i++) {
        QGroupBox* groupbox = new QGroupBox(tr("ROM slots %1 - %2").arg(i*8).arg((i+1)*8-1));
        hlayout->addWidget(groupbox);
        QGridLayout* grid = new QGridLayout();
        groupbox->setLayout(grid);

        for(unsigned int j=0; j<8; j++) {
            QLabel *bank = new QLabel(tr("Bank %1").arg(i*8+j), this);
            grid->addWidget(bank, j, 0);
            grid->addWidget(this->create_data_label(i*8+j), j, 1);
            QPushButton *button_copy = new QPushButton();
            button_copy->setIcon(QIcon(":/assets/icon/clipboard.png"));
            button_copy->setProperty("slot_id", QVariant(i*8+j));
            button_copy->setToolTip("Copy MD5 checksum to clipboard");
            grid->addWidget(button_copy, j, 2);
            connect(button_copy, SIGNAL(released()), this, SLOT(slot_copy_checksum()));
        }
    }

    // button to close Window and exit screen
    QPushButton *button_ok = new QPushButton("OK", this);
    layout->addWidget(button_ok);
    connect(button_ok, SIGNAL(released()), this, SLOT(accept()));
}

QLabel* BankViewer::create_data_label(unsigned int bank_id) {
    QByteArray subdata = this->data.mid(bank_id * BANKSIZE, BANKSIZE);
    QByteArray hash = QCryptographicHash::hash(subdata, QCryptographicHash::Md5);
    QString checksum = QString(hash.toHex());
    QString title = "Unknown";
    QString stylesheet;

    auto itr = this->checksum_table.find(checksum);
    if(itr != this->checksum_table.end()) {
        title = itr.value();
    }

    QString text;
    if(subdata.count(0xFF) == BANKSIZE) {
        stylesheet = "color: #098056;";
        text = tr("Free<br>(empty 16kb x 0xFF)");
    } else if(subdata.count(0x00) == BANKSIZE) {
        stylesheet = "color: #098056;";
        text = tr("Free<br>(empty 16kb x 0x00)");
    } else {
        stylesheet = "color: #b0216d;";
        text = tr("<b>%1</b><br><span style='color: black;'>%2<span>").arg(title).arg(checksum);
    }

    QLabel *res = new QLabel(text);
    //res->setTextFormat(Qt::TextFormat::RichText);
    res->setStyleSheet(stylesheet);
    return res;
}

void BankViewer::slot_copy_checksum() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    unsigned int slot_id = button->property("slot_id").toUInt();
    QByteArray subdata = this->data.mid(slot_id * BANKSIZE, BANKSIZE);
    QByteArray hash = QCryptographicHash::hash(subdata, QCryptographicHash::Md5);
    QString checksum = QString(hash.toHex());
    QGuiApplication::clipboard()->setText(checksum);
}
