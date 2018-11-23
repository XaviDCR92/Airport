/****************************************************************************
** Meta object code from reading C++ file 'mygraphicsscene.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../MapEditor/mygraphicsscene.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mygraphicsscene.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MyGraphicsScene_t {
    QByteArrayData data[5];
    char stringdata0[67];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MyGraphicsScene_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MyGraphicsScene_t qt_meta_stringdata_MyGraphicsScene = {
    {
QT_MOC_LITERAL(0, 0, 15), // "MyGraphicsScene"
QT_MOC_LITERAL(1, 16, 15), // "positionClicked"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 14), // "noItemSelected"
QT_MOC_LITERAL(4, 48, 18) // "updateSelectedItem"

    },
    "MyGraphicsScene\0positionClicked\0\0"
    "noItemSelected\0updateSelectedItem"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MyGraphicsScene[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       3,    0,   32,    2, 0x06 /* Public */,
       4,    0,   33,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QPointF,    2,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MyGraphicsScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MyGraphicsScene *_t = static_cast<MyGraphicsScene *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->positionClicked((*reinterpret_cast< QPointF(*)>(_a[1]))); break;
        case 1: _t->noItemSelected(); break;
        case 2: _t->updateSelectedItem(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MyGraphicsScene::*)(QPointF );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGraphicsScene::positionClicked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MyGraphicsScene::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGraphicsScene::noItemSelected)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MyGraphicsScene::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGraphicsScene::updateSelectedItem)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MyGraphicsScene::staticMetaObject = { {
    &QGraphicsScene::staticMetaObject,
    qt_meta_stringdata_MyGraphicsScene.data,
    qt_meta_data_MyGraphicsScene,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MyGraphicsScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyGraphicsScene::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MyGraphicsScene.stringdata0))
        return static_cast<void*>(this);
    return QGraphicsScene::qt_metacast(_clname);
}

int MyGraphicsScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsScene::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void MyGraphicsScene::positionClicked(QPointF _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MyGraphicsScene::noItemSelected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void MyGraphicsScene::updateSelectedItem()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
