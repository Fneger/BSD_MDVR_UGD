/****************************************************************************
** Meta object code from reading C++ file 'qasiotcpsocket.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qasiotcpsocket.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qasiotcpsocket.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QAsioTcpsocket_t {
    QByteArrayData data[17];
    char stringdata0[218];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QAsioTcpsocket_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QAsioTcpsocket_t qt_meta_stringdata_QAsioTcpsocket = {
    {
QT_MOC_LITERAL(0, 0, 14), // "QAsioTcpsocket"
QT_MOC_LITERAL(1, 15, 13), // "connectToHost"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 8), // "hostName"
QT_MOC_LITERAL(4, 39, 4), // "port"
QT_MOC_LITERAL(5, 44, 18), // "disconnectFromHost"
QT_MOC_LITERAL(6, 63, 15), // "setSocketOption"
QT_MOC_LITERAL(7, 79, 31), // "TcpAbstractSocket::SocketOption"
QT_MOC_LITERAL(8, 111, 6), // "option"
QT_MOC_LITERAL(9, 118, 7), // "isEnble"
QT_MOC_LITERAL(10, 126, 5), // "value"
QT_MOC_LITERAL(11, 132, 15), // "getSocketOption"
QT_MOC_LITERAL(12, 148, 19), // "std::pair<bool,int>"
QT_MOC_LITERAL(13, 168, 8), // "opention"
QT_MOC_LITERAL(14, 177, 8), // "do_start"
QT_MOC_LITERAL(15, 186, 26), // "resizeClientBackThreadSize"
QT_MOC_LITERAL(16, 213, 4) // "size"

    },
    "QAsioTcpsocket\0connectToHost\0\0hostName\0"
    "port\0disconnectFromHost\0setSocketOption\0"
    "TcpAbstractSocket::SocketOption\0option\0"
    "isEnble\0value\0getSocketOption\0"
    "std::pair<bool,int>\0opention\0do_start\0"
    "resizeClientBackThreadSize\0size"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QAsioTcpsocket[] = {

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
       1,    2,   44,    2, 0x0a /* Public */,
       5,    0,   49,    2, 0x0a /* Public */,
       6,    3,   50,    2, 0x0a /* Public */,
      11,    1,   57,    2, 0x0a /* Public */,
      14,    0,   60,    2, 0x0a /* Public */,
      15,    1,   61,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::UShort,    3,    4,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7, QMetaType::Bool, QMetaType::UInt,    8,    9,   10,
    0x80000000 | 12, 0x80000000 | 7,   13,
    QMetaType::Void,
    QMetaType::Int, QMetaType::Int,   16,

       0        // eod
};

void QAsioTcpsocket::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QAsioTcpsocket *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connectToHost((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 1: _t->disconnectFromHost(); break;
        case 2: _t->setSocketOption((*reinterpret_cast< TcpAbstractSocket::SocketOption(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< uint(*)>(_a[3]))); break;
        case 3: { std::pair<bool,int> _r = _t->getSocketOption((*reinterpret_cast< TcpAbstractSocket::SocketOption(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< std::pair<bool,int>*>(_a[0]) = std::move(_r); }  break;
        case 4: _t->do_start(); break;
        case 5: { int _r = _t->resizeClientBackThreadSize((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject QAsioTcpsocket::staticMetaObject = { {
    &TcpAbstractSocket::staticMetaObject,
    qt_meta_stringdata_QAsioTcpsocket.data,
    qt_meta_data_QAsioTcpsocket,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QAsioTcpsocket::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QAsioTcpsocket::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QAsioTcpsocket.stringdata0))
        return static_cast<void*>(this);
    return TcpAbstractSocket::qt_metacast(_clname);
}

int QAsioTcpsocket::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = TcpAbstractSocket::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
