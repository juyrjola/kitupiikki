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

#include <QAction>
#include <QActionGroup>

#include <QStackedWidget>
#include <QToolBar>
#include <QSettings>
#include <QStatusBar>
#include <QFileDialog>
#include <QDateEdit>
#include <QMouseEvent>
#include <QShortcut>

#include <QMenuBar>

#include <QDebug>
#include <QDockWidget>

#include <QDesktopServices>
#include <QUrl>
#include <QFile>

#include "kitupiikkiikkuna.h"

#include "aloitussivu/aloitussivu.h"
#include "kirjaus/kirjaussivu.h"
#include "maaritys/maarityssivu.h"
#include "selaus/selauswg.h"
#include "raportti/raporttisivu.h"
#include "arkisto/arkistosivu.h"
#include "uusikp/uusikirjanpito.h"
#include "laskutus/laskusivu.h"

#include "db/kirjanpito.h"

#include "onniwidget.h"

#include "lisaikkuna.h"
#include "laskutus/laskudialogi.h"
#include "kirjaus/siirrydlg.h"

#include "tools/inboxlista.h"
#include "maaritys/alvmaaritys.h"

KitupiikkiIkkuna::KitupiikkiIkkuna(QWidget *parent) : QMainWindow(parent),
    nykysivu(nullptr)
{

    connect( Kirjanpito::db(), SIGNAL(tietokantaVaihtui()), this, SLOT(kirjanpitoLadattu()));
    connect(kp(), SIGNAL(perusAsetusMuuttui()), this, SLOT(kirjanpitoLadattu()));

    setWindowIcon(QIcon(":/pic/Possu64.png"));
    setWindowTitle( tr("Kitupiikki %1").arg(qApp->applicationVersion()));

    aloitussivu = new AloitusSivu();
    kirjaussivu =  new KirjausSivu(this);
    laskutussivu = new LaskuSivu();
    selaussivu = new SelausWg();
    maarityssivu = new MaaritysSivu();
    raporttisivu = new RaporttiSivu();
    arkistosivu = new ArkistoSivu();

    pino = new QStackedWidget;
    setCentralWidget(pino);

    lisaaSivut();
    luoHarjoitusDock();
    luoInboxDock();

    // Himmennetään ne valinnat, jotka mahdollisia vain kirjanpidon ollessa auki
    for(int i=KIRJAUSSIVU; i<SIVUT_LOPPU;i++)
        sivuaktiot[i]->setEnabled(false);

    restoreGeometry( kp()->settings()->value("geometry").toByteArray());
    // Ladataan viimeksi avoinna ollut kirjanpito
    if( kp()->settings()->contains("viimeisin"))
    {
        QString viimeisin = kp()->settings()->value("viimeisin").toString();
        // Portable-käsittely
        if( !kp()->portableDir().isEmpty())
        {
            QDir portableDir( kp()->portableDir());
            viimeisin = QDir::cleanPath( portableDir.absoluteFilePath(viimeisin) );
        }
        // #78 Varmistetaan, että kirjanpito edelleen olemassa (0.7 8.3.2018)
        if( QFile::exists( viimeisin ) )
            Kirjanpito::db()->avaaTietokanta(viimeisin, false);
        else
            aloitussivu->kirjanpitoVaihtui();
    }
    else
        aloitussivu->kirjanpitoVaihtui();



    connect( selaussivu, SIGNAL(tositeValittu(int)), this, SLOT(naytaTosite(int)) );
    connect( aloitussivu, SIGNAL(selaus(int,Tilikausi)), this, SLOT(selaaTilia(int,Tilikausi)));
    connect( kirjaussivu, SIGNAL(palaaEdelliselleSivulle()), this, SLOT(palaaSivulta()));

    connect( kp(), SIGNAL(onni(QString)), this, SLOT(naytaOnni(QString)));
    connect( kp(), SIGNAL(naytaTosite(int)), this, SLOT(naytaTosite(int)));
    connect( aloitussivu, SIGNAL(ktpkasky(QString)), this, SLOT(ktpKasky(QString)));

    // Aktiot kirjaamisella ja selaamisella uudessa ikkunassa

    uusiKirjausAktio = new QAction(QIcon(":/pic/uusitosite.png"), tr("Kirjaa uudessa ikkunassa\tShift+F2"), this);
    connect( uusiKirjausAktio, SIGNAL(triggered(bool)), this, SLOT(uusiKirjausIkkuna()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F2), this, SLOT(uusiKirjausIkkuna()), nullptr ,Qt::ApplicationShortcut);

    uusiSelausAktio = new QAction(QIcon(":/pic/Paivakirja64.png"), tr("Selaa uudessa ikkunassa\tShift+F3"), this );
    connect( uusiSelausAktio, SIGNAL(triggered(bool)), this, SLOT(uusiSelausIkkuna()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F3), this, SLOT(uusiSelausIkkuna()), nullptr, Qt::ApplicationShortcut);

    uusiLaskuAktio = new QAction(QIcon(":/pic/lasku.png"), tr("Uusi lasku\tShift+F4"), this);
    connect( uusiLaskuAktio, SIGNAL(triggered(bool)), this, SLOT(uusiLasku()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F4), this, SLOT(uusiLasku()), nullptr, Qt::ApplicationShortcut);

    new QShortcut(QKeySequence("Ctrl+G"), this, SLOT(siirryTositteeseen()), nullptr, Qt::ApplicationShortcut);


    toolbar->installEventFilter(this);
    toolbar->setContextMenuPolicy(Qt::PreventContextMenu);



}

