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

/**
  * @dir arkistoija
  * @brief Arkiston muodostaminen
  */

#ifndef ARKISTOIJA_H
#define ARKISTOIJA_H

#include <QObject>

#include <QDir>
#include <QByteArray>
#include <QTextStream>
#include <QBuffer>

#include "db/kirjanpito.h"

/**
 * @brief Arkiston kirjoittaja
 */
class Arkistoija : public QObject
{
    Q_OBJECT
protected:
    Arkistoija(Tilikausi tilikausi);
    
    void luoHakemistot();
    void arkistoiTositteet();

    void kirjoitaIndeksiJaArkistoiRaportit();

    void arkistoiTiedosto(const QString& tiedostonnimi,
                          const QString& html);

    void arkistoiByteArray(const QString& tiedostonnimi, const QByteArray& array);

    void kirjoitaHash();

    QString navipalkki(int edellinen=0, int seuraava=0);
    
    QDir hakemisto_;
    Tilikausi tilikausi_;    

    bool onkoLogoa = false;

    QByteArray shaBytes;
    
public:    
    /**
     * @brief Tallentaa kirjanpitoarkiston
     * @param tilikausi
     * @return Sha256-tiiviste heksamuodossa
     */
    static QString arkistoi(Tilikausi &tilikausi);
};

#endif // ARKISTOIJA_H
