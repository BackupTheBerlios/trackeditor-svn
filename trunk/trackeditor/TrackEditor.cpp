/****************************************************************************
 **
 ** Trolltech hereby grants a license to use the Qt/Eclipse Integration
 ** plug-in (the software contained herein), in binary form, solely for the
 ** purpose of creating code to be used with Trolltech's Qt software.
 **
 ** Qt Designer is licensed under the terms of the GNU General Public
 ** License versions 2.0 and 3.0 ("GPL License"). Trolltech offers users the
 ** right to use certain no GPL licensed software under the terms of its GPL
 ** Exception version 1.2 (http://trolltech.com/products/qt/gplexception).
 **
 ** THIS SOFTWARE IS PROVIDED BY TROLLTECH AND ITS CONTRIBUTORS (IF ANY) "AS
 ** IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 ** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 ** PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 ** OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 ** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 ** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 ** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 ** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 ** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 ** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** Since we now have the GPL exception I think that the "special exception
 ** is no longer needed. The license text proposed above (other than the
 ** special exception portion of it) is the BSD license and we have added
 ** the BSD license as a permissible license under the exception.
 **
 ****************************************************************************/

#include <sys/stat.h>
#include <fcntl.h>

#include <qapplication.h>
#include <QSocketNotifier>
#include <QDebug>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QList>

#include "TrackEditor.h"
#include "DeviceData.h"
#include "TrackView.h"
#include "tk1File.h"
#include "gpxFile.h"
#include "CWintec.h"
#include "plotWidget.h"
#include "CDiagramsLayout.h"
#include "CSettings.h"
#include "CSerialPortSettings.h"
#include "qextserialport/qextserialport.h"
#include "CMarker.h"
#include "CAboutDialog.h"

#include "CDeviceDialog.h"
#include "csettingsdlg.h"

