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
#ifndef INBOXMAARITYS_H
#define INBOXMAARITYS_H

#include "tallentavamaarityswidget.h"

namespace Ui {
    class InboxMaaritys;
}

/**
 * @brief Kirjattavien kansion valinta
 */
class InboxMaaritys : public TallentavaMaaritysWidget
{
public:
    InboxMaaritys();
    ~InboxMaaritys() override;

    bool tallenna() override;

    QString ohjesivu() override { return "maaritykset/inbox"; }

public slots:
    void valitseKansio();

private:
    Ui::InboxMaaritys *ui_;

};

#endif // INBOXMAARITYS_H
