/*
   Copyright (C) 2018 Arto Hyvättinen

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "apurivinkki.h"
#include "ui_apurivinkki.h"

ApuriVinkki::ApuriVinkki(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ApuriVinkki)
{
    ui->setupUi(this);

    setWindowFlags( Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute( Qt::WA_DeleteOnClose);
}

ApuriVinkki::~ApuriVinkki()
{
    delete ui;
}

void ApuriVinkki::mousePressEvent(QMouseEvent *event)
{
    // Suljetaan ApuriVinkki napsauttamalla sitä

    hide();
    QWidget::mousePressEvent(event);
}
