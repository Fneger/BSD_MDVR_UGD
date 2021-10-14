/****************************************************************************
** Meta object code from reading C++ file 'testserver.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../myserver/testserver.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'testserver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TestServer_t {
    QByteArrayData data[12];
    char stringdata0[99];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TestServer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TestServer_t qt_meta_stringdata_TestServer = {
    {
QT_MOC_LITERAL(0, 0, 10), // "TestServer"
QT_MOC_LITERAL(1, 11, 6), // "listen"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 4), // "port"
QT_MOC_LITERAL(4, 24, 5), // "close"
QT_MOC_LITERAL(5, 30, 6), // "newCon"
QT_MOC_LITERAL(6, 37, 15), // "QAsioTcpsocket*"
QT_MOC_LITERAL(7, 53, 6), // "socket"
QT_MOC_LITERAL(8, 60, 8), // "readData"
QT_MOC_LITERAL(9, 69, 4), // "data"
QT_MOC_LITERAL(10, 74, 16), // "some_data_sended"
QT_MOC_LITERAL(11, 91, 7) // "wsended"

    },
    "TestServer\0listen\0\0port\0close\0newCon\0"
    "QAsioTcpsocket*\0socket\0readData\0data\0"
    "some_data_sended\0wsended"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TestServer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x0a /* Public */,
       1,    0,   47,    2, 0x2a /* Public | MethodCloned */,
       4,    0,   48,    2, 0x0a /* Public */,
       5,    1,   49,    2, 0x09 /* Protected */,
       8,    1,   52,    2, 0x09 /* Protected */,
      10,    1,   55,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void, QMetaType::Short,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, QMetaType::QByteArray,    9,
    QMetaType::Void, QMetaType::LongLong,   11,

       0        // eod
};

void TestServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TestServer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->listen((*reinterpret_cast< qint16(*)>(_a[1]))); break;
        case 1: _t->listen(); break;
        case 2: _t->close(); break;
        case 3: _t->newCon((*reinterpret_cast< QAsioTcpsocket*(*)>(_a[1]))); break;
        case 4: _t->readData((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 5: _t->some_data_sended((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAsioTcpsocket* >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TestServer::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_TestServer.data,
    qt_meta_data_TestServer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TestServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TestServer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TestServer.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TestServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
struct qt_meta_stringdata_myClient_t {
    QByteArrayData data[7];
    char stringdata0[53];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_myClient_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_myClient_t qt_meta_stringdata_myClient = {
    {
QT_MOC_LITERAL(0, 0, 8), // "myClient"
QT_MOC_LITERAL(1, 9, 8), // "readData"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 4), // "data"
QT_MOC_LITERAL(4, 24, 10), // "erroString"
QT_MOC_LITERAL(5, 35, 4), // "erro"
QT_MOC_LITERAL(6, 40, 12) // "disConnected"

    },
    "myClient\0readData\0\0data\0erroString\0"
    "erro\0disConnected"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_myClient[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x09 /* Protected */,
       4,    1,   32,    2, 0x09 /* Protected */,
       6,    0,   35,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void, QMetaType::QByteArray,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void,

       0        // eod
};

void myClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<myClient *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->readData((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 1: _t->erroString((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->disConnected(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject myClient::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_myClient.data,
    qt_meta_data_myClient,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *myClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *myClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_myClient.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int myClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