TrackEditor::TrackEditor(QWidget *parent) :
        QMainWindow(parent),
        // m_idev_factory(),
        m_serial_port(0),
        m_device_io(0),
        m_dev_data(0),
        m_command_mode_step(-1),
        m_command_response_step(-1),
        m_device_file(0),
        m_socket_notifier(0),
        m_nema_string(""),
        m_line(""),
        m_log_buf(),
        m_tmp_buf(),
        m_read_start(-1),
        m_retry_count(-1),
        m_expect_binary_data(-1),
        m_binary_data_already_read(-1),
        m_lastsection(false),
        m_blocksize(-1),
        m_track_collection(0),
        m_selection_model(0),
        m_track_filename("")
{
    ui.setupUi(this);

        // set m_track_collection to 0 to prevent setTrackCollection() from trying to delete it.
	m_track_collection = 0;

	PlotData::initializeMaps();

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

	connect(ui.action_Connect, SIGNAL(triggered()), this, SLOT(connectDevice()));
	connect(ui.action_Disconnect, SIGNAL(triggered()), this, SLOT(disconnectDevice()));

	connect(ui.action_Load_Track, SIGNAL(triggered()), this, SLOT(loadTrack()));
	connect(ui.actionAppend_Track, SIGNAL(triggered()), this, SLOT(appendTrack()));
	connect(ui.action_Save_Track, SIGNAL(triggered()), this, SLOT(saveTrack()));
	connect(ui.action_Save_Track_As, SIGNAL(triggered()), this, SLOT(saveTrackAs()));

	connect(ui.action_Read_Log, SIGNAL(triggered()), this, SLOT(readLog()));
	ui.action_Read_Log->setDisabled(true);
	connect(ui.action_Start_Recording, SIGNAL(triggered()), this, SLOT(startRecording()));
	connect(ui.action_Stop_Recording, SIGNAL(triggered()), this, SLOT(stopRecording()));

	connect(ui.actionStart_Animation, SIGNAL(triggered()), &m_animation, SLOT(start()));
	connect(ui.actionStop_Animation, SIGNAL(triggered()), &m_animation, SLOT(stop()));

	connect(ui.actionFaster, SIGNAL(triggered()), &m_animation, SLOT(incSpeed()));
	connect(ui.actionSlower, SIGNAL(triggered()), &m_animation, SLOT(decSpeed()));

	connect(ui.actionX_0_125, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX0125()));
	connect(ui.actionX_0_25, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX025()));
	connect(ui.actionX_0_5, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX05()));
	connect(ui.actionX_1, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX1()));
	connect(ui.actionX_2, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX2()));
	connect(ui.actionX_4, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX4()));
	connect(ui.actionX_8, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX8()));
	connect(ui.actionX_16, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX16()));
	connect(ui.actionX_32, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX32()));
	connect(ui.actionX_64, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX64()));
	connect(ui.actionX_128, SIGNAL(triggered()), &m_animation, SLOT(setTimeScaleX128()));


	connect(ui.actionSettings, SIGNAL(triggered()), this, SLOT(showSettingsDlg()));

	connect(ui.action_About, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

	connect(this, SIGNAL(setText(QString)), ui.nemaText, SLOT(appendPlainText(QString)));


	m_track_view = new TrackView(ui.scrollArea);
	ui.scrollArea->setWidget(m_track_view);

	connect(&m_animation, SIGNAL(setMarkers(QList<CMarker>)), m_track_view, SLOT(setMarkers(QList<CMarker>)));

	connect(ui.actionZoom_in, SIGNAL(triggered()), m_track_view, SLOT(zoomIn()));
	connect(ui.actionZoom_out, SIGNAL(triggered()), m_track_view, SLOT(zoomOut()));

	ui.treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_selection_model = ui.treeView->selectionModel();
    connect(m_selection_model, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged(QItemSelection,QItemSelection)));


	m_diagrams_layout = new CDiagramsLayout(ui.diagramWidget);
	ui.diagramWidget->setLayout(m_diagrams_layout);

	connect(m_diagrams_layout, SIGNAL(setMarkers(QList<CMarker>)), m_track_view, SLOT(setMarkers(QList<CMarker>)));

	m_settings = new CSettings();
	m_settings->load();

	QList<enum plotTypeY> distList;
	distList = m_settings->getDistQuantities();

	QList<enum plotTypeY> timeList;
	timeList = m_settings->getTimeQuantities();

	QList<enum plotTypeY> trackPointsList;
	trackPointsList = m_settings->getTrackpointsQuantities();

	m_diagrams_layout->setQuantities(distList, timeList, trackPointsList );


	setTrackCollection(new TrackCollection);

    m_device_io = 0;
	m_dev_data = 0;
	m_expect_binary_data = 0;

	m_command_mode_step = 0;
	m_command_response_step = 0;

	m_track_filename.clear();

    connect(ui.treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(treeViewClicked(QModelIndex)));

    ui.treeView->setEditTriggers( QAbstractItemView::DoubleClicked
								| QAbstractItemView::SelectedClicked
								| QAbstractItemView::EditKeyPressed );


    m_progress_dlg = new QDialog(this);
    prg_dlg.setupUi(m_progress_dlg);
    m_progress_dlg->setModal(false);
    connect(prg_dlg.cancelButton, SIGNAL(clicked()), this, SLOT(readLogFinished()));

	statusBar()->addWidget(m_animation.statusBarWidget());
	statusBar()->addPermanentWidget(m_track_view->statusBarWidget());

	restoreLayout();
}

TrackEditor::~TrackEditor() {

	if(m_device_io != 0)
	{
		delete m_device_io;
	}
	ui.scrollArea->setWidget(0);

	delete m_settings;
	delete m_track_view;
	delete m_track_collection;
	closeTTY();
	delete m_progress_dlg;
}

