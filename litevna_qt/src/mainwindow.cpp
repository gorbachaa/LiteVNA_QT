#define _USE_MATH_DEFINES

#include <QMetaType>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "polarview.hpp"
#include "markerslider.hpp"
#include "impedancedisplay.hpp"
#include "frequencydialog.hpp"
#include "graphpanel.hpp"
#include "utility.hpp"
#include "touchstone.hpp"
#include "calkitsettingsdialog.hpp"
#include "ui_graphlimitsdialog.h"
#include "calibrationfinetunedialog.hpp"
#include "firmwareupdatedialog.hpp"
#include <calibration.hpp>
#include <xavna_cpp.hpp>
#include <xavna_generic.hpp>
#include <iostream>
#include <stdexcept>
#include <time.h>

#include <QString>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QGraphicsLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QSettings>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QScatterSeries>


using namespace std;
using namespace QtCharts;



MainWindow::MainWindow( QWidget *parent ) :  QMainWindow( parent ), ui( new Ui::MainWindow )
{
    ui->setupUi( this );

    vna = new VNADevice();
    impdisp = new ImpedanceDisplay();
    refreshTimer = new QTimer();

    nv.init( ui->w_bottom->layout() );
    nv.xAxisValueStr = [](double val) {
        if( val < 0.999999 ) return ssprintf( 32, "%.03f kHz", val * 1000. );
        else
        if( val <= 9.99999 ) return ssprintf( 32, "%.05f MHz", val );
        else
        if( val <= 99.9999 ) return ssprintf( 32, "%.04f MHz", val );
        else
        if( val < 999.999 ) return ssprintf( 32, "%.03f MHz", val );
        else
                            return ssprintf( 32, "%.02f MHz", val );
    };
    connect( &nv, &NetworkView::markerChanged, [this](int marker, int /*index*/) {
        if( marker == 0 ) updateValueDisplays();
    });

    connect( &dtf,&DTFWindow::hidden, [this]() {
        ui->actionTime_to_fault->setChecked( false );
    });

    loadSettings();
    setCallbacks();
    setupViews();
    populateCalTypes();
    populateDevicesMenu();

    ui->dock_ext->setVisible( false );

    ui->dock_bottom->setTitleBarWidget( new QWidget() );

    ((QBoxLayout*)ui->dock_impedance_contents->layout())->insertWidget( 0,impdisp );

    connect( refreshTimer, &QTimer::timeout, [this](){
        if( !ui->actionDisable_polarView_refresh->isChecked() ) this->polarView->repaint();
        nv.updateMarkerViews();
        updateValueDisplays();
        nv.updateBottomLabels();
    });

    setAttribute( Qt::WA_DeleteOnClose );

    enableUI( false );
}

MainWindow::~MainWindow()
{
    fflush( stdout );

    vna->stopScan();
    vna->close();

    delete refreshTimer;
    //delete chartView;
    //delete polarView;
    delete vna;
    delete ui;
    fflush( stdout );
}

void MainWindow::loadSettings()
{
    QSettings settings;
    recentFiles = settings.value( "recentFiles" ).toStringList();
    refreshRecentFiles();
    cks = settings.value("calkits").value<CalKitSettings>();

    nv.graphLimits = NetworkView::defaultGraphLimits;
}

void MainWindow::saveSettings_calKit()
{
    QSettings settings;
    QVariant var;
    var.setValue( cks );
    settings.setValue( "calkits", var );
}

void MainWindow::populateCalTypes()
{
    ui->d_caltype->clear();
    for( const VNACalibration* cal:calibrationTypes ) {
        ui->d_caltype->addItem( QString::fromStdString( cal->description() ) );
    }
}

void MainWindow::setupViews()
{
    bool tr = vna->isTRMode();

    // create smith chart view
    polarView = new PolarView();
    polarView->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Preferred );
    ui->w_polar->layout()->addWidget( polarView );
    nv.views.push_back( SParamView{
                                    { 0, 0, SParamViewSource::TYPE_COMPLEX },
                                    this->polarView, nullptr,
                                    {}, nullptr
                                  } );


    // create top-right buttons section on smith chart
    auto* topFloat = createTopFloat( polarView );
    auto* topFloatLayout = (QBoxLayout*)topFloat->layout();

    // S11/S22 radiobuttons
    for( int i = 0;i < 2; i++ ) {
        QRadioButton* rb = new QRadioButton( qs( ssprintf( 32, "S%d%d", ( i + 1 ), ( i + 1 ) ) ) );
        rb->setChecked( i == 0 );
        if( vna->isTRMode() && i != 0 )
            rb->setEnabled( false );
        topFloatLayout->insertWidget( i, rb );
        connect( rb, &QRadioButton::clicked, [this,i](){
                                        nv.views.at( 0 ).src.col = i;
                                        nv.views.at( 0 ).src.row = i;
                                        nv.updateView( 0 );
        });
    }

    // maximize button
    QPushButton* polarView_maximize = new QPushButton();
    polarView_maximize->setIcon( QIcon( ":/resources/maximize" ) );
    //polarView_maximize->setFlat( true );
    topFloatLayout->addWidget( polarView_maximize );
    connect( polarView_maximize, &QPushButton::clicked, [this, polarView_maximize](){
                                        if( maximizePane( polarView ) )
                                            polarView_maximize->setIcon( QIcon( ":/resources/unmaximize" ) );
                                        else polarView_maximize->setIcon( QIcon( ":/resources/maximize" ) );
    });



    GraphPanel* gp = nv.createGraphView( true, vna->isTRMode() );
    ui->w_graph->layout()->addWidget( gp );
    connect( gp->maximizeButton(), &QPushButton::clicked, [this, gp](){
                                        if( maximizePane( gp ) )
                                            gp->maximizeButton()->setIcon( QIcon(":/resources/unmaximize" ) );
                                        else gp->maximizeButton()->setIcon( QIcon(":/resources/maximize" ) );
    });
    graphs.push_back( gp );

    updateSweepParams();
    nv.addMarker( false );

    // enable/disable menu items based on device type
    ui->actionCapture_S_1->setEnabled( tr );
    ui->actionCapture_S_2->setEnabled( tr );
    ui->actionExport_s1p_port_2->setEnabled( !tr );

    viewIsTR = tr;
}

