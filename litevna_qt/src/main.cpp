
#include "mainwindow.hpp"
#include "calkitsettings.hpp"
#include <QApplication>
#include <QtPlugin>
#include <QTranslator>
#include <QLibraryInfo>
#include <QFile>

int main( int argc, char *argv[] )
{
    qRegisterMetaType<string>( "string" );
    qRegisterMetaType<CalKitSettings>( "CalKitSettings" );
    qRegisterMetaTypeStreamOperators<CalKitSettings>( "CalKitSettings" );

    QCoreApplication::setApplicationName( "LiteVNA QT GUI" );

    QApplication app( argc, argv );
    QFile file( ":/resources/style" );                            // Apply style sheet
    if( file.open( QIODevice::ReadOnly | QIODevice::Text ) ){
        app.setStyleSheet( file.readAll() );
        file.close();
    }


    MainWindow w;
    QIcon appIcon( ":/resources/icon" );
    w.setWindowIcon( appIcon );
    w.setWindowTitle( "LiteVNA QT v" VERSION " by gorbachaa" );
    w.show();

    return app.exec();
}