void TrackEditor::connectDevice() {
	//        QDialog *devdlg = new QDialog(this);
	//        Ui::DeviceDialog dlg;
	//        dlg.setupUi(devdlg);

	CDeviceDialog *devdlg = new CDeviceDialog(this);
	devdlg->setModal(true);
	int retval = devdlg->exec();

	if(retval == QDialog::Accepted) {
		qDebug("OK pressed.");
		CSerialPortSettings settings = devdlg->getPortSettings();
		m_serial_port = new QextSerialPort(settings.getName(), settings);
		bool res = m_serial_port->open( QIODevice::ReadWrite );
		connect(m_serial_port, SIGNAL(readyRead()), this, SLOT(readDevice()));

		// openTTY("/dev/rfcomm0", 115200);
		m_device_io = new CWintec("WBT201");
		connect(this, SIGNAL(emitData(QByteArray)), m_device_io, SLOT(addData(QByteArray)));
		connect(m_device_io, SIGNAL(sendData(QByteArray)), this, SLOT(sendData(QByteArray)));
		connect(m_device_io, SIGNAL(nemaString(QString)), ui.nemaText, SLOT(appendPlainText(QString)));

		connect(m_device_io, SIGNAL(progress(int)), this, SLOT(progress(int)));

		connect(m_device_io, SIGNAL(readLogFinished()), this, SLOT(readLogFinished()));

		connect(m_device_io, SIGNAL(newTrack(Track*)), this, SLOT(newTrack(Track*)));
		connect(m_device_io, SIGNAL(newWayPoint(TrackPoint*)), this, SLOT(newWayPoint(TrackPoint*)));
		connect(m_device_io, SIGNAL(newLogPoint(TrackPoint*)), this, SLOT(newLogPoint(TrackPoint*)));

		// readDevice();

		ui.action_Read_Log->setDisabled(false);
	}
}

void TrackEditor::openTTY(const char* name, int speed) {

	m_device_fd = open(name, O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC);
	m_device_file = new QFile();

	bool openres = m_device_file->open(m_device_fd, QIODevice::ReadWrite);
	if (m_device_fd == -1 || openres == false)
	{
		qDebug("failed to open device %s with speed %d.", name, speed);
	}
	else
	{

		// int fd,c, res;
		struct termios newtio;

		tcgetattr(m_device_fd, &m_oldtio); /* save current port settings */

		bzero(&newtio, sizeof(newtio));
		newtio.c_cflag = CRTSCTS | CS8 | CLOCAL | CREAD;
		newtio.c_iflag = IGNPAR;
		newtio.c_oflag = 0;

		/* set input mode (non-canonical, no echo,...) */
		newtio.c_lflag = 0;

		newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
		newtio.c_cc[VMIN] = 5; /* blocking read until 5 chars received */

		tcflush(m_device_fd, TCIFLUSH);
		tcsetattr(m_device_fd, TCSANOW, &newtio);

		m_socket_notifier = new QSocketNotifier(m_device_fd, QSocketNotifier::Read);
		connect(m_socket_notifier, SIGNAL(activated(int)), this, SLOT(readDevice(int)));
		readDevice(m_device_fd);
	}
}

void TrackEditor::closeTTY()
{

	if(m_device_file != 0)
	{
		m_device_file->close();
		delete m_device_file;
		m_device_file = 0;
	}
	disconnect(this, SLOT(readDevice(int)) );
	delete m_socket_notifier;
	m_socket_notifier = 0;

	if(m_device_fd != -1)
	{
		tcsetattr(m_device_fd, TCSANOW, &m_oldtio);
		::close(m_device_fd);
		m_device_fd = -1;
	}
}

void TrackEditor::readDevice()
{
//	qDebug() << QString("TrackEditor::readDevice()");
	QByteArray data = m_serial_port->read(m_serial_port->bytesAvailable());
//	qDebug() << data;
	emit emitData(data);
}


void TrackEditor::readDevice(int dev_fd)
{
	static char buffer[4096 + 1];
	int bytes_read;
	if (dev_fd == m_device_fd)
	{
		bytes_read = read(m_device_fd, buffer, 4096);
		QByteArray data(buffer, bytes_read);
		emit emitData(data);
	}
}


void TrackEditor::sendData(QByteArray data)
{
	if(m_device_fd != -1)
	{
		//write(m_device_fd, data.data(), data.size());
		m_serial_port->write(data);
	}
}