void MainWindow::destroyViews()
{
    nv.clear();
    for( auto* graph: graphs ) {
        delete graph;
    }
    graphs.clear();
    delete polarView;
    polarView = nullptr;
}

void MainWindow::setCallbacks()
{
    vna->frequencyCompletedCallback = [this](int freqIndex, VNARawValue val) {
        //printf("frequencyCompletedCallback: %d\n",freqIndex);
        //fflush(stdout);
        this->rawValues[freqIndex] = val;
        QMetaObject::invokeMethod(this, "updateViews", Qt::QueuedConnection, Q_ARG(int, freqIndex));
    };
    vna->sweepCompletedCallback = [this](const vector<VNARawValue>&) {

    };
    vna->backgroundErrorCallback = [this](const exception& exc) {
        fprintf(stderr,"background thread error: %s\n",exc.what());
        QString msg = exc.what();
        QMetaObject::invokeMethod(this, "handleBackgroundError", Qt::QueuedConnection, Q_ARG(QString, msg));
    };
}

void MainWindow::populateDevicesMenu()
{
    bool remove = false;
    for( QAction* act: ui->menuDevice->actions() ) {
        if(act == ui->actionRefresh) break;
        if(remove) ui->menuDevice->removeAction(act);
        if(act == ui->actionSelect_device) remove = true;
    }

    vector<string> devices;
    try {
        devices = vna->findDevices();
        for( string dev:devices ) {
            QAction* action = new QAction(qs("   " + dev));
            connect(action, &QAction::triggered, [this,dev](){
                this->openDevice(dev);
            });
            ui->menuDevice->insertAction(ui->actionRefresh, action);
        }
    } catch(exception& ex) {
        cerr << ex.what() << endl;
    }
    if( devices.empty() ) {
        QAction* action = new QAction("   No devices found; check dmesg or device manager");
        action->setEnabled(false);
        ui->menuDevice->insertAction( ui->actionRefresh, action );
    }
}

void MainWindow::openDevice( string dev )
{
    try {
        vna->open(dev);
        // views need to be recreated if going in or out of T/R mode
        if(vna->isTRMode() != viewIsTR) {
            destroyViews();
            setupViews();
        }
        ui->actionT_R_mode->setChecked(vna->isTRMode());
        ui->actionT_R_mode->setEnabled(!vna->isTR());
        ui->actionSwap_ports->setEnabled(!vna->isTR());
        if(vna->isTR()) ui->actionSwap_ports->setChecked(false);
        vna->startScan();
        enableUI(true);
        refreshTimer->start(refreshIntervalMs);
    } catch(logic_error& ex) {
        if(strcmp(ex.what(), "DFU mode") == 0) {
            auto resp = QMessageBox::question(this, "DFU mode", "Device is in DFU mode. Flash a new firmware?");
            if(resp == QMessageBox::Yes) {
                QString fileName = QFileDialog::getOpenFileName(this,
                        tr("Open binary file"), "",
                        tr("Raw binary file *.bin (*.bin);;All Files (*)"));
                if (fileName.isEmpty()) return;
		try {
                    updateFirmware(dev, fileName.toStdString());
                } catch(exception& ex) {
                    QMessageBox::critical(this,"Exception",ex.what());
		}
            }
        } else {
            QMessageBox::critical(this,"Exception",ex.what());
        }
    } catch(exception& ex) {
        QMessageBox::critical(this,"Exception",ex.what());
    }
}

void MainWindow::handleBackgroundError( QString msg )
{
    vna->close();
    enableUI(false);
    refreshTimer->stop();
    QMessageBox::critical(this, "Error", msg);
}

void MainWindow::s11MeasurementCompleted( QString fileName )
{
    string data = serializeTouchstone(tmp_s11,vna->startFreqHz,vna->stepFreqHz);
    saveFile(fileName, data);
    enableUI(true);
}

