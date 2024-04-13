#include "hexviewwidget.h"

HexViewWidget::HexViewWidget(QWidget *parent)
    : QAbstractScrollArea{parent} {

    this->setFont(QFont("Courier", 10));

    #if QT_VERSION >= 0x051100
        this->charwidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
    #else
        this->charwidth = fontMetrics().width(QLatin1Char('9'));
    #endif

    this->charheight = fontMetrics().height();
}

void HexViewWidget::paintEvent(QPaintEvent *event) {
    QMutexLocker(&this->lock);

    if(this->data.size() == 0) {
        return;
    }

    this->update_positions();

    QPainter painter(viewport());
    QSize area_size = viewport()->size();
    QSize widget_size = this->get_widget_size();
    this->verticalScrollBar()->setPageStep(area_size.height() / charheight);
    this->verticalScrollBar()->setRange(0, (widget_size.height() - area_size.height()) / charheight + 1);

    // complete background
    painter.fillRect(event->rect(), this->palette().color(QPalette::Base));

    // background rectangle for addressses
    QColor address_area_color = QColor(0x7a613d);
//    painter.fillRect(QRect(this->pos_addr,
//                           event->rect().top() + this->charheight,
//                           this->pos_hex - GAP_ADR_HEX + 2,
//                           widget_size.height()),
//                           address_area_color);
//    painter.fillRect(QRect(event->rect().left(),
//                           event->rect().top(),
//                           event->rect().right(),
//                           this->charheight + GAP_HEADER),
//                           address_area_color);

    unsigned int start_idx = verticalScrollBar() -> value();
    unsigned int end_idx = start_idx + area_size.height() / this->charheight - 1;

    // print header
    painter.setPen(address_area_color);
    painter.drawText(this->pos_addr, this->charheight, "Offset (h)");

    for(unsigned int i=0; i<CHARACTERS_LINE; i++) {
        const QString hex_string  = QString("%1").arg(i, 2, 16, QChar('0')).toUpper();
        painter.drawText(this->pos_hex + i * 3 * this->charwidth, this->charheight, hex_string);
    }

    painter.drawText(this->pos_ascii, this->charheight, "Decoded text");

    for(unsigned int line_idx = start_idx, ypos = this->charheight * 2;  line_idx < end_idx; line_idx++, ypos += this->charheight) {

        // print address
        painter.setPen(address_area_color);
        QString address = QString("%1").arg(line_idx * this->bytes_per_line, 10, 16, QChar('0'));
        painter.drawText(this->pos_addr, ypos, address);

        // print hex characters
        for(unsigned int i=0; i<CHARACTERS_LINE; i++) {
            if(i % 2 == 0) {
                painter.setPen(QColor((unsigned int)0xFF000000));
            } else {
                painter.setPen(QColor((unsigned int)0x00888888));
            }
            const uint8_t ch = this->data[line_idx * bytes_per_line + i];
            const QString hex_string  = QString("%1").arg(ch, 2, 16, QChar('0')).toUpper();
            painter.drawText(this->pos_hex + i * 3 * this->charwidth, ypos, hex_string);
        }

        // print ascii characters
        painter.setPen(QColor((unsigned int)0x000000));
        for(unsigned int i=0; i<CHARACTERS_LINE; i++) {
            uint8_t ch = this->data[line_idx * bytes_per_line + i];
            if ((ch < 0x20) || (ch > 0x7e)) {
                ch = '.';
            }
            painter.drawText(this->pos_ascii + i * this->charwidth, ypos, QString(ch));
        }
    }
}

void HexViewWidget::mouseMoveEvent(QMouseEvent *event) {
    this->viewport()->update();
}

void HexViewWidget::mousePressEvent(QMouseEvent *event) {
    this->viewport()->update();
}

QSize HexViewWidget::get_widget_size() const {
    if(this->data.size() == 0) {
        return QSize(0, 0);
    }

    unsigned int width = CHARACTERS_LINE + (this->bytes_per_line * this->charwidth);
    unsigned int height = this->data.size() / this->bytes_per_line;
    if(this->data.size() % this->bytes_per_line) {
        height++;
    }

    height *= this->charheight;

    return QSize(width, height);
}

void HexViewWidget::update_positions() {
    #if QT_VERSION >= 0x051100
        this->charwidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
    #else
        this->charwidth = fontMetrics().width(QLatin1Char('9'));
    #endif
    this->charheight = fontMetrics().height();

    int service_symbol_width = ADR_LENGTH * this->charwidth + GAP_ADR_HEX + GAP_HEX_ASCII;

    this->bytes_per_line = (this->size().width() - service_symbol_width) / (4 * this->charwidth) - 1; // 4 symbols per byte

    this->pos_addr = 0;
    this->pos_hex = ADR_LENGTH * this->charwidth + GAP_ADR_HEX;
    this->pos_ascii = pos_hex + (CHARACTERS_LINE * 3 - 1) * this->charwidth + GAP_HEX_ASCII;
}