void TrackEditor::disconnectDevice()
{
	if(m_device_io != 0) {
		disconnect(m_device_io, SLOT(addData(QByteArray)));
		disconnect(m_device_io, SIGNAL(sendData(QByteArray)), this, SLOT(sendData(QByteArray)));
		disconnect(m_device_io, SIGNAL(nemaString(QString)), ui.nemaText, SLOT(appendPlainText(QString)));

		disconnect(m_device_io, SIGNAL(progress(int)), this, SLOT(progress(int)));

		disconnect(m_device_io, SIGNAL(readLogFinished()), this, SLOT(readLogFinished()));

		disconnect(m_device_io, SIGNAL(newTrack(Track*)), this, SLOT(newTrack(Track*)));
		disconnect(m_device_io, SIGNAL(newWayPoint(TrackPoint*)), this, SLOT(newWayPoint(TrackPoint*)));
		disconnect(m_device_io, SIGNAL(newLogPoint(TrackPoint*)), this, SLOT(newLogPoint(TrackPoint*)));

		ui.action_Read_Log->setDisabled(true);

		delete m_device_io;
		m_device_io = 0;

		disconnect(m_serial_port, SIGNAL(readyRead ()), this, SLOT(readDevice()));
		m_serial_port->close();
		delete m_serial_port;
		m_serial_port = 0;

		//closeTTY();

	}
}

void TrackEditor::readLog() {

	if(m_device_io == 0) {
		connectDevice();
	}
	if(m_device_io == 0) {
		return;
	}

	progress(0);
	m_progress_dlg->show();

	connect(m_progress_dlg, SIGNAL(cancelReadLog()), this, SLOT(cancelReadLog()));
	m_device_io->readLog();
}

void TrackEditor::progress(int percent) {
	prg_dlg.progressBar->setValue(percent);
}

void TrackEditor::cancelReadLog() {
	m_device_io->cancelReadLog();
	readLogFinished();
}

void TrackEditor::readLogFinished() {
	disconnect(m_progress_dlg, SIGNAL(cancelReadLog()), this, SLOT(cancelReadLog()));
	m_progress_dlg->close();
}


void TrackEditor::setTrackCollection(TrackCollection* track_collection) {
//	m_track_collection.clear();

	TrackCollection* tmp_tc = m_track_collection;
	m_track_collection = track_collection;
	m_track_collection->setDiagramQuantities(m_settings->getDistQuantities(), m_settings->getTimeQuantities(), m_settings->getTrackpointsQuantities());

	m_track_view->setTrackColletcion(m_track_collection);
    ui.treeView->setModel(m_track_collection);

	ui.treeView->resizeColumnToContents(0);
	ui.treeView->resizeColumnToContents(1);
	ui.treeView->resizeColumnToContents(2);
	ui.treeView->resizeColumnToContents(3);
	ui.treeView->resizeColumnToContents(4);

	if(tmp_tc != 0) {
		delete tmp_tc;
	}

    disconnect(m_selection_model, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
    m_selection_model = ui.treeView->selectionModel();
    connect(m_selection_model, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged(QItemSelection,QItemSelection)));

    m_animation.setTrackCollection(m_track_collection);
	m_track_view->update();

}

void TrackEditor::newTrack(Track* track)
{
	track->setIndex(m_track_collection->size());
	track->setDiagramQuantities(m_settings->getDistQuantities(), m_settings->getTimeQuantities(), m_settings->getTrackpointsQuantities() );
	m_track_collection->appendTrack(track);

	ui.treeView->resizeColumnToContents(0);
	ui.treeView->resizeColumnToContents(1);
	ui.treeView->resizeColumnToContents(2);
	ui.treeView->resizeColumnToContents(3);
	ui.treeView->resizeColumnToContents(4);

	prg_dlg.m_num_tracks->setText(QString().number(m_track_collection->size()));

	m_track_view->update();
}

void TrackEditor::newWayPoint(TrackPoint* tp)
{

}

void TrackEditor::newLogPoint(TrackPoint* tp)
{

	prg_dlg.m_num_logpoints->setText(QString().number(m_track_collection->getNumWaypoints()));

}

void TrackEditor::startRecording() {

}

void TrackEditor::stopRecording() {

}


void TrackEditor::showSettingsDlg()
{
	CSettingsDlg dlg(this);
	dlg.exec();
}

