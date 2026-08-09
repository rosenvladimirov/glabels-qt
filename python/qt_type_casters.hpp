#ifndef QT_TYPE_CASTERS_HPP
#define QT_TYPE_CASTERS_HPP

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QColor>
#include <QFlags>
#include <Qt>

namespace pybind11::detail {

// QString <-> Python str
template <>
struct type_caster<QString> {
public:
    PYBIND11_TYPE_CASTER(QString, const_name("str"));

    bool load(handle src, bool) {
        if (!src || src.is_none()) return false;
        PyObject* tmp = PyUnicode_AsUTF8String(src.ptr());
        if (!tmp) {
            PyErr_Clear();
            return false;
        }
        const char* utf8 = PyBytes_AsString(tmp);
        value = QString::fromUtf8(utf8);
        Py_DECREF(tmp);
        return true;
    }

    static handle cast(const QString& src, return_value_policy, handle) {
        QByteArray utf8 = src.toUtf8();
        return PyUnicode_FromStringAndSize(utf8.constData(), utf8.size());
    }
};


// QStringList <-> Python list[str]
template <>
struct type_caster<QStringList> {
public:
    PYBIND11_TYPE_CASTER(QStringList, const_name("list[str]"));

    bool load(handle src, bool) {
        if (!isinstance<sequence>(src)) return false;
        auto seq = reinterpret_borrow<sequence>(src);
        value.clear();
        for (size_t i = 0; i < seq.size(); i++) {
            value.append(seq[i].cast<QString>());
        }
        return true;
    }

    static handle cast(const QStringList& src, return_value_policy, handle) {
        list result;
        for (const auto& s : src)
            result.append(pybind11::cast(s));
        return result.release();
    }
};


// QByteArray <-> Python bytes
template <>
struct type_caster<QByteArray> {
public:
    PYBIND11_TYPE_CASTER(QByteArray, const_name("bytes"));

    bool load(handle src, bool) {
        if (isinstance<pybind11::bytes>(src)) {
            auto b = reinterpret_borrow<pybind11::bytes>(src);
            char* buffer = nullptr;
            Py_ssize_t length = 0;
            PyBytes_AsStringAndSize(b.ptr(), &buffer, &length);
            value = QByteArray(buffer, static_cast<int>(length));
            return true;
        }
        return false;
    }

    static handle cast(const QByteArray& src, return_value_policy, handle) {
        return PyBytes_FromStringAndSize(src.constData(), src.size());
    }
};


// QColor <-> Python tuple(r, g, b, a)
template <>
struct type_caster<QColor> {
public:
    PYBIND11_TYPE_CASTER(QColor, const_name("tuple[int, int, int, int]"));

    bool load(handle src, bool) {
        if (!isinstance<sequence>(src)) return false;
        auto seq = reinterpret_borrow<sequence>(src);
        if (seq.size() == 3) {
            value = QColor(seq[0].cast<int>(), seq[1].cast<int>(), seq[2].cast<int>());
            return true;
        }
        if (seq.size() == 4) {
            value = QColor(seq[0].cast<int>(), seq[1].cast<int>(),
                           seq[2].cast<int>(), seq[3].cast<int>());
            return true;
        }
        return false;
    }

    static handle cast(const QColor& src, return_value_policy, handle) {
        return make_tuple(src.red(), src.green(), src.blue(), src.alpha()).release();
    }
};


// QList<T> <-> Python list
template <typename T>
struct type_caster<QList<T>> {
public:
    using value_conv = make_caster<T>;
    PYBIND11_TYPE_CASTER(QList<T>, const_name("list[") + value_conv::name + const_name("]"));

    bool load(handle src, bool convert) {
        if (!isinstance<sequence>(src)) return false;
        auto seq = reinterpret_borrow<sequence>(src);
        value.clear();
        for (size_t i = 0; i < seq.size(); i++) {
            value_conv conv;
            if (!conv.load(seq[i], convert)) return false;
            value.append(cast_op<T&&>(std::move(conv)));
        }
        return true;
    }

    static handle cast(const QList<T>& src, return_value_policy policy, handle parent) {
        list result;
        for (const auto& item : src)
            result.append(pybind11::cast(item, policy, parent));
        return result.release();
    }
};


// QMap<QString, QString> <-> Python dict[str, str]
using QStringMap = QMap<QString, QString>;

template <>
struct type_caster<QStringMap> {
public:
    PYBIND11_TYPE_CASTER(QStringMap, const_name("dict[str, str]"));

    bool load(handle src, bool) {
        if (!isinstance<dict>(src)) return false;
        auto d = reinterpret_borrow<dict>(src);
        value.clear();
        for (auto item : d)
            value.insert(item.first.cast<QString>(), item.second.cast<QString>());
        return true;
    }

    static handle cast(const QStringMap& src, return_value_policy, handle) {
        dict result;
        for (auto it = src.constBegin(); it != src.constEnd(); ++it)
            result[pybind11::cast(it.key())] = pybind11::cast(it.value());
        return result.release();
    }
};


// QFlags<Qt::AlignmentFlag> <-> Python int
//
// Без този кастер `ModelTextObject` не може да се построи от Python изобщо:
// константите се изнасят като голи int (`AlignLeft` = 1, `AlignTop` = 32), а
// конструкторът иска `QFlags<Qt::AlignmentFlag>`, за който pybind11 няма
// вграден превод. Всяко извикване на `Label.add_text` падаше с TypeError.
//
// ⚠️ Приема се и вече готов QFlags, и цяло число — второто, защото точно така
// изглеждат изнесените константи от `bind_types.cpp`, а и защото подравняването
// естествено се комбинира с побитово ИЛИ, което в Python дава int.
template <>
struct type_caster<QFlags<Qt::AlignmentFlag>> {
public:
    PYBIND11_TYPE_CASTER(QFlags<Qt::AlignmentFlag>, const_name("int"));

    bool load(handle src, bool) {
        if (!src || src.is_none()) return false;
        PyObject* index = PyNumber_Index(src.ptr());
        if (!index) {
            PyErr_Clear();
            return false;
        }
        const long raw = PyLong_AsLong(index);
        Py_DECREF(index);
        if (raw == -1 && PyErr_Occurred()) {
            PyErr_Clear();
            return false;
        }
        value = QFlags<Qt::AlignmentFlag>(static_cast<Qt::AlignmentFlag>(raw));
        return true;
    }

    static handle cast(const QFlags<Qt::AlignmentFlag>& src,
                       return_value_policy, handle) {
        return PyLong_FromLong(static_cast<long>(src.toInt()));
    }
};


}  // namespace pybind11::detail

#endif // QT_TYPE_CASTERS_HPP
