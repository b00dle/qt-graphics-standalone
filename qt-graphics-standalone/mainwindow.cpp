#include "mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QFile>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , view_()
    , menu_()
{
    view_ = new Tile::GraphicsView;

    menu_ = new QMenu(tr("File"));
    QAction* open_action = new QAction(tr("Open Project..."), this);
    open_action->setShortcut(QKeySequence(tr("Ctrl+o")));
    connect(open_action, SIGNAL(triggered()),
            this, SLOT(onOpenProject()));
    menu_->addAction(open_action);
    menuBar()->addMenu(menu_);

    setCentralWidget(view_);
}

MainWindow::~MainWindow()
{
    view_->deleteLater();
}

void MainWindow::onOpenProject()
{
    QString file_name = QFileDialog::getOpenFileName(
        this, tr("Open Project"),
        "",
        tr("JSON (*.json)")
    );

    if(file_name.size() > 0) {
        QFile json_file(file_name);

        // opening failed
        if(!json_file.open(QFile::ReadOnly)) {
            QMessageBox b;
            b.setText(tr("The selected file could not be opened."));
            b.setInformativeText(tr("Do you wish to select a different file?"));
            b.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            b.setDefaultButton(QMessageBox::Yes);
            if(b.exec() == QMessageBox::Yes)
                onOpenProject();
            else
                return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(json_file.readAll());

        // graphics view could not be set from json
        if(!view_->setFromJsonObject(doc.object())) {
            QMessageBox b;
            b.setText(tr("The selected file does not seem to contain valid project data."));
            b.setInformativeText(tr("Do you wish to select a different file?"));
            b.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            b.setDefaultButton(QMessageBox::Yes);
            if(b.exec() == QMessageBox::Yes)
                onOpenProject();
        }
    }
}
