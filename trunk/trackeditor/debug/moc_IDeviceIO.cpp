/****************************************************************************
** Meta object code from reading C++ file 'IDeviceIO.h'
**
** Created: Sun Sep 14 22:34:55 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../IDeviceIO.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'IDeviceIO.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IDeviceIO[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x05,
      39,   31,   10,   10, 0x05,
      53,   10,   10,   10, 0x05,
      70,   10,   10,   10, 0x05,
      95,   10,   10,   10, 0x05,
     120,   10,   10,   10, 0x05,
     141,   10,   10,   10, 0x05,
     159,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
     182,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_IDeviceIO[] = {
    "IDeviceIO\0\0nemaString(QString)\0percent\0"
    "progress(int)\0newTrack(Track*)\0"
    "newWayPoint(TrackPoint*)\0"
    "newLogPoint(TrackPoint*)\0sendData(QByteArray)\0"
    "readLogFinished()\0readLogFailed(QString)\0"
    "addData(QByteArray)\0"
};

const QMetaObject IDeviceIO::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_IDeviceIO,
      qt_meta_data_IDeviceIO, 0 }
};

const QMetaObject *IDeviceIO::metaObject() const
{
    return &staticMetaObject;
}

void *IDeviceIO::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IDeviceIO))
        return static_cast<void*>(const_cast< IDeviceIO*>(this));
    return QObject::qt_metacast(_clname);
}

int IDeviceIO::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: nemaString((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: progress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: newTrack((*reinterpret_cast< Track*(*)>(_a[1]))); break;
        case 3: newWayPoint((*reinterpret_cast< TrackPoint*(*)>(_a[1]))); break;
        case 4: newLogPoint((*reinterpret_cast< TrackPoint*(*)>(_a[1]))); break;
        case 5: sendData((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 6: readLogFinished(); break;
        case 7: readLogFailed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: addData((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        }
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void IDeviceIO::nemaString(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void IDeviceIO::progress(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void IDeviceIO::newTrack(Track * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void IDeviceIO::newWayPoint(TrackPoint * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void IDeviceIO::newLogPoint(TrackPoint * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void IDeviceIO::sendData(QByteArray _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void IDeviceIO::readLogFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 6, 0);
}

// SIGNAL 7
void IDeviceIO::readLogFailed(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_END_MOC_NAMESPACE
