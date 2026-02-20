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
#include <QGraphicsLineItem>
#include <QScrollArea>
#include <QFont>
#include <QMap>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSlider>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Smart City Bus Route Optimizer");
    resize(1100, 800);

    initbusnet();

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCentralWidget(scrollArea);

    QWidget *contentWidget = new QWidget;
    scrollArea->setWidget(contentWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20,20,20,20);

    QFrame *header = new QFrame;
    header->setStyleSheet("background:#8B5A7C;color:white;border-radius:8px;padding:16px;");
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    QLabel *title = new QLabel("Smart City Bus Route Optimizer");
    title->setFont(QFont("Segoe UI",18,QFont::Bold));
    QLabel *subtitle = new QLabel("Find the optimal bus route based on distance, transfers, or fare");
    subtitle->setStyleSheet("opacity:0.85;");
    headerLayout->addWidget(title);
    headerLayout->addWidget(subtitle);
    mainLayout->addWidget(header);

    QGroupBox *routeFinder = new QGroupBox("Route Finder");
    QVBoxLayout *routeLayout = new QVBoxLayout(routeFinder);
    QGridLayout *formGrid = new QGridLayout;
    QComboBox *srcCombo = new QComboBox;
    QComboBox *dstCombo = new QComboBox;
    for(int i=0;i<stopcount;i++){
        srcCombo->addItem(stops[i].name);
        dstCombo->addItem(stops[i].name);
    }
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
    view->setMinimumHeight(400);
    view->setRenderHint(QPainter::Antialiasing);
    visualLayout->addWidget(view);
    mainLayout->addWidget(visualBox);

    QMap<QString,QPointF> stopPositions;
    auto stopDraw = [&](const QString &name, int x, int y, QColor color){
        scene->addEllipse(x,y,16,16,QPen(Qt::NoPen),QBrush(color));
        auto t = scene->addText(name);
        t->setPos(x-10,y+18);
        stopPositions[name] = QPointF(x,y);
    };

    int scale=2;
    stopDraw("A",100*scale, 150*scale,Qt::green);
    stopDraw("B",200*scale,250*scale,Qt::green);
    stopDraw("C",300*scale,100*scale,Qt::yellow);
    stopDraw("D",450*scale,180*scale,Qt::green);
    stopDraw("E",500*scale,120*scale,Qt::yellow);
    stopDraw("F",150*scale,300*scale,Qt::green);
    stopDraw("G",250*scale,50*scale,Qt::yellow);
    stopDraw("H",350*scale,280*scale,Qt::green);
    stopDraw("I",450*scale,80*scale,Qt::yellow);
    stopDraw("J",550*scale,220*scale,Qt::green);
    scene->setSceneRect(scene->itemsBoundingRect());
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    for(int i=0;i<stopcount;i++){
        for(int j=0;j<stops[i].edgecount;j++){
            int v =stops[i].edge[j].to;
            if(i<v){
                scene->addLine(
                    stopPositions[stops[i].name].x()+8,
                    stopPositions[stops[i].name].y()+8,
                    stopPositions[stops[v].name].x()+8,
                    stopPositions[stops[v].name].y()+8,
                    QPen(Qt::gray, 1)
                    );
            }
        }
    }

    QGroupBox *results = new QGroupBox("Results");
    QVBoxLayout *resultsLayout = new QVBoxLayout(results);
    QGridLayout *statsGrid = new QGridLayout;

    auto statCard = [&](const QString &label, QLabel* valueLabel, QColor border){
        QFrame *card = new QFrame;
        card->setStyleSheet(QString("QFrame { border: 2px solid %1; border-radius: 10px; background: white;}").arg(border.name()));
        QVBoxLayout *l = new QVBoxLayout(card);
        l->setContentsMargins(16,14,16,14);
        l->setSpacing(6);
        QLabel *labelText = new QLabel(label);
        labelText->setStyleSheet("color:#555;font-size:13px;border:none;background:transparent;");
        valueLabel->setStyleSheet("border:none;background:transparent;");
        valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        l->addWidget(labelText);
        l->addWidget(valueLabel);
        return card;
    };

    QLabel *distanceValue = new QLabel("0 km");
    QLabel *timeValue = new QLabel("0 min");
    QLabel *costValue = new QLabel("Rs.0");
    QLabel *transferValue = new QLabel("0");
    statsGrid->addWidget(statCard("Distance",distanceValue,Qt::black),0,0);
    statsGrid->addWidget(statCard("Time",timeValue,Qt::blue),0,1);
    statsGrid->addWidget(statCard("Cost",costValue, Qt::gray),0,2);
    statsGrid->addWidget(statCard("Transfers",transferValue,Qt::red),0,3);
    resultsLayout->addLayout(statsGrid);

    QTabWidget *tabs = new QTabWidget;
    QWidget *routeTab = new QWidget;
    QVBoxLayout *routeTabLayout = new QVBoxLayout(routeTab);
    QLabel *routeDetailsLabel = new QLabel("Route will appear here");
    routeTabLayout->addWidget(routeDetailsLabel);

    QWidget *musicTab = new QWidget;
    QVBoxLayout *musicLayout = new QVBoxLayout(musicTab);
    QVBoxLayout *playlistLayout = new QVBoxLayout;
    musicLayout->addLayout(playlistLayout);

    QSlider *progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setRange(0, 100);
    QLabel *elapsedLabel = new QLabel("0:00");
    QLabel *remainingLabel = new QLabel("-0:00");
    QHBoxLayout *progressLayout = new QHBoxLayout;
    progressLayout->addWidget(elapsedLabel);
    progressLayout->addWidget(progressSlider);
    progressLayout->addWidget(remainingLabel);
    musicLayout->addLayout(progressLayout);

    QHBoxLayout *controlsLayout = new QHBoxLayout;
    QPushButton *playBtn = new QPushButton("▶ Play");
    QPushButton *pauseBtn = new QPushButton("⏸ Pause");
    QPushButton *nextBtn = new QPushButton("⏭ Next");
    QPushButton *prevBtn = new QPushButton("⏮ Prev");
    controlsLayout->addWidget(prevBtn);
    controlsLayout->addWidget(playBtn);
    controlsLayout->addWidget(pauseBtn);
    controlsLayout->addWidget(nextBtn);
    musicLayout->addLayout(controlsLayout);

    tabs->addTab(routeTab,"Route Details");
    tabs->addTab(musicTab,"Music For Your Journey!");
    resultsLayout->addWidget(tabs);
    mainLayout->addWidget(results);

    connect(findBtn,&QPushButton::clicked,this,[=]() mutable {
        QString srcName = srcCombo->currentText();
        QString dstName = dstCombo->currentText();
        if(srcName == dstName) return;

        int optimize = minDist->isChecked() ? 0 : 1;
        Routeresults result = find_route(srcName.toStdString().c_str(),
                                         dstName.toStdString().c_str(),
                                         optimize);

        distanceValue->setText(QString::number(result.distance) + " km");
        timeValue->setText(QString::number(result.time) + " min");
        costValue->setText("Rs. " + QString::number(result.cost));
        transferValue->setText(QString::number(result.transfers));

        QString pathText;
        for(int i=0;i<result.pathlength;i++){
            int node = result.path[i];
            QString stopName = stops[node].name;
            pathText += stopName;
            if(i < result.pathlength-1) pathText += " → ";
            if(i < result.pathlength-1){
                int nextNode = result.path[i+1];
                scene->addLine(
                    stopPositions[stopName].x()+8,
                    stopPositions[stopName].y()+8,
                    stopPositions[stops[nextNode].name].x()+8,
                    stopPositions[stops[nextNode].name].y()+8,
                    QPen(Qt::red,4)
                    );
            }
        }
        routeDetailsLabel->setText(pathText);

        PlaylistResult pl = recommend_playlist((int)result.time, (int)result.distance);

        QLayoutItem* child;
        while((child = playlistLayout->takeAt(0)) != nullptr){
            delete child->widget();
            delete child;
        }

        playlistLayout->addWidget(new QLabel(QString("🎵 ") + pl.name));
        for(int i=0; i<pl.trackCount; i++){
            playlistLayout->addWidget(new QLabel("• " + QString(pl.tracks[i])));
        }
    });

    connect(clearBtn,&QPushButton::clicked,this,[=](){
        for(auto item: scene->items()){
            if(auto line = dynamic_cast<QGraphicsLineItem*>(item)){
                if(line->pen().color() == Qt::red)
                    scene->removeItem(line);
            }
        }
        distanceValue->setText("0 km");
        timeValue->setText("0 min");
        costValue->setText("Rs. 0");
        transferValue->setText("0");
        routeDetailsLabel->setText("Route cleared");

        QLayoutItem* child;
        while((child = playlistLayout->takeAt(0)) != nullptr){
            delete child->widget();
            delete child;
        }
    });
}
