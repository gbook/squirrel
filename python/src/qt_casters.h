// All Qt <-> Python type conversions live here, so that porting the bindings to
// another binding library (e.g. nanobind) only requires rewriting this file.
#ifndef QT_CASTERS_H
#define QT_CASTERS_H

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <QString>
#include <QList>
#include <QDate>
#include <QDateTime>

namespace pybind11 { namespace detail {

/* QString <-> str */
template <> struct type_caster<QString> {
    PYBIND11_TYPE_CASTER(QString, const_name("str"));

    bool load(handle src, bool) {
        if (!src || !PyUnicode_Check(src.ptr())) return false;
        Py_ssize_t size = 0;
        const char *utf8 = PyUnicode_AsUTF8AndSize(src.ptr(), &size);
        if (!utf8) { PyErr_Clear(); return false; }
        value = QString::fromUtf8(utf8, static_cast<qsizetype>(size));
        return true;
    }
    static handle cast(const QString &s, return_value_policy, handle) {
        QByteArray u = s.toUtf8();
        return PyUnicode_FromStringAndSize(u.constData(), u.size());
    }
};

/* QList<T> (and therefore QStringList) <-> list */
template <typename T> struct type_caster<QList<T>> : list_caster<QList<T>, T> {};

/* QDate <-> datetime.date (invalid QDate -> None) */
template <> struct type_caster<QDate> {
    PYBIND11_TYPE_CASTER(QDate, const_name("datetime.date | None"));

    bool load(handle src, bool) {
        if (!src) return false;
        if (src.is_none()) { value = QDate(); return true; }
        auto date = module_::import("datetime").attr("date");
        if (!isinstance(src, date)) return false;
        value = QDate(src.attr("year").cast<int>(), src.attr("month").cast<int>(), src.attr("day").cast<int>());
        return true;
    }
    static handle cast(const QDate &d, return_value_policy, handle) {
        if (!d.isValid()) return none().release();
        return module_::import("datetime").attr("date")(d.year(), d.month(), d.day()).release();
    }
};

/* QDateTime <-> datetime.datetime (invalid QDateTime -> None; naive local time) */
template <> struct type_caster<QDateTime> {
    PYBIND11_TYPE_CASTER(QDateTime, const_name("datetime.datetime | None"));

    bool load(handle src, bool) {
        if (!src) return false;
        if (src.is_none()) { value = QDateTime(); return true; }
        auto dt = module_::import("datetime").attr("datetime");
        if (!isinstance(src, dt)) return false;
        value = QDateTime(QDate(src.attr("year").cast<int>(), src.attr("month").cast<int>(), src.attr("day").cast<int>()),
                          QTime(src.attr("hour").cast<int>(), src.attr("minute").cast<int>(), src.attr("second").cast<int>(),
                                src.attr("microsecond").cast<int>() / 1000));
        return true;
    }
    static handle cast(const QDateTime &d, return_value_policy, handle) {
        if (!d.isValid()) return none().release();
        QDate date = d.date();
        QTime t = d.time();
        return module_::import("datetime").attr("datetime")(
            date.year(), date.month(), date.day(), t.hour(), t.minute(), t.second(), t.msec() * 1000).release();
    }
};

}} // namespace pybind11::detail

#endif // QT_CASTERS_H