void MainWindow::sparamsMeasurementCompleted( QString fileName )
{
    string data = serializeTouchstone(tmp_sparams,vna->startFreqHz,vna->stepFreqHz);
    saveFile(fileName, data);
    enableUI(true);
}

void MainWindow::sMeasurementCompleted()
{
    enableUI(true);
}

void MainWindow::updateSweepParams()
{
    double maxGrpDelayNs = (1./vna->stepFreqHz)*.5*1e9;
    nv.graphLimits[SParamViewSource::TYPE_GRPDELAY] = {0., maxGrpDelayNs};
    nv.updateYAxis();
    rawValues.resize(vna->nPoints);
    nv.updateXAxis(vna->startFreqHz*freqScale, vna->stepFreqHz*freqScale, vna->nPoints);
    dtf.updateSweepParams(vna->stepFreqHz, vna->nPoints);
}

void MainWindow::updateValueDisplays()
{
    int freqIndex = nv.markers.at(0).freqIndex;
    if(curCal) impdisp->setValue(nv.values.at(freqIndex)(0,0), vna->freqAt(freqIndex));
    else impdisp->clearValue();
}

void MainWindow::updateUIState()
{
    bool enable = uiState.enabled;
    ui->dock_bottom->setEnabled(enable);
    ui->dock_cal->setEnabled(enable);
    ui->dock_impedance->setEnabled(enable);
    ui->centralWidget->setEnabled(enable);
    //ui->menuCalibration->setEnabled(enable);
    ui->menuS_parameters->setEnabled(enable && uiState.calEnabled);
}

void MainWindow::enableUI( bool enable )
{
    uiState.enabled = enable;
    uiState.calEnabled = (curCal != nullptr);
    updateUIState();
}

bool MainWindow::maximizePane( QWidget *w )
{
    QWidget* initialMaximizedPane = maximizedPane;
    if(maximizedPane == nullptr) {
        // we are going from normal view to maximized view
        unmaximizedLayoutState = this->saveState();
        for(QDockWidget* dock:findChildren<QDockWidget*>()) {
            if(dock != ui->dock_bottom)
                dock->hide();
        }
    } else {
        this->centralWidget()->layout()->removeWidget(maximizedPane);
        auto* layout = dynamic_cast<QBoxLayout*>(maximizedPaneParent->layout());
        layout->insertWidget(maximizedPaneIndex, maximizedPane);
        maximizedPane = nullptr;
        maximizedPaneParent = nullptr;
    }
    if(initialMaximizedPane == w) {
        // we are going from maximized view to normal view
        ui->w_content->show();
        this->restoreState(unmaximizedLayoutState);
        return false;
    }
    auto* layout = dynamic_cast<QBoxLayout*>(w->parentWidget()->layout());
    maximizedPaneIndex = layout->indexOf(w);
    maximizedPane = w;
    maximizedPaneParent = w->parentWidget();
    w->parentWidget()->layout()->removeWidget(w);
    this->centralWidget()->layout()->addWidget(w);
    ui->w_content->hide();
    return true;
}

QWidget *MainWindow::createTopFloat( QWidget *w )
{
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    QBoxLayout* layout2 = new QBoxLayout(QBoxLayout::LeftToRight);

    QWidget* widget2 = new QWidget();
    layout2->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    layout2->setMargin( 0 );
    widget2->setLayout( layout2 );
    widget2->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    widget2->setStyleSheet( "background-color:transparent" );

    layout->addWidget( widget2 );
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    w->setLayout(layout);
    layout->setMargin(4);
    return widget2;
}

void MainWindow::updatePortExtension()
{
    portExt1Seconds = ui->slider_ext1->value()*1e-12;
    portExt2Seconds = ui->slider_ext2->value()*1e-12;
    portExtZ0 = ui->slider_extz->value()/100.;
    ui->t_ext1->setText(qs(ssprintf(32, "%.0f ps", round(portExt1Seconds*1e12))));
    ui->t_ext2->setText(qs(ssprintf(32, "%.0f ps", round(portExt2Seconds*1e12))));
    ui->t_extz->setText(qs(ssprintf(32, "%.2f", portExtZ0)));
    updateViews(-1);
}

static string calFileVer = "calFileVersion 1";
string MainWindow::serializeCalibration( const CalibrationInfo &cal )
{
    scopedCLocale _locale; // use standard number formatting
    string tmp;
    tmp.append( calFileVer );
    tmp += '\n';
    tmp.append(cal.calName + "\n");
    saprintf(tmp, "%d %f %f ", cal.nPoints, cal.startFreqHz, cal.stepFreqHz);
    saprintf(tmp, "%d %d", cal.attenuation1, cal.attenuation2);
    tmp += '\n';
    for(auto& calstd:cal.measurements) {
        if(calstd.first.length() == 0)
            continue;
        tmp.append(calstd.first);
        tmp += '\n';
        for(const VNARawValue& val:calstd.second) {
            int sz=val.rows()*val.cols();
            for(int i=0;i<sz;i++) {
                saprintf(tmp, "%f %f ", val(i).real(), val(i).imag());
            }
            tmp += '\n';
        }
    }
    return tmp;
}

