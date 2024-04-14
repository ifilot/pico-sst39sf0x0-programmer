#include "hexviewwidget.h"

HexViewWidget::HexViewWidget(QWidget *parent)
    : QAbstractScrollArea{parent} {
}

void HexViewWidget::paintEvent(QPaintEvent *event) {
    QMutexLocker(&this->lock);

    this->update_positions();

    QPainter painter(viewport());
    QSize area_size = viewport()->size();
    QSize widget_size = this->get_widget_size();
    this->verticalScrollBar()->setPageStep(area_size.height() / charheight);
    this->verticalScrollBar()->setRange(0, (widget_size.height() - area_size.height()) / charheight + 1);

    // grab colors
    settings.sync();
    QColor background_color = QColor(settings.value("background_color", BACKGROUND_COLOR_DEFAULT).toUInt());
    QColor address_color = QColor(settings.value("address_color", ADDRESS_COLOR_DEFAULT).toUInt());
    QColor header_color = QColor(settings.value("header_color", HEADER_COLOR_DEFAULT).toUInt());
    QColor column_color = QColor(settings.value("column_color", COLUMN_COLOR_DEFAULT).toUInt());
    QColor alt_column_color = QColor(settings.value("alt_column_color", ALT_COLUMN_COLOR_DEFAULT).toUInt());
    QColor ascii_color = QColor(settings.value("ascii_color", ASCII_COLOR_DEFAULT).toUInt());

    // set background color
    painter.fillRect(event->rect(), background_color);

//    qDebug() << "Loading " << column_color.name() << " for column color";
//    qDebug() << "Loading " << text_color.name() << " for text color";
//    qDebug() << "Loading " << header_color.name() << " for header color";

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
    painter.setPen(header_color);
    painter.drawText(this->pos_addr, this->charheight, "Offset (h)");

    for(unsigned int i=0; i<CHARACTERS_LINE; i++) {
        const QString hex_string  = QString("%1").arg(i, 2, 16, QChar('0')).toUpper();
        painter.drawText(this->pos_hex + i * 3 * this->charwidth, this->charheight, hex_string);
    }

    painter.drawText(this->pos_ascii, this->charheight, "Decoded text");

    if(this->data.size() == 0) {
        return;
    }

    for(unsigned int line_idx = start_idx, ypos = this->charheight * 2;  line_idx < end_idx; line_idx++, ypos += this->charheight) {

        // print address
        painter.setPen(address_color);
        QString address = QString("%1").arg(line_idx * this->bytes_per_line, 10, 16, QChar('0'));
        painter.drawText(this->pos_addr, ypos, address);

        // print hex characters
        for(unsigned int i=0; i<CHARACTERS_LINE; i++) {
            if(i % 2 == 0) {
                painter.setPen(column_color);
            } else {
                painter.setPen(alt_column_color);
            }
            const uint8_t ch = this->data[line_idx * bytes_per_line + i];
            const QString hex_string  = QString("%1").arg(ch, 2, 16, QChar('0')).toUpper();
            painter.drawText(this->pos_hex + i * 3 * this->charwidth, ypos, hex_string);
        }

        // print ascii characters
        painter.setPen(ascii_color);
        for(unsigned int i=0; i<CHARACTERS_LINE; i++) {
            uint8_t ch = this->data[line_idx * bytes_per_line + i];
            if ((ch < 0x20) || (ch > 0x7e)) {
                ch = '.';
            }
            painter.drawText(this->pos_ascii + i * this->charwidth, ypos, QChar(ch));
        }
    }
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
    this->charwidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
    this->charheight = fontMetrics().height();

    int service_symbol_width = ADR_LENGTH * this->charwidth + GAP_ADR_HEX + GAP_HEX_ASCII;

    this->bytes_per_line = (this->size().width() - service_symbol_width) / (4 * this->charwidth) - 1; // 4 symbols per byte

    this->pos_addr = 0;
    this->pos_hex = ADR_LENGTH * this->charwidth + GAP_ADR_HEX;
    this->pos_ascii = pos_hex + (CHARACTERS_LINE * 3 - 1) * this->charwidth + GAP_HEX_ASCII;
}
