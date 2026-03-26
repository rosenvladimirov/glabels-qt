/****************************************************************************
** Meta object code from reading C++ file 'Model.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../model/Model.hpp"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Model.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN7glabels5model5ModelE_t {};
} // unnamed namespace

template <> constexpr inline auto glabels::model::Model::qt_create_metaobjectdata<qt_meta_tag_ZN7glabels5model5ModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "glabels::model::Model",
        "changed",
        "",
        "nameChanged",
        "sizeChanged",
        "selectionChanged",
        "modifiedChanged",
        "variablesChanged",
        "mergeChanged",
        "mergeSourceChanged",
        "mergeSelectionChanged",
        "onObjectChanged",
        "onObjectMoved",
        "onVariablesChanged",
        "onMergeSourceChanged",
        "onMergeSelectionChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'changed'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'nameChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'sizeChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'selectionChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'modifiedChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'variablesChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mergeChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mergeSourceChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mergeSelectionChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onObjectChanged'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onObjectMoved'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onVariablesChanged'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMergeSourceChanged'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMergeSelectionChanged'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Model, qt_meta_tag_ZN7glabels5model5ModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject glabels::model::Model::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7glabels5model5ModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7glabels5model5ModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7glabels5model5ModelE_t>.metaTypes,
    nullptr
} };

void glabels::model::Model::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Model *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->changed(); break;
        case 1: _t->nameChanged(); break;
        case 2: _t->sizeChanged(); break;
        case 3: _t->selectionChanged(); break;
        case 4: _t->modifiedChanged(); break;
        case 5: _t->variablesChanged(); break;
        case 6: _t->mergeChanged(); break;
        case 7: _t->mergeSourceChanged(); break;
        case 8: _t->mergeSelectionChanged(); break;
        case 9: _t->onObjectChanged(); break;
        case 10: _t->onObjectMoved(); break;
        case 11: _t->onVariablesChanged(); break;
        case 12: _t->onMergeSourceChanged(); break;
        case 13: _t->onMergeSelectionChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Model::*)()>(_a, &Model::changed, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Model::*)()>(_a, &Model::nameChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Model::*)()>(_a, &Model::sizeChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Model::*)()>(_a, &Model::selectionChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Model::*)()>(_a, &Model::modifiedChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (Model::*)()>(_a, &Model::variablesChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (Model::*)()>(_a, &Model::mergeChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (Model::*)()>(_a, &Model::mergeSourceChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (Model::*)()>(_a, &Model::mergeSelectionChanged, 8))
            return;
    }
}

const QMetaObject *glabels::model::Model::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *glabels::model::Model::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7glabels5model5ModelE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int glabels::model::Model::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void glabels::model::Model::changed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void glabels::model::Model::nameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void glabels::model::Model::sizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void glabels::model::Model::selectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void glabels::model::Model::modifiedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void glabels::model::Model::variablesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void glabels::model::Model::mergeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void glabels::model::Model::mergeSourceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void glabels::model::Model::mergeSelectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}
QT_WARNING_POP