CalibrationInfo MainWindow::deserializeCalibration( QTextStream &inp )
{
    scopedCLocale _locale; // use standard number formatting
    CalibrationInfo ret;
    string versionStr = inp.readLine().toStdString();
    if(versionStr != calFileVer) {
        throw runtime_error("Unsupported file version: "+versionStr+". Should be: "+calFileVer);
    }
    ret.calName = inp.readLine().toStdString();
    inp >> ret.nPoints;
    inp >> ret.startFreqHz;
    inp >> ret.stepFreqHz;
    inp >> ret.attenuation1;
    inp >> ret.attenuation2;
    inp.readLine();

    QString calstd;
    while((calstd = inp.readLine())!=nullptr) {
        vector<VNARawValue> values;
        for( int i = 0; i < ret.nPoints; i++ ) {
            VNARawValue value;
            for( int j = 0; j < 4; j++ ) {
                double re,im;
                inp >> re;
                inp >> im;
                value(j) = complex<double>( re, im );
            }
            values.push_back( value );
        }
        // discard the rest of the line
        inp.readLine();

        ret.measurements[calstd.toStdString()] = values;
        fprintf( stderr, "found cal standard %s\n", calstd.toUtf8().data() );
        fflush( stderr );
    }
    return ret;
}

void MainWindow::saveFile( QString path, const string &data )
{
    QFile file(path);
    if( !file.open( QIODevice::WriteOnly ) ) {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }
    file.write( data.data(), data.size() );
}

void MainWindow::saveCalibration( QString path )
{
    CalibrationInfo cal = {vna->nPoints,vna->startFreqHz,vna->stepFreqHz,
                           vna->attenuation1,vna->attenuation2,
                           curCal->name(),curCalMeasurements,
                           cks};
    saveFile(path, serializeCalibration(cal));
    addRecentFile( path );
}

void MainWindow::loadCalibration( QString path )
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }
    QTextStream stream(&file);
    CalibrationInfo calInfo = deserializeCalibration(stream);

    const VNACalibration* cal=nullptr;
    int i=0;
    for(auto* c:calibrationTypes) {
        if(c->name() == calInfo.calName) {
            cal = c;
            ui->d_caltype->setCurrentIndex(i);
            break;
        }
        i++;
    }
    if(!cal) {
        string errmsg = "The calibration file uses calibration type \""+calInfo.calName+"\" which is not supported";
        QMessageBox::critical(this,"Error",qs(errmsg));
        return;
    }
    bool scan=vna->_threadRunning;
    vna->stopScan();

    vna->nPoints = calInfo.nPoints;
    vna->startFreqHz = calInfo.startFreqHz;
    vna->stepFreqHz = calInfo.stepFreqHz;
    vna->attenuation1 = calInfo.attenuation1;
    vna->attenuation2 = calInfo.attenuation2;

    updateSweepParams();
    calMeasurements = calInfo.measurements;
    this->on_d_caltype_currentIndexChanged(ui->d_caltype->currentIndex());
    this->on_b_apply_clicked();

    if(scan) vna->startScan();
    addRecentFile(path);
}

void MainWindow::addRecentFile( QString path )
{
    recentFiles.insert(0,path);
    recentFiles.removeDuplicates();
    refreshRecentFiles();
    QSettings settings;
    settings.setValue("recentFiles", recentFiles);
}

void MainWindow::refreshRecentFiles()
{
    for(auto* act:recentFileActions) {
        ui->menuCalibration->removeAction(act);
    }
    recentFileActions.clear();
    for(QString entry:recentFiles) {
        auto* act = ui->menuCalibration->addAction(entry, [entry,this](){
            loadCalibration(entry);
        });
        recentFileActions.push_back(act);
    }
}

void MainWindow::captureSParam( vector<VNACalibratedValue> *var )
{
    enableUI(false);
    var->resize(vna->nPoints);
    vna->takeMeasurement([this,var](const vector<VNARawValue>& vals){
        assert(curCal != nullptr);
        for(int i=0;i<vna->nPoints;i++)
            var->at(i) = curCal->computeValue(curCalCoeffs.at(i),vals.at(i));
        QMetaObject::invokeMethod(this, "sMeasurementCompleted", Qt::QueuedConnection);
    });
}

QString MainWindow::fileDialogSave( QString title, QString filter, QString defaultSuffix, QString defaultFile )
{
    QFileDialog dialog( this );
    dialog.setWindowTitle( title );
    dialog.setAcceptMode( QFileDialog::AcceptSave );
    dialog.setDefaultSuffix( defaultSuffix );
    dialog.setNameFilter( filter );
    if( defaultFile != "" ) dialog.selectFile( defaultFile );
    if( dialog.exec() == QDialog::Accepted ) return dialog.selectedFiles().at( 0 );
    return nullptr;
}

string MainWindow::freqStr( double freqHz )
{
    return ssprintf( 32, "%.2f", freqHz*freqScale );
}

void MainWindow::updateFirmware( string dev, string fileName )
{
    FirmwareUpdateDialog dialog( this );
    dialog.beginUploadFirmware( dev, fileName );
    dialog.exec();
}