//void TrackEditor::setDiagramQuantities(QStringList distVals, QStringList timeVals, QStringList trackPointVals) {
void TrackEditor::setDiagramQuantities(QList<enum plotTypeY> distVals, QList<enum plotTypeY> timeVals, QList<enum plotTypeY> trackPointVals) {
	//m_distVals = distVals;
	//m_timeVals = timeVals;
	//m_trackPointVals = trackPointVals;

	m_track_collection->setDiagramQuantities(distVals, timeVals, trackPointVals);
	m_diagrams_layout->setQuantities(distVals, timeVals, trackPointVals);
	m_settings->setDistQuantities(distVals);
	m_settings->setTimeQuantities(timeVals);
	m_settings->setTrackpointsQuantities(trackPointVals);
	m_settings->save();
}


void TrackEditor::actionTriggered() {
	openTTY("/dev/rfcomm0", 115200);
	emit setText(QString("Hallo"));
	qDebug("action triggered");
}

void TrackEditor::treeViewClicked(QModelIndex index) {
	// m_diagrams_layout->setTrack(m_track_collection->at(index.row()));
	// m_plotWidget->setTrack(m_track_collection->at(index.row()));
	qDebug() << QString("treeViewClicked trackNr: %1 Column %2").arg(index.row()).arg(index.column());
	if(index.column() == 1) {
		// clicked on the color field
		QColor color = QColorDialog::getColor(m_track_collection->at(index.row())->getColor());
		if(color.isValid())
		{
			m_track_collection->at(index.row())->setColor(color);
		}
	}
}

void TrackEditor::selectionChanged(QItemSelection selected, QItemSelection deselected) {
	qDebug() << QString("selction changed");
	QModelIndexList selected_indices = m_selection_model->selectedIndexes();
    m_track_collection->setModelIndexList(selected_indices);

    m_diagrams_layout->setTracks(m_track_collection->getSelectedTracks());

    m_track_view->setTrackColletcion(m_track_collection);
}


void TrackEditor::markerChanged(double lat, double lng) {

}

void TrackEditor::markerChangedDist(double dist) {

}

void TrackEditor::markerChangedTime(double time) {

}

void TrackEditor::markerChangedTrackPoint(double tp_index) {

}



void TrackEditor::parseNEMA(QString line) {
	qDebug() << line;
}

void TrackEditor::loadTrack() {
	FILE* fd;
	char buffer[32];
    int bytes_read = 0;

    QSettings settings;
    QString startDir = settings.value("trackdir").toString();

    QString filename( QFileDialog::getOpenFileName( this,  tr("Open Track"), startDir,  tr("GPX Tracks (*.gpx);;Google Earth Tracks (*.kml *.kmz);;Wintec TK1 Tracks (*.tk1);;Raw Wintec Tracklogs (*.tkraw)")) );
    if ( filename.isEmpty() )
        return;

    QFileInfo fi(filename);
    settings.setValue("trackdir", fi.path());

	if(filename.endsWith(".gpx", Qt::CaseInsensitive)) {
		// load from GPX file
		gpxFile gpx_file;
		TrackCollection* tc = gpx_file.read(filename);
		setTrackCollection(tc);
		return;
	}
	else if(m_track_filename.endsWith(".kml", Qt::CaseInsensitive)) {
		//kmlFile kml_file;
		//kml_file.read(m_track_collection, filename);
		// load from kml file

	}
	else if(m_track_filename.endsWith(".kmz", Qt::CaseInsensitive)) {
		// load from kmz file

	}
	else if(m_track_filename.endsWith(".tk1", Qt::CaseInsensitive)) {
		tk1File tk1_file;
		m_track_collection = tk1_file.read(filename);
		// load from as tk1 file

	}
	else {
		 // QMessageBox::warning(this, tr("Track Editor"),
		 //                          tr("Unknown file type. TrackEditor is only\n"
		 //                            "able to load raw tk1 files."),
		 //                           QMessageBox::Discard);
	}

	m_log_buf.clear();
//	fd = fopen(filename.toLatin1(), "rb");
//	do {
//		bytes_read = fread(buffer, TrackPoint::size(), 1, fd);
//		m_log_buf.append(QByteArray(buffer,TrackPoint::size()));
//	} while( bytes_read != 0);
//	fclose(fd);

	//createTrackpoints();
}


