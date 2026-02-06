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
        {"02ffba4d22826a47e9f83a48057c0c15", "Bootstrap BASICNL for SD-CARD cartridge v1.3.0"},
        {"55dbbb16230d9bbd078409447719ba79", "Flasher for DATA cartridge v0.5.1"},
        {"784c7ce4f6479be115dad9d9357a5108", "Cassette to EEPROM Utility v0.5.1"},
        {"3949a2126191e4782f005287681fbb7b", "Flasher for SD-CARD cartridge v0.3.0"},
        {"eb8810c685f14c6b69fa26fcff1a0f1b", "Flasher for SD-CARD cartridge v1.3.0"},
        {"5ad780d5e50bcf810540994c73a65dc0", "RAM (expansion board) Test v0.4.0"},
        {"6549c34aa7f46d831f9c1ac8ca2c15bd", "RAM (expansion board) Test v0.7.0"},

        // ===== New hashes (6/2/2026) =====
        {"5191eb8ed909c44acdb0c5672d4fced1", "BASIC EN 1.0"},
        {"b57e185b526d0b97fbc07a7e95fdbde3", "Basic NL 1.0"},
        {"1b55185494ca8b356cbad223ed305a37", "Basic NL 1.1 A2"},
        {"17366eec37f0b96e718962cd903ab880", "Buildschirmtext DE"},
        {"075e0e28d9e3e2d9855914355d8ea9a2", "BIS system"},
        {"ceeee0ada3a3d88adc9db635e68fadd1", "BIS"},
        {"38c0112723820a2b73b7652d4d15c010", "Basic-24 system tracks"},
        {"7e2fbc0342181d9dcd6d808cb4228b18", "Basic-24"},
        {"b1147f6bbc62fa1f9dcab1e6f9cdeece", "CPM Nater"},
        {"143b0d7e5f54ec7aa1c35f1e6ec8a4c1", "ECOL 3.0"},
        {"fd696d4019605029940a7f71475e5173", "EPROM programmer"},
        {"886d05e6686c9775ee7261e14d711ccf", "Forth"},
        {"6103619c0a5aa016870902ca3be73a40", "GOS 3.6"},
        {"e85d0ef274f24e11d7d94c94682b617b", "Maintenance 1 (alt)"},
        {"9dd72282c607842617b2b4a30e6022b2", "MCPM system"},
        {"a634ad2266fad2ec0abb1186c3846bf0", "MCPM"},
        {"155f73691b33db60924b6f9d99c05570", "Miniware 1.0"},
        {"242c335798605cec3329ef73fdc31851", "NeboCalc system"},
        {"406ffe34b008c48f43b3eda6fbe396e3", "NeboCalc"},
        {"aae03013dbdfbd58c223bc0fe0a6d8e8", "PDOS"},
        {"ae54b8aebfe111c29ad12f5510b5edb0", "Seeters"},
        {"2d0b109fec74b6502ff54a5830d23fb5", "Text 1 DE"},
        {"3215b55e6f4c9e87a699c6eb031f0fae", "Text 2 DE"},
        {"2d3e7cb3a348a17b9a046e443c38dcbe", "Text2000 2 (alt)"},
        {"5b5e3e6a3c0596ac89855b1e7108db45", "UCSD Pascal system"},
        {"ae613f20424e9bc20a214c67771a5000", "UCSD Pascal"},
        {"4d5181943573bf7ab5f8cae5ccabd328", "Flexbase 1.6"},

        // ===== Games =====
        {"d5cfc1d92e33b3420da0f6338312cf51", "Brick Wall"},
        {"e44c1d4edbbc3c7f39296d1cba321116", "Doolhof"},
        {"f30d7354a167b434107b918c3465d20d", "Fraxxon (joystick)"},
        {"0ace2d93c5fd050517f194af45a78fac", "Ghosthunt"},
        {"7010b1a04dfe5f01e54d6a3f6eab416f", "Lazy Bug"},
        {"425ee14c206efc8ca0f9cd1d001795de", "Monkey Kong"},
        {"c74fb7c0c3fb79ccebe7843470ecbd79", "Multipede"},
        {"49298c95a412800bbd60c677e69e3d34", "Space Fight"},
        {"8ddc72b03929796c9969097332cfa749", "Tetris"}
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