void MainWindow::hideEvent( QHideEvent* )
{
    // hack required because qt seems to close all dock widgets when
    // the main window is minimized
    // TODO(xaxaxa): remove this when qt bug is fixed
    layoutState = this->saveState();
}

void MainWindow::showEvent( QShowEvent* )
{
    this->restoreState( layoutState );
}

void MainWindow::on_d_caltype_currentIndexChanged( int index )
{
    QLayoutItem *child;
    while ((child = ui->w_calstandards->layout()->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }
    if(index<0 || index>=(int)calibrationTypes.size()) return;
    calButtons.clear();
    const VNACalibration* cal = calibrationTypes[index];
    for(auto calstd:cal->getRequiredStandards()) {
        string name = calstd[0];
        string desc = calstd[1];

        QPushButton* btn = new QPushButton(QString::fromStdString(desc));
        btn->setToolTip(QString::fromStdString(name));

        if(calMeasurements[name].size() != 0)
            btn->setStyleSheet(calButtonDoneStyle);
        ui->w_calstandards->layout()->addWidget(btn);
        calButtons[name] = btn;

        connect(btn,&QPushButton::clicked,[btn,this]() {
            btn_measure_click(btn);
        });
    }
}

void MainWindow::btn_measure_click( QPushButton *btn )
{
    string name = btn->toolTip().toStdString();
    cout << name << endl;
    ui->dock_cal_contents->setEnabled(false);
    vna->takeMeasurement([this,name](const vector<VNARawValue>& vals){
        calMeasurements[name] = vals;
        QMetaObject::invokeMethod(this, "calMeasurementCompleted", Qt::QueuedConnection, Q_ARG(string, name));
    });
}

void MainWindow::on_actionOther_triggered()
{
   auto tmp = QInputDialog::getText(this,"Specify device path...","Device (/dev/tty... on linux/mac, COM... on windows)").toStdString();
   if(tmp != "") openDevice(tmp);
}

void MainWindow::calMeasurementCompleted( string calName )
{
    ui->dock_cal_contents->setEnabled(true);
    calButtons[calName]->setStyleSheet(calButtonDoneStyle);
}

void MainWindow::on_b_clear_m_clicked()
{
    calMeasurements.clear();
    for(auto& tmp:calButtons) {
        tmp.second->setStyleSheet("");
    }
}

void MainWindow::on_b_apply_clicked()
{
    int index = ui->d_caltype->currentIndex();
    auto* cal = calibrationTypes[index];

    auto calStds = cal->getRequiredStandards();
    vector<string> names;
    for(auto tmp:calStds) {
        names.push_back(tmp[0]);
    }
    // indexed by frequency, calStdType
    vector<vector<VNARawValue> > measurements(vna->nPoints);
    vector<vector<VNACalibratedValue> > calStdModels(vna->nPoints);

    for(int i=0;i<(int)names.size();i++) {
        if(int(calMeasurements[names[i]].size()) != vna->nPoints) {
            QMessageBox::critical(this,"Error",qs("measurement for \"" + names[i] + "\" does not exist"));
            return;
        }
    }

    // populate measurements and calStdModels
    for(int j=0;j<(int)measurements.size();j++) {
        measurements[j].resize(names.size());
        calStdModels[j].resize(names.size());
        for(int i=0;i<(int)names.size();i++) {
            measurements[j][i] = calMeasurements[names[i]].at(j);
        }
    }
    // populate calStdModels
    for(int i=0;i<(int)names.size();i++) {
        string name = names[i];
        // if the cal standard not set in settings, use ideal parameters
        if(cks.calKitModels.find(name) == cks.calKitModels.end()) {
            printf("using ideal parameters for %s\n", name.c_str());
            assert(idealCalStds.find(name) != idealCalStds.end());
            auto tmp = idealCalStds[name];
            for(int j=0;j<(int)measurements.size();j++)
                calStdModels[j][i] = tmp;
        } else {
            printf("using user parameters for %s\n", name.c_str());
            auto tmp = cks.calKitModels[name];
            for(int j=0;j<(int)measurements.size();j++) {
                auto sss = tmp.interpolate(vna->freqAt(j));
                printf("%lf dB, %lf deg\n", dB(norm(sss(0,0))), arg(sss(0,0))*180/M_PI);
                calStdModels[j][i] = sss;
            }
        }
    }
    curCal = cal;
    curCalMeasurements = calMeasurements;
    curCalMeasurementsArray = measurements;
    curCalStdModelsArray = calStdModels;
    curCalCoeffs.resize(vna->nPoints);
    for(int i=0;i<(int)measurements.size();i++) {
        curCalCoeffs[i] = curCal->computeCoefficients(measurements[i], calStdModels[i]);
    }
    ui->l_cal->setText(qs(curCal->description()));
    uiState.calEnabled = true;
    updateUIState();
}

void MainWindow::on_b_clear_clicked()
{
    curCal = NULL;
    curCalCoeffs.clear();
    tmp_sn1.clear();
    tmp_sn2.clear();
    ui->l_cal->setText("None");
    uiState.calEnabled = false;
    updateUIState();
}

void MainWindow::on_actionSweep_params_triggered()
{
    FrequencyDialog dialog(this);
    dialog.fromVNA(*vna);
    if(dialog.exec() == QDialog::Accepted) {
        bool running = vna->_threadRunning;
        if(running) vna->stopScan();
        if(dialog.toVNA(*vna)) {
            emit on_b_clear_clicked();
            emit on_b_clear_m_clicked();
        }
        updateSweepParams();
        if(running) vna->startScan();
    }
}

complex<double> calculatePortExt( complex<double> refl, complex<double> Tcable, double Z0, double Zcable )
{
    double Z0sq = Z0*Z0;
    double Zcablesq = Zcable*Zcable;

    /* derived using sympy:
    from sympy import *

    def toRefl(Z, Z0):
        return (Z/Z0-1)/(Z/Z0+1)

    def toZ(refl, Z0):
        return (1+refl)/(1-refl)*Z0

    def convertRefl(refl, Z0, newZ0):
        Z = toZ(refl, Z0)
        return toRefl(Z, newZ0)

    refl, Z0, Zcable, Tcable = symbols('refl Z0 Zcable Tcable')

    cableFarRefl = convertRefl(refl, Z0, Zcable)
    cableNearRefl = cableFarRefl*Tcable
    nearRefl = convertRefl(cableNearRefl, Zcable, Z0)

    result = simplify(nearRefl)
    result = collect(result, Tcable)
    result = collect(result, refl)

    print result
    */

    //auto result = (Tcable*(-Z0sq*refl - Z0sq - 2*Z0*Zcable*refl - Zcablesq*refl + Zcablesq) + Z0sq - Zcablesq + refl*(Z0sq - 2*Z0*Zcable + Zcablesq))
    //            / (Tcable*(Z0sq*refl + Z0sq - 2*Z0*Zcable - Zcablesq*refl + Zcablesq) - Z0sq - 2*Z0*Zcable - Zcablesq + refl*(-Z0sq + Zcablesq));
    auto term1 = Z0sq*refl + Z0sq;
    auto term2 = -Zcablesq*refl + Zcablesq;
    auto term3 = 2*Z0*Zcable;
    auto result = (Tcable*(-term1 - term3*refl + term2) + Z0sq - Zcablesq + refl*(Z0sq - term3 + Zcablesq))
                / (Tcable*(term1  - term3      + term2) - Z0sq - Zcablesq - term3 + refl*(-Z0sq + Zcablesq));
    return result;
}

void MainWindow::updateViews( int freqIndex )
{
    computeViews(freqIndex);
    if(ui->actionDisable_chart_update->isChecked()) return;
    nv.updateViews(freqIndex);

    time_t t = time(nullptr);
    if(abs(int64_t(t) - int64_t(lastDTFUpdate)) >= 1)
        dtf.updateValues(nv.values);
    lastDTFUpdate = t;
}

void MainWindow::computeViews( int freqIndex )
{
    if(freqIndex < 0) {
        for(int i=0; i<vna->nPoints; i++)
            computeViews(i);
        return;
    }
    if(freqIndex >= (int)nv.values.size()) return;
    if(curCal)
        nv.values.at(freqIndex) = curCal->computeValue(curCalCoeffs.at(freqIndex), this->rawValues.at(freqIndex));
    else nv.values.at(freqIndex) = (VNACalibratedValue) this->rawValues.at(freqIndex);

    // calculate port extension
    double freqHz = vna->freqAt(freqIndex);

    auto& S11 = nv.values.at(freqIndex)(0, 0);
    S11 = calculatePortExt(S11, polar(1., 4*M_PI*freqHz*portExt1Seconds), 50., portExtZ0);

    auto& S22 = nv.values.at(freqIndex)(1, 1);
    S22 = calculatePortExt(S22, polar(1., 4*M_PI*freqHz*portExt2Seconds), 50., portExtZ0);

    // S21
    nv.values.at(freqIndex)(1, 0) *= polar(1., 2*M_PI*freqHz*(portExt1Seconds+portExt2Seconds));
    // S12
    nv.values.at(freqIndex)(0, 1) *= polar(1., 2*M_PI*freqHz*(portExt1Seconds+portExt2Seconds));
}

void MainWindow::on_actionLoad_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open calibration"), "",
            tr("VNA calibration (*.cal);;All Files (*)"));
    if (fileName.isEmpty()) return;
    loadCalibration(fileName);
}

