/****************************************************************************
 *                                                                          *
 *   PICO-SST39SF0x0-FLASHER                                                *
 *   Copyright (C) 2023 Ivo Filot <ivo@ivofilot.nl>                         *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU Lesser General Public License as         *
 *   published by the Free Software Foundation, either version 3 of the     *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public license      *
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>. *
 *                                                                          *
 ****************************************************************************/

#include "dialogslotselection.h"

DialogSlotSelection::DialogSlotSelection(unsigned int _nr_banks) :
    nr_banks(_nr_banks)
{
    // build layout
    QVBoxLayout* layout = new QVBoxLayout();
    this->setLayout(layout);
    layout->addWidget(new QLabel("Please select a ROM slot"));

    // add container for ROM slots
    QGroupBox* groupbox = new QGroupBox("ROM slots");
    layout->addWidget(groupbox);
    QGridLayout* grid = new QGridLayout();
    groupbox->setLayout(grid);

    // add buttons
    for(unsigned int i=0; i<(this->nr_banks/8); i++) {
        for(unsigned int j=0; j<8; j++) {
            QPushButton* btn = new QPushButton(tr("%1").arg(i*8+j));
            grid->addWidget(btn, i, j);
            connect(btn, SIGNAL(released()), this, SLOT(slot_select_romslot()));
        }
    }
}

void DialogSlotSelection::slot_select_romslot() {
    QPushButton* btn = qobject_cast<QPushButton *>(sender());
    this->slot_id = btn->text().toInt();
    this->accept();
}
