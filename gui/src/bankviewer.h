#ifndef BANKVIEWER_H
#define BANKVIEWER_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>
#include <QDebug>
#include <QByteArray>
#include <QCryptographicHash>
#include <QPushButton>
#include <QClipboard>
#include <QGuiApplication>
#include <QHash>

#include "romsizes.h" // contains sizes

class BankViewer : public QDialog
{
    Q_OBJECT

private:
    QByteArray data;

    const QHash<QString,QString> checksum_table = {
        {"2191811aa64f8e7f273ce0f462374728", "Basic NL 1.1"},
        {"526132548429f7c1bc275b07f5eb609d", "Assembler 5.6"},
        {"1bf96263f5456e04f9c47d4b2e98e9b2", "Assembler 5.9"},
        {"9aa06a27142c9a6af5b93c6a2c98016c", "Familiegeheugen 2"},
        {"449951c31f6439f6215e593ce22fb73a", "Familiegeheugen 3"},
        {"26b0c8cfeba85c52723d4cca55198f5a", "Familiegeheugen 4"},
        {"29cbab5494e9ca3389d7e596e678f228", "Flexbase"},
        {"758757a6a1bdc459b89c6023b4be6f30", "Forth 3.0"},
        {"99420a88ab7cf03642c51b61b65e87b9", "Maintenance 1"},
        {"ac38f883bebf919c76596c12493dabfa", "Maintenance 2.2"},
        {"e04ec6879e9c946a1dd8b997c6d78d31", "Tekst 1"},
        {"f8840ebb9d0fe7872c501ff378dba059", "Text2000 2"},
        {"7145d89a3f9047e90484187a9d247a0b", "Text2000 3"},
        {"ac61d66e577b5874d6c4281069ba25f6", "WordProcessor 1"},
        {"c13a29d0409b975abd91547b23301267", "WordProcessor 2"},
        {"ea226775d5d5832c8e9e704fbda56eb2", "Zemon 1.4"},
        {"2c0cb28a502beff4d2b402161d1a37d5", "JWS Basic"},

        {"87efa58eef7aac30d4fdbfecf3f3442c", "Bootstrap BASICNL for DATA cartridge v0.5.1"},
        {"6cf63d15c3991190b2ec7fac4c6e7df5", "Bootstrap BASICNL for SD-CARD cartridge v0.3.0"},
        {"784c7ce4f6479be115dad9d9357a5108", "Cassette to EEPROM Utility v0.5.1"},
        {"55dbbb16230d9bbd078409447719ba79", "Flasher for DATA cartridge v0.5.1"},
        {"3949a2126191e4782f005287681fbb7b", "Flasher for SD-CARD cartridge v0.3.0"},
        {"5ad780d5e50bcf810540994c73a65dc0", "RAM (expansion board) Test v0.4.0"},
    };

public:
    explicit BankViewer(unsigned int nrbanks,
                        const QByteArray& _data,
                        QWidget *parent = nullptr);

private:
    void build_widget(unsigned int nrbanks);

    QLabel* create_data_label(unsigned int bank_id);

private slots:
    void slot_copy_checksum();

signals:
};

#endif // BANKVIEWER_H
