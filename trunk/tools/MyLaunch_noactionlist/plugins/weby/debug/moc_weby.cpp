/****************************************************************************
** Meta object code from reading C++ file 'weby.h'
**
** Created: Fri Oct 23 17:39:13 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../weby.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'weby.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_WebyPlugin[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_WebyPlugin[] = {
    "WebyPlugin\0"
};

const QMetaObject WebyPlugin::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_WebyPlugin,
      qt_meta_data_WebyPlugin, 0 }
};

const QMetaObject *WebyPlugin::metaObject() const
{
    return &staticMetaObject;
}

void *WebyPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WebyPlugin))
        return static_cast<void*>(const_cast< WebyPlugin*>(this));
    if (!strcmp(_clname, "PluginInterface"))
        return static_cast< PluginInterface*>(const_cast< WebyPlugin*>(this));
    if (!strcmp(_clname, "net.launchy.PluginInterface/1.0"))
        return static_cast< PluginInterface*>(const_cast< WebyPlugin*>(this));
    return QObject::qt_metacast(_clname);
}

int WebyPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
