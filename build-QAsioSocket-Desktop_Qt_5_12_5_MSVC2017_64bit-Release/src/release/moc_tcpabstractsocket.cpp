/****************************************************************************
** Meta object code from reading C++ file 'tcpabstractsocket.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../QAsioSocket/src/tcpabstractsocket.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tcpabstractsocket.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TcpAbstractSocket_t {
    QByteArrayData data[28];
    char stringdata0[311];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TcpAbstractSocket_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TcpAbstractSocket_t qt_meta_stringdata_TcpAbstractSocket = {
    {
QT_MOC_LITERAL(0, 0, 17), // "TcpAbstractSocket"
QT_MOC_LITERAL(1, 18, 9), // "connected"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 12), // "disConnected"
QT_MOC_LITERAL(4, 42, 11), // "stateChange"
QT_MOC_LITERAL(5, 54, 11), // "SocketState"
QT_MOC_LITERAL(6, 66, 5), // "state"
QT_MOC_LITERAL(7, 72, 12), // "sentReadData"
QT_MOC_LITERAL(8, 85, 4), // "data"
QT_MOC_LITERAL(9, 90, 10), // "erroString"
QT_MOC_LITERAL(10, 101, 4), // "erro"
QT_MOC_LITERAL(11, 106, 10), // "hostFinded"
QT_MOC_LITERAL(12, 117, 12), // "bytesWritten"
QT_MOC_LITERAL(13, 130, 5), // "bytes"
QT_MOC_LITERAL(14, 136, 13), // "connectToHost"
QT_MOC_LITERAL(15, 150, 8), // "hostName"
QT_MOC_LITERAL(16, 159, 4), // "port"
QT_MOC_LITERAL(17, 164, 18), // "disconnectFromHost"
QT_MOC_LITERAL(18, 183, 5), // "write"
QT_MOC_LITERAL(19, 189, 11), // "const char*"
QT_MOC_LITERAL(20, 201, 4), // "size"
QT_MOC_LITERAL(21, 206, 15), // "setSocketOption"
QT_MOC_LITERAL(22, 222, 31), // "TcpAbstractSocket::SocketOption"
QT_MOC_LITERAL(23, 254, 6), // "option"
QT_MOC_LITERAL(24, 261, 7), // "isEnble"
QT_MOC_LITERAL(25, 269, 5), // "value"
QT_MOC_LITERAL(26, 275, 15), // "getSocketOption"
QT_MOC_LITERAL(27, 291, 19) // "std::pair<bool,int>"

    },
    "TcpAbstractSocket\0connected\0\0disConnected\0"
    "stateChange\0SocketState\0state\0"
    "sentReadData\0data\0erroString\0erro\0"
    "hostFinded\0bytesWritten\0bytes\0"
    "connectToHost\0hostName\0port\0"
    "disconnectFromHost\0write\0const char*\0"
    "size\0setSocketOption\0"
    "TcpAbstractSocket::SocketOption\0option\0"
    "isEnble\0value\0getSocketOption\0"
    "std::pair<bool,int>"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TcpAbstractSocket[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x06 /* Public */,
       3,    0,   95,    2, 0x06 /* Public */,
       4,    1,   96,    2, 0x06 /* Public */,
       7,    1,   99,    2, 0x06 /* Public */,
       9,    1,  102,    2, 0x06 /* Public */,
      11,    0,  105,    2, 0x06 /* Public */,
      12,    1,  106,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      14,    2,  109,    2, 0x0a /* Public */,
      17,    0,  114,    2, 0x0a /* Public */,
      18,    1,  115,    2, 0x0a /* Public */,
      18,    2,  118,    2, 0x0a /* Public */,
      18,    1,  123,    2, 0x2a /* Public | MethodCloned */,
      21,    3,  126,    2, 0x0a /* Public */,
      26,    1,  133,    2, 0x0a /* Public */,
      21,    2,  136,    2, 0x0a /* Public */,
      21,    2,  141,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, QMetaType::QByteArray,    8,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   13,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UShort,   15,   16,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,    8,
    QMetaType::Void, 0x80000000 | 19, QMetaType::Int,    8,   20,
    QMetaType::Void, 0x80000000 | 19,    8,
    QMetaType::Void, 0x80000000 | 22, QMetaType::Bool, QMetaType::UInt,   23,   24,   25,
    0x80000000 | 27, 0x80000000 | 22,    2,
    QMetaType::Void, 0x80000000 | 22, QMetaType::Bool,   23,   24,
    QMetaType::Void, 0x80000000 | 22, QMetaType::UInt,   23,   25,

       0        // eod
};

void TcpAbstractSocket::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TcpAbstractSocket *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connected(); break;
        case 1: _t->disConnected(); break;
        case 2: _t->stateChange((*reinterpret_cast< SocketState(*)>(_a[1]))); break;
        case 3: _t->sentReadData((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 4: _t->erroString((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->hostFinded(); break;
        case 6: _t->bytesWritten((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 7: _t->connectToHost((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 8: _t->disconnectFromHost(); break;
        case 9: _t->write((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 10: _t->write((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 11: _t->write((*reinterpret_cast< const char*(*)>(_a[1]))); break;
        case 12: _t->setSocketOption((*reinterpret_cast< TcpAbstractSocket::SocketOption(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< uint(*)>(_a[3]))); break;
        case 13: { std::pair<bool,int> _r = _t->getSocketOption((*reinterpret_cast< TcpAbstractSocket::SocketOption(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< std::pair<bool,int>*>(_a[0]) = std::move(_r); }  break;
        case 14: _t->setSocketOption((*reinterpret_cast< TcpAbstractSocket::SocketOption(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 15: _t->setSocketOption((*reinterpret_cast< TcpAbstractSocket::SocketOption(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TcpAbstractSocket::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpAbstractSocket::connected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TcpAbstractSocket::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpAbstractSocket::disConnected)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TcpAbstractSocket::*)(SocketState );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpAbstractSocket::stateChange)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TcpAbstractSocket::*)(const QByteArray & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpAbstractSocket::sentReadData)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TcpAbstractSocket::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpAbstractSocket::erroString)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (TcpAbstractSocket::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpAbstractSocket::hostFinded)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (TcpAbstractSocket::*)(qint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TcpAbstractSocket::bytesWritten)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TcpAbstractSocket::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_TcpAbstractSocket.data,
    qt_meta_data_TcpAbstractSocket,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TcpAbstractSocket::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TcpAbstractSocket::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TcpAbstractSocket.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TcpAbstractSocket::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void TcpAbstractSocket::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void TcpAbstractSocket::disConnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TcpAbstractSocket::stateChange(SocketState _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void TcpAbstractSocket::sentReadData(const QByteArray & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void TcpAbstractSocket::erroString(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void TcpAbstractSocket::hostFinded()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void TcpAbstractSocket::bytesWritten(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