void MainWindow::on_actionSave_triggered()
{
    if(curCal == NULL) {
        QMessageBox::critical(this, "Error", "No calibration is currently active");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save calibration"), "",
            tr("VNA calibration (*.cal);;All Files (*)"));
    if (fileName.isEmpty()) return;
    saveCalibration(fileName);
}

void MainWindow::export_s1p( int port )
{
    QString fileName = fileDialogSave(
            tr("Save S parameters"),
            tr("Touchstone .s1p (*.s1p);;All Files (*)"), "s1p");
    if (fileName.isEmpty()) return;

    tmp_s11.resize(vna->nPoints);
    enableUI(false);
    vna->takeMeasurement([this,fileName,port](const vector<VNARawValue>& vals){
        assert(curCal != nullptr);
        for(int i=0;i<vna->nPoints;i++)
            tmp_s11.at(i) = curCal->computeValue(curCalCoeffs.at(i),vals.at(i))(port,port);
        QMetaObject::invokeMethod(this, "s11MeasurementCompleted", Qt::QueuedConnection, Q_ARG(QString, fileName));
    });
}

void MainWindow::on_actionExport_s1p_triggered()
{
    export_s1p(0);
}

void MainWindow::on_actionExport_s1p_port_2_triggered()
{
    export_s1p(1);
}

