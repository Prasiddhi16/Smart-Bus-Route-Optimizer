#include "mainwindow.h"

extern "C" {
#include "backend.h"
}
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QFrame>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QScrollArea>
#include <QFont>
//creation of the front end portion of the optimizer
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Smart City Bus Route Optimizer");
    resize(1100, 800);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCentralWidget(scrollArea);

    QWidget *contentWidget = new QWidget;
    scrollArea->setWidget(contentWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QFrame *header = new QFrame;
    header->setStyleSheet("background:#8B5A7C;color:white;border-radius:8px;padding:16px;");

    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    QLabel *title = new QLabel("Smart City Bus Route Optimizer");
    title->setFont(QFont("Segoe UI", 18, QFont::Bold));
    QLabel *subtitle = new QLabel("Find the optimal bus route based on distance, transfers, or fare");
    subtitle->setStyleSheet("opacity:0.85;");

    headerLayout->addWidget(title);
    headerLayout->addWidget(subtitle);
    mainLayout->addWidget(header);

    QGroupBox *routeFinder = new QGroupBox("Route Finder");
    QVBoxLayout *routeLayout = new QVBoxLayout(routeFinder);

    QGridLayout *formGrid = new QGridLayout;
    QComboBox *srcCombo = new QComboBox;
    srcCombo->addItems({"Stop A","Stop B","Stop C","Stop D","Stop E"});
    QComboBox *dstCombo = new QComboBox;
    dstCombo->addItems({"Stop F","Stop G","Stop H","Stop I"});

    formGrid->addWidget(new QLabel("Source Stop"),0,0);
    formGrid->addWidget(srcCombo,1,0);
    formGrid->addWidget(new QLabel("Destination Stop"),0,1);
    formGrid->addWidget(dstCombo,1,1);

    routeLayout->addLayout(formGrid);

    QHBoxLayout *algoLayout = new QHBoxLayout;
    QRadioButton *minDist = new QRadioButton("Minimize Distance");
    QRadioButton *optTime = new QRadioButton("Optimize Time");
    minDist->setChecked(true);
    algoLayout->addWidget(minDist);
    algoLayout->addWidget(optTime);
    algoLayout->addStretch();
    routeLayout->addLayout(algoLayout);

    QHBoxLayout *actionLayout = new QHBoxLayout;
    QPushButton *findBtn = new QPushButton("Find Route");
    QPushButton *clearBtn = new QPushButton("Clear");
    findBtn->setStyleSheet("background:#8B5A7C;color:white;padding:8px 16px;border-radius:6px;");
    actionLayout->addWidget(findBtn);
    actionLayout->addWidget(clearBtn);
    routeLayout->addLayout(actionLayout);

    mainLayout->addWidget(routeFinder);

    QGroupBox *visualBox = new QGroupBox("Bus Network Visualization");
    QVBoxLayout *visualLayout = new QVBoxLayout(visualBox);

    QGraphicsView *view = new QGraphicsView;
    QGraphicsScene *scene = new QGraphicsScene(view);
    view->setScene(scene);
    view->setMinimumHeight(300);
    view->setRenderHint(QPainter::Antialiasing);

    visualLayout->addWidget(view);
    mainLayout->addWidget(visualBox);

    auto stop = [&](int x, int y, const QString &name, QColor color)
    {
        scene->addEllipse(x,y,16,16,QPen(Qt::NoPen),QBrush(color));
        auto t = scene->addText(name);
        t->setPos(x - 10, y + 18);
    };

    scene->addLine(108,158,208,208,QPen(Qt::gray,2));
    scene->addLine(208,208,308,108,QPen(Qt::gray,2));
    scene->addLine(308,108,458,158,QPen(Qt::gray,2));

    stop(100,150,"Stop A",Qt::green);
    stop(200,200,"Stop B",Qt::green);
    stop(300,100,"Stop C",Qt::yellow);
    stop(450,150,"Stop F",Qt::green);

    QGroupBox *results = new QGroupBox("Results");
    QVBoxLayout *resultsLayout = new QVBoxLayout(results);
    QGridLayout *statsGrid = new QGridLayout;

    auto statCard = [&](const QString &label, const QString &value, QColor border)
    {
        QFrame *card = new QFrame;
        card->setStyleSheet(
            QString(
                "QFrame {"
                " border: 2px solid %1;"
                " border-radius: 10px;"
                " background: white;"
                "}"
                ).arg(border.name())
            );

        QVBoxLayout *l = new QVBoxLayout(card);
        l->setContentsMargins(16, 14, 16, 14);
        l->setSpacing(6);

        QLabel *labelText = new QLabel(label);
        labelText->setStyleSheet(
            "color: #555;"
            "font-size: 13px;"
            "border: none;"
            "background: transparent;"
            );

        QLabel *valueText = new QLabel(value);
        valueText->setFont(QFont("Segoe UI", 18, QFont::Bold));
        valueText->setStyleSheet(
            "border: none;"
            "background: transparent;"
            );

        l->addWidget(labelText);
        l->addWidget(valueText);

        return card;
    };



    statsGrid->addWidget(statCard("Distance","5 km",Qt::black),0,0);
    statsGrid->addWidget(statCard("Time","14 min",Qt::yellow),0,1);
    statsGrid->addWidget(statCard("Cost","$18",Qt::green),0,2);
    statsGrid->addWidget(statCard("Transfers","0",Qt::red),0,3);

    resultsLayout->addLayout(statsGrid);

    QTabWidget *tabs = new QTabWidget;

    QWidget *routeTab = new QWidget;
    QVBoxLayout *routeTabLayout = new QVBoxLayout(routeTab);
    routeTabLayout->addWidget(new QLabel("Stop A → Stop B → Stop C → Stop D"));
    routeTabLayout->addWidget(new QLabel("Bus Route: 101"));
    routeTabLayout->addWidget(new QLabel("Distance: 2.5 km | Time: 8 min"));

    QWidget *musicTab = new QWidget;
    QVBoxLayout *musicLayout = new QVBoxLayout(musicTab);
    musicLayout->addWidget(new QLabel("Lo-fi Chill"));
    musicLayout->addWidget(new QLabel("Soft Pop"));
    musicLayout->addWidget(new QLabel("Upbeat Hits"));

    tabs->addTab(routeTab,"Route Details");
    tabs->addTab(musicTab,"Music Recommendation");

    resultsLayout->addWidget(tabs);
    mainLayout->addWidget(results);

    test_backend();


}