KitupiikkiIkkuna::~KitupiikkiIkkuna()
{
    kp()->settings()->setValue("geometry",saveGeometry());
    if( !kp()->portableDir().isEmpty())
    {
        QDir portableDir( kp()->portableDir());
        kp()->settings()->setValue("viimeisin", QDir::cleanPath( portableDir.relativeFilePath( kp()->tiedostopolku() ) ));
    }
    else
        kp()->settings()->setValue("viimeisin", kp()->tiedostopolku() );
}

void KitupiikkiIkkuna::valitseSivu(int mikasivu, bool paluu)
{
    if( !paluu )
        edellisetIndeksit.push( pino->currentIndex() );

    if( nykysivu && !nykysivu->poistuSivulta(mikasivu))
    {
        // Sivulta ei saa poistua!
        // Palautetaan valinta nykyiselle sivulle
        sivuaktiot[ pino->currentIndex() ]->setChecked(true);
        return;
    }

    nykysivu = sivut[mikasivu];
    sivuaktiot[mikasivu]->setChecked(true);

    // Sivu esille
    pino->setCurrentWidget( nykysivu);

    // Laittaa sivun valmiiksi
    nykysivu->siirrySivulle();

}


void KitupiikkiIkkuna::kirjanpitoLadattu()
{
    if( !Kirjanpito::db()->asetus("Nimi").isEmpty())
    {
        if( Kirjanpito::db()->onkoHarjoitus())
            setWindowTitle( tr("%1 - Kitupiikki %2 [Harjoittelu]").arg(Kirjanpito::db()->asetus("Nimi")).arg( qApp->applicationVersion() ));
        else
            setWindowTitle( tr("%1 - Kitupiikki %2").arg(Kirjanpito::db()->asetus("Nimi")).arg(qApp->applicationVersion()));

        harjoitusDock->setVisible( Kirjanpito::db()->onkoHarjoitus());

        for(int i=KIRJAUSSIVU; i<SIVUT_LOPPU;i++)
            sivuaktiot[i]->setEnabled(true);
    }

    edellisetIndeksit.clear();  // Tyhjennetään "selaushistoria"
}

void KitupiikkiIkkuna::palaaSivulta()
{
    if( !edellisetIndeksit.isEmpty())
        valitseSivu( edellisetIndeksit.pop(), true );
}