void MainWindow::on_actionCapture_S_1_triggered()
{
    captureSParam(&tmp_sn1);
}

void MainWindow::on_actionCapture_S_2_triggered()
{
    captureSParam(&tmp_sn2);
}

void MainWindow::on_actionExport_s2p_triggered()
{
    vector<VNACalibratedValue> res(vna->nPoints);
    if(vna->isTRMode()) {
        if(int(tmp_sn1.size())!=vna->nPoints) {
            QMessageBox::critical(this,"Error","S*1 has not been captured");
            return;
        }
        if(int(tmp_sn2.size())!=vna->nPoints) {
            QMessageBox::critical(this,"Error","S*2 has not been captured");
            return;
        }
        for(int i=0;i<vna->nPoints;i++) {
            res[i] << tmp_sn1[i](0,0), tmp_sn2[i](1,0),
                    tmp_sn1[i](1,0), tmp_sn2[i](0,0);
        }
    }

    QString fileName = fileDialogSave(
            tr("Save S parameters"),
            tr("Touchstone .s2p (*.s2p);;All Files (*)"), "s2p");
    if (fileName.isEmpty()) return;

    if(vna->isTRMode()) {
        string data = serializeTouchstone(res,vna->startFreqHz,vna->stepFreqHz);
        saveFile(fileName, data);
    } else {
        tmp_sparams.resize(vna->nPoints);
        enableUI(false);
        vna->takeMeasurement([this,fileName](const vector<VNARawValue>& vals){
            assert(curCal != nullptr);
            for(int i=0;i<vna->nPoints;i++)
                tmp_sparams.at(i) = curCal->computeValue(curCalCoeffs.at(i),vals.at(i));
            QMetaObject::invokeMethod(this, "sparamsMeasurementCompleted", Qt::QueuedConnection, Q_ARG(QString, fileName));
        });
    }

}

void MainWindow::on_actionImpedance_pane_toggled( bool arg1 )
{
    ui->dock_impedance->setVisible(arg1);
}

void MainWindow::on_actionCalibration_pane_toggled( bool arg1 )
{
    ui->dock_cal->setVisible( arg1 );
}

void MainWindow::on_dock_cal_visibilityChanged( bool visible )
{
    if( visible != ui->actionCalibration_pane->isChecked() )
        ui->actionCalibration_pane->setChecked( visible );
}

void MainWindow::on_dock_impedance_visibilityChanged( bool visible )
{
    if(visible != ui->actionImpedance_pane->isChecked())
        ui->actionImpedance_pane->setChecked( visible );
}

void MainWindow::on_actionRefresh_triggered()
{
    populateDevicesMenu();
}

void MainWindow::on_actionKit_settings_triggered()
{
    CalKitSettingsDialog dialog;
    dialog.fromSettings(cks);
    if(dialog.exec() == QDialog::Accepted) {
        dialog.toSettings(cks);
        saveSettings_calKit();
    }
}

extern "C" int __init_xavna_mock();
extern map<string, xavna_constructor> xavna_virtual_devices;
void MainWindow::on_actionMock_device_triggered()
{
    if(xavna_virtual_devices.find("mock") == xavna_virtual_devices.end())
        __init_xavna_mock();
    openDevice("mock");
}

void MainWindow::on_menuDevice_aboutToShow()
{
    populateDevicesMenu();
}

void MainWindow::on_actionTime_to_fault_toggled( bool arg1 )
{
    if(arg1) dtf.show();
    else dtf.hide();
}

void MainWindow::on_actionGraph_limits_triggered()
{
    QDialog dialog;
    {
        Ui::GraphLimitsDialog ui1;
        ui1.setupUi(&dialog);
        auto& limits = nv.graphLimits.at(SParamViewSource::TYPE_MAG);
        ui1.t_min->setText(qs(ssprintf(32, "%.0f", limits.at(0))));
        ui1.t_max->setText(qs(ssprintf(32, "%.0f", limits.at(1))));
        ui1.t_divs->setText(qs(ssprintf(32, "%.0f", limits.at(2))));
        if(dialog.exec() == QDialog::Accepted) {
            limits.at(0) = atoi(ui1.t_min->text().toStdString().c_str());
            limits.at(1) = atoi(ui1.t_max->text().toStdString().c_str());
            limits.at(2) = atoi(ui1.t_divs->text().toStdString().c_str());
            nv.updateYAxis();
        }
    }
}

