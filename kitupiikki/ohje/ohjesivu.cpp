/*
   Copyright (C) 2017 Arto Hyvättinen

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

#include "ohjesivu.h"

#include <QWebEngineView>
#include <QHBoxLayout>

OhjeSivu::OhjeSivu(QWidget *parent) : KitupiikkiSivu(parent)
{
    QWebEngineView *view = new QWebEngineView;
    QHBoxLayout *leiska = new QHBoxLayout;
    leiska->addWidget(view);
    setLayout(leiska);

    view->load(QUrl("https://artoh.github.io/kitupiikki"));
}