void TrackEditor::appendTrack()
{

    QSettings settings;
    QString startDir = settings.value("trackdir").toString();

	QString filename( QFileDialog::getOpenFileName( this,  tr("Append Track"), startDir,  tr("GPX Tracks (*.gpx);;Google Earth Tracks (*.kml *.kmz);;Wintec TK1 Tracks (*.tk1);;Raw Wintec Tracklogs (*.tkraw)")) );
    if ( filename.isEmpty() )
        return;

    QFileInfo fi(filename);
    settings.setValue("trackdir", fi.path());

	if(filename.endsWith(".gpx", Qt::CaseInsensitive)) {
		// load from GPX file
		gpxFile gpx_file;
		TrackCollection* tc = gpx_file.read(filename);
		tc->setDiagramQuantities(m_settings->getDistQuantities(), m_settings->getTimeQuantities(), m_settings->getTrackpointsQuantities() );
		m_track_collection->appendTrackCollection(tc);
		return;
	}
	else if(m_track_filename.endsWith(".kml", Qt::CaseInsensitive)) {
		//kmlFile kml_file;
		//kml_file.read(m_track_collection, filename);
		// load from kml file

	}
	else if(m_track_filename.endsWith(".kmz", Qt::CaseInsensitive)) {
		// load from kmz file

	}
	else if(m_track_filename.endsWith(".tk1", Qt::CaseInsensitive)) {
		tk1File tk1_file;
		m_track_collection = tk1_file.read(filename);
		// load from as tk1 file

	}
	else {
		 // QMessageBox::warning(this, tr("Track Editor"),
		 //                          tr("Unknown file type. TrackEditor is only\n"
		 //                            "able to load raw tk1 files."),
		 //                           QMessageBox::Discard);
	}


	ui.treeView->resizeColumnToContents(0);
	ui.treeView->resizeColumnToContents(1);
	ui.treeView->resizeColumnToContents(2);
	ui.treeView->resizeColumnToContents(3);
	ui.treeView->resizeColumnToContents(4);

}



void TrackEditor::saveTrack() {
	if(m_track_filename.isEmpty()) {
		saveTrackAs();
	}
	else {
		save();
	}
}

void TrackEditor::saveTrackAs() {

    QSettings settings;
    QString startDir = settings.value("trackdir").toString();

    m_track_filename = QFileDialog::getSaveFileName( this,  tr("Save Track As"),QString::null,  tr("GPX Tracks (*.gpx);;Google Earth Tracks (*.kml *.kmz);;Wintec TK1 Tracks (*.tk1)"));
    if ( m_track_filename.isEmpty() )
        return;

	QFileInfo fi(m_track_filename);
    settings.setValue("trackdir", fi.path());

    save();
}

void TrackEditor::save() {
	if(m_track_filename.endsWith(".gpx", Qt::CaseInsensitive)) {
		// save as GPX file
		gpxFile gpx_file;
		gpx_file.write(m_track_collection, m_track_filename);
	}
	else if(m_track_filename.endsWith(".kml", Qt::CaseInsensitive)) {
		//kmlFile kml_file;
		//kml_file.write(m_track_collection, m_track_filename);
		// save as kml file

	}
	else if(m_track_filename.endsWith(".kmz", Qt::CaseInsensitive)) {
		// save as kmz file

	}
	else if(m_track_filename.endsWith(".tk1", Qt::CaseInsensitive)) {
		tk1File tk1_file;
		tk1_file.write(m_track_collection, m_track_filename);
		// save as tk1 file

	}
	else {
		 QMessageBox::warning(this, tr("Track Editor"),
		                           tr("Unknown file type. TrackEditor is only\n"
		                              "able to save GPX, KML, KMZ and TK1 files."),
		                            QMessageBox::Discard);
	}


}


void TrackEditor::load(QString filename) {


}

void TrackEditor::showAboutDialog()
{
		CAboutDialog aboutDlg(this);
		aboutDlg.exec();
}


void TrackEditor::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("splitter1", ui.splitter->saveState());
    settings.setValue("splitter2", ui.splitter_2->saveState());

    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    QMainWindow::closeEvent(event);
}

void TrackEditor::restoreLayout()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    ui.splitter->restoreState(settings.value("splitter1").toByteArray());
    ui.splitter_2->restoreState(settings.value("splitter2").toByteArray());
}