void KitupiikkiIkkuna::selaaTilia(int tilinumero, const Tilikausi& tilikausi)
{
    valitseSivu( SELAUSSIVU );
    selaussivu->selaa(tilinumero, tilikausi);
}

void KitupiikkiIkkuna::uusiKirjausIkkuna()
{
    LisaIkkuna *ikkuna = new LisaIkkuna;
    ikkuna->kirjaa();
}

void KitupiikkiIkkuna::uusiSelausIkkuna()
{
    LisaIkkuna *ikkuna = new LisaIkkuna;
    ikkuna->selaa();
}

void KitupiikkiIkkuna::uusiLasku()
{
    if( !LaskuDialogi::laskuIkkunoita() )
    {
        // Ei salli useampaa laskuikkunaa!
        LaskuDialogi *dlg = new LaskuDialogi();
        dlg->show();
    }
    else
    {
        QMessageBox::information(this, tr("Uutta laskua ei voi luoda"),
                                 tr("Päällekkäisten viitenumeroiden välttämiseksi voit tehdä vain "
                                    "yhden laskun kerrallaan.\n"
                                    "Sulje avoinna oleva laskuikkuna ennen uuden laskun luomista."));
    }
}

void KitupiikkiIkkuna::aktivoiSivu(QAction *aktio)
{
    int sivu = aktio->data().toInt();
    if( sivu == KIRJAUSSIVU)
    {
        // Kun kirjaussivu valitaan, tyhjennetään edellisten luettelo jottei
        // tule paluuta kirjauksen jälkeen
        edellisetIndeksit.clear();
        valitseSivu(KIRJAUSSIVU, true);
    }
    else
        valitseSivu(sivu);
}

void KitupiikkiIkkuna::naytaTosite(int tositeid)
{
    valitseSivu( KIRJAUSSIVU );
    kirjaussivu->naytaTosite(tositeid);
}

void KitupiikkiIkkuna::ktpKasky(const QString& kasky)
{
    if( kasky.startsWith("maaritys/"))
    {
        valitseSivu( MAARITYSSIVU, true );
        maarityssivu->valitseSivu(kasky.mid(9));
    }
    else if(kasky=="alvilmoitus")
    {
        valitseSivu(MAARITYSSIVU, true);
        maarityssivu->valitseSivu("Arvonlisävero");
        AlvMaaritys *alv = qobject_cast<AlvMaaritys*>(maarityssivu->nykyWidget());
        if(alv)
            alv->ilmoita();
    }
    else if( kasky == "raportit")
        valitseSivu( TULOSTESIVU, true);
    else if( kasky == "kirjaa")
        valitseSivu( KIRJAUSSIVU, true);
    else if( kasky == "uusitilikausi" || kasky=="arkisto" || kasky=="tilinpaatos")
    {
        valitseSivu( ARKISTOSIVU, true);
        if( kasky == "uusitilikausi")
            arkistosivu->uusiTilikausi();
        else if(kasky == "tilinpaatos")
            arkistosivu->tilinpaatos();
    }
    else if( kasky == "paivitatilikartta")
    {
        maarityssivu->paivitaTilikartta();
        aloitussivu->siirrySivulle();   // Päivitä aloitussivu, jotta päivitysinfo katoaa
    }
}

void KitupiikkiIkkuna::naytaOnni(const QString &teksti)
{
    OnniWidget *onni = new OnniWidget(this);
    onni->nayta( teksti );
    onni->move( ( width() - onni->width()) / 2 ,
                height() - onni->height());
}

void KitupiikkiIkkuna::ohje()
{
    if( nykysivu )
        kp()->ohje( nykysivu->ohjeSivunNimi() );
    else
        kp()->ohje();
}

void KitupiikkiIkkuna::siirryTositteeseen()
{
    int id = SiirryDlg::tositeId(kp()->paivamaara(), "" );
    if( !id || (nykysivu && !nykysivu->poistuSivulta(KIRJAUSSIVU) ))
    {
        return;
    }
    valitseSivu(KIRJAUSSIVU, false);
    kirjaussivu->naytaTosite( id );

}