void MainWindow::on_actionPort_length_extension_toggled( bool arg1 )
{
    ui->dock_ext->setVisible( arg1 );
}

void MainWindow::on_slider_ext1_valueChanged( int )
{
    updatePortExtension();
}

void MainWindow::on_slider_ext2_valueChanged( int )
{
    updatePortExtension();
}

void MainWindow::on_dock_ext_visibilityChanged( bool visible )
{
    if(visible != ui->actionPort_length_extension->isChecked())
        ui->actionPort_length_extension->setChecked( visible );
}

void MainWindow::on_b_reset_ext_clicked()
{
    ui->slider_ext1->setValue(0);
    ui->slider_ext2->setValue(0);
    ui->slider_extz->setValue(5000);
}

void MainWindow::on_t_ext1_returnPressed()
{
    string txt = ui->t_ext1->text().toStdString();
    double lengthPs = atof(txt.c_str());
    ui->slider_ext1->setValue(int(lengthPs));
}

void MainWindow::on_t_ext2_returnPressed()
{
    string txt = ui->t_ext2->text().toStdString();
    double lengthPs = atof(txt.c_str());
    ui->slider_ext2->setValue(int(lengthPs));
}

void MainWindow::on_actionT_R_mode_toggled( bool checked )
{
    if(vna->isTR()) return;
    bool scanning = vna->isScanning();
    vna->stopScan();

    vna->forceTR = checked;
    if(viewIsTR != vna->isTRMode()) {
        destroyViews();
        setupViews();
    }

    if(scanning) vna->startScan();
}

void MainWindow::on_actionSwap_ports_toggled( bool checked )
{
    vna->swapPorts = checked;
}

void MainWindow::on_b_cal_help_clicked()
{
    int index = ui->d_caltype->currentIndex();
    auto* cal = calibrationTypes.at(index);
    QMessageBox::information(this,"Info",qs(cal->helpText()));
}

string serializeCSV( vector<Matrix2cd> data, double startFreqHz, double stepFreqHz )
{
    string res;
    res += "Freq (MHz),re(S11),im(S11),re(Z),im(Z)\n";
    for(int i=0;i<(int)data.size();i++) {
        double freqHz = startFreqHz + i*stepFreqHz;
        double freqMHz = freqHz*1e-6;
        complex<double> s11 = data[i](0,0);
        double z0 = 50.;
        complex<double> Z = -z0*(s11+1.)/(s11-1.);
        res += ssprintf(256,"%.3f,%.5f,%.5f,%.3f,%.3f\n",
                 freqMHz, s11.real(), s11.imag(), Z.real(),Z.imag());
    }
    return res;
}

void MainWindow::on_actionExport_csv_triggered()
{
    vector<VNACalibratedValue> res(vna->nPoints);
    struct tm tm1;
    time_t t = time(nullptr);
    tm1 = *localtime(&t);
    string defaultFilename = "impedance-" + sstrftime("%Y%m%d", tm1) + ".csv";

    QString fileName = fileDialogSave(
            tr("Save impedance parameters"),
            tr("CSV files (*.csv);;All Files (*)"),
            "csv",
            qs(defaultFilename));
    if (fileName.isEmpty()) return;

    enableUI(false);
    vna->takeMeasurement([this,fileName](const vector<VNARawValue>& vals) {
        assert(curCal != nullptr);
        tmp_sparams.resize(vna->nPoints);
        for(int i=0;i<vna->nPoints;i++)
            tmp_sparams.at(i) = curCal->computeValue(curCalCoeffs.at(i), vals.at(i));
        string data = serializeCSV(tmp_sparams,vna->startFreqHz,vna->stepFreqHz);
        saveFile(fileName, data);
        QMetaObject::invokeMethod(this, "sMeasurementCompleted", Qt::QueuedConnection);
    });
}

void MainWindow::on_actionFine_tune_triggered()
{
    if(curCal == nullptr) {
        QMessageBox::critical(this, "Error", "No calibration active. Please perform a calibration first.");
        return;
    }
    CalibrationFineTuneDialog dialog;
    dialog.init(curCal, curCalStdModelsArray, vna->startFreqHz, vna->stepFreqHz);
    dialog.modelsChanged = [&]() {
        for(int i=0; i<vna->nPoints; i++) {
            curCalCoeffs.at(i) = curCal->computeCoefficients(curCalMeasurementsArray.at(i), dialog.newModels.at(i));
        }
        updateViews(-1);
    };
    if(dialog.exec() == QDialog::Accepted) {
        curCalStdModelsArray = dialog.newModels;
        // update global cal kit parameters
        dialog.toSettings(cks);
        saveSettings_calKit();
    } else {
        // revert original models and re-apply calibration
        dialog.newModels = dialog.origModels;
        dialog.modelsChanged(); // this calls the lambda above
    }
}

void MainWindow::on_t_extz_returnPressed()
{
    string txt = ui->t_extz->text().toStdString();
    double Z = atof(txt.c_str());
    ui->slider_extz->setValue(int(round(Z*100)));
}

void MainWindow::on_slider_extz_valueChanged( int )
{
    updatePortExtension();
}