void KitupiikkiIkkuna::mousePressEvent(QMouseEvent *event)
{
    // Vähän kokeellista: palataan edelliselle sivulle, jos menty Käy-valinnalla ;)
    if( event->button() == Qt::BackButton )
        palaaSivulta();

    QMainWindow::mousePressEvent(event);
}

bool KitupiikkiIkkuna::eventFilter(QObject *watched, QEvent *event)
{
    // Jos painetaan vasemmalla napilla Kirjausta tai Selausta,
    // tarjotaan mahdollisuus avata toiminto uudessa ikkunassa

    if( watched == toolbar && event->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
        if( mouse->button() == Qt::RightButton
            && ( toolbar->actionAt( mouse->pos() ) == sivuaktiot[KIRJAUSSIVU ] || toolbar->actionAt( mouse->pos() ) == sivuaktiot[SELAUSSIVU ] ||
                 toolbar->actionAt( mouse->pos() ) == sivuaktiot[LASKUTUSSIVU ] ))
        {
            QMenu valikko;
            if( toolbar->actionAt( mouse->pos() ) == sivuaktiot[KIRJAUSSIVU ] )
                valikko.addAction( uusiKirjausAktio );
            else if( toolbar->actionAt( mouse->pos() ) == sivuaktiot[SELAUSSIVU ] )
                valikko.addAction( uusiSelausAktio);
            else if( toolbar->actionAt( mouse->pos()) == sivuaktiot[LASKUTUSSIVU] )
                valikko.addAction( uusiLaskuAktio );

            valikko.exec(QCursor::pos() );

            return false;
        }

    }
    return QMainWindow::eventFilter(watched, event);
}

void KitupiikkiIkkuna::closeEvent(QCloseEvent *event)
{
    // Pääikkunan sulkeutuessa sivuikkunatkin suljetaan
    qApp->quit();
    event->accept();
}

QAction *KitupiikkiIkkuna::lisaaSivu(const QString &nimi, const QString &kuva, const QString &vihje, const QString &pikanappain, Sivu sivutunnus,
                                     KitupiikkiSivu *sivu)
{
    QAction *uusi = new QAction( nimi, aktioryhma);
    uusi->setIcon( QIcon(kuva));
    uusi->setToolTip( nimi + "\t " + pikanappain );
    uusi->setStatusTip(vihje);
    uusi->setShortcut(QKeySequence(pikanappain));
    uusi->setCheckable(true);
    uusi->setActionGroup(aktioryhma);
    uusi->setData(sivutunnus);
    toolbar->addAction(uusi);

    sivuaktiot[sivutunnus] = uusi;
    sivut[sivutunnus] = sivu;

    pino->addWidget(sivu);

    return uusi;
}

void KitupiikkiIkkuna::lisaaSivut()
{
    // Luodaan vasemman reunan työkalupalkki
    toolbar = new QToolBar(this);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setIconSize(QSize(64,64));
    toolbar->setStyleSheet("QToolBar {background-color: palette(mid); spacing: 5px; }  QToolBar::separator { border: none; margin-bottom: 16px; }  QToolButton { border: 0px solid lightgray; margin-right: 0px; font-size: 8pt; width: 90%; margin-left: 3px; margin-top: 0px; border-top-left-radius: 6px; border-bottom-left-radius: 6px}  QToolButton:checked {background-color: palette(window); } QToolButton:hover { font-size: 9pt; font-weight: bold; } ");
    toolbar->setMovable(false);

    aktioryhma = new QActionGroup(this);
    lisaaSivu("Aloita",":/pic/Possu64.png","Erilaisia ohjattuja toimia","Home", ALOITUSSIVU, aloitussivu);
    lisaaSivu("Uusi\ntosite",":/pic/uusitosite.png","Kirjaa uusi tosite","F2", KIRJAUSSIVU, kirjaussivu);
    lisaaSivu("Selaa",":/pic/Paivakirja64.png","Selaa kirjauksia aikajärjestyksessä","F3", SELAUSSIVU, selaussivu);
    lisaaSivu("Laskut",":/pic/lasku.png","Laskuta ja selaa laskuja","F4",LASKUTUSSIVU, laskutussivu);
    lisaaSivu("Tulosteet",":/pic/print.png","Tulosta erilaisia raportteja","F5", TULOSTESIVU, raporttisivu);
    lisaaSivu("Tilikaudet",":/pic/kirja64.png","Tilinpäätös ja arkistot","F6", ARKISTOSIVU, arkistosivu);
    lisaaSivu("Määritykset",":/pic/ratas.png","Kirjanpitoon liittyvät määritykset","F7", MAARITYSSIVU, maarityssivu);

    // Possulla on tonttulakki tuomaanpäivästä loppiaiseen ;)
    if( (QDate::currentDate().month() == 12 && QDate::currentDate().day() >= 21) ||
        (QDate::currentDate().month() == 1 && QDate::currentDate().day() <= 6) )
        sivuaktiot[ALOITUSSIVU]->setIcon(QIcon(":/pic/Joulupossu.png"));


    aktioryhma->actions().first()->setChecked(true);

    connect(aktioryhma, SIGNAL(triggered(QAction*)), this, SLOT(aktivoiSivu(QAction*)));

    QWidget *vali = new QWidget();
    vali->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(vali);

    QAction *ohjeAktio = new QAction(QIcon(":/pic/ohje.png"),tr("Käsikirja"), this);
    ohjeAktio->setShortcut( QKeySequence(Qt::Key_F1));
    ohjeAktio->setToolTip("Ohjeet \tF1");
    connect( ohjeAktio, SIGNAL(triggered(bool)), this, SLOT(ohje()));
    toolbar->addAction(ohjeAktio);


    addToolBar(Qt::LeftToolBarArea, toolbar);
}


void KitupiikkiIkkuna::luoHarjoitusDock()
{
    QLabel *teksti = new QLabel("<b>Harjoittelutila käytössä</b> Voit nopeuttaa ajan kulumista");
    teksti->setStyleSheet("color: white;");

    QDateEdit *pvmedit = new QDateEdit;
    pvmedit->setDate( QDate::currentDate());
    pvmedit->setStyleSheet("background: palette(window); border-radius: 0px; border: 1px solid black; color: palette(text);");
    pvmedit->setCalendarPopup(true);

    QHBoxLayout *leiska = new QHBoxLayout;
    leiska->addWidget(teksti, 3);
    leiska->addWidget(pvmedit,1, Qt::AlignRight);

    QWidget *wg = new QWidget;
    wg->setLayout(leiska);

    harjoitusDock = new QDockWidget;
    harjoitusDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    harjoitusDock->setWidget(wg);
    harjoitusDock->setStyleSheet("background: green; border-bottom-left-radius: 10px;");
    harjoitusDock->setTitleBarWidget(new QWidget(this));

    addDockWidget(Qt::TopDockWidgetArea, harjoitusDock);
    connect( pvmedit, SIGNAL(dateChanged(QDate)), Kirjanpito::db(), SLOT(asetaHarjoitteluPvm(QDate)));
    connect( pvmedit, SIGNAL(dateChanged(QDate)), aloitussivu, SLOT(siirrySivulle()));  // Jotta päivittyy ;)
    harjoitusDock->setVisible(false);
}

void KitupiikkiIkkuna::luoInboxDock()
{
    InboxLista *inbox = new InboxLista;

    inboxDock = new QDockWidget(tr("Kirjattavat"));
    inboxDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    inboxDock->setWidget(inbox);

    addDockWidget(Qt::RightDockWidgetArea, inboxDock);
    inboxDock->setVisible( false );
    connect( inbox, &InboxLista::nayta, inboxDock, &QDockWidget::setVisible );

}
