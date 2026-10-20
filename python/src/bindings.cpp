// Thin pybind11 bindings for libsquirrel. Keep logic out of this file: anything
// that can be done in Python belongs in squirrel/__init__.py.
#include <stdexcept>
#include <QCoreApplication>
#include "qt_casters.h"
#include "squirrel.h"

namespace py = pybind11;

struct SquirrelError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

namespace {

// Qt's SQL drivers need a QCoreApplication to locate their plugins.
void ensureCoreApplication() {
    if (QCoreApplication::instance()) return;
    static int argc = 1;
    static char arg0[] = "squirrel";
    static char *argv[] = { arg0, nullptr };
    static QCoreApplication *app = new QCoreApplication(argc, argv);
    (void)app;
}

} // namespace

PYBIND11_MODULE(_squirrel, m) {
    m.doc() = "Low-level bindings for libsquirrel. Use the 'squirrel' package instead.";

    ensureCoreApplication();
    py::register_exception<SquirrelError>(m, "SquirrelError", PyExc_RuntimeError);

    py::enum_<ObjectType>(m, "ObjectType")
        .value("ANALYSIS", Analysis)
        .value("BEH_SERIES", BehSeries)
        .value("DATA_DICTIONARY", DataDictionary)
        .value("DATA_DICTIONARY_ITEM", DataDictionaryItem)
        .value("DATA_STEP", DataStep)
        .value("EXPERIMENT", Experiment)
        .value("GROUP_ANALYSIS", GroupAnalysis)
        .value("INTERVENTION", Intervention)
        .value("OBSERVATION", Observation)
        .value("PACKAGE", Package)
        .value("PARAMS", Params)
        .value("PIPELINE", Pipeline)
        .value("SERIES", Series)
        .value("STUDY", Study)
        .value("SUBJECT", Subject);

    py::class_<squirrelSubject>(m, "Subject")
        .def_readonly("id", &squirrelSubject::ID)
        .def_readonly("guid", &squirrelSubject::GUID)
        .def_readonly("date_of_birth", &squirrelSubject::DateOfBirth)
        .def_readonly("sex", &squirrelSubject::Sex)
        .def_readonly("gender", &squirrelSubject::Gender)
        .def_readonly("ethnicity1", &squirrelSubject::Ethnicity1)
        .def_readonly("ethnicity2", &squirrelSubject::Ethnicity2)
        .def_readonly("enrollment_group", &squirrelSubject::EnrollmentGroup)
        .def_readonly("enrollment_status", &squirrelSubject::EnrollmentStatus)
        .def_readonly("alternate_ids", &squirrelSubject::AlternateIDs)
        .def_readonly("notes", &squirrelSubject::Notes)
        .def_readonly("sequence_number", &squirrelSubject::SequenceNumber)
        .def("virtual_path", &squirrelSubject::VirtualPath)
        .def("__repr__", [](const squirrelSubject &s) {
            return "<squirrel.Subject id=" + s.ID.toStdString() + ">";
        });

    // The GIL is held for every call (pybind11 default). A Squirrel object owns a
    // QSqlDatabase connection, which is bound to the thread that created it, so
    // an object must only be used from one thread.
    py::class_<squirrel>(m, "Squirrel")
        .def(py::init<bool, bool>(), py::arg("debug") = false, py::arg("quiet") = false)

        // options
        .def_property("package_path", &squirrel::GetPackagePath, &squirrel::SetPackagePath)
        .def_property("system_temp_dir", &squirrel::GetSystemTempDir, &squirrel::SetSystemTempDir)
        .def_property("debug", &squirrel::GetDebug, &squirrel::SetDebug)
        .def("set_quick_read", &squirrel::SetQuickRead, py::arg("quick"))
        .def("set_overwrite_package", &squirrel::SetOverwritePackage, py::arg("overwrite"))
        .def("set_file_mode", [](squirrel &s, bool existing) {
            s.SetFileMode(existing ? ExistingPackage : NewPackage);
        }, py::arg("existing"))

        // package header fields
        .def_readwrite("name", &squirrel::PackageName)
        .def_readwrite("description", &squirrel::Description)
        .def_readwrite("datetime", &squirrel::Datetime)
        .def_readwrite("changes", &squirrel::Changes)
        .def_readwrite("data_format", &squirrel::DataFormat)
        .def_readwrite("license", &squirrel::License)
        .def_readwrite("notes", &squirrel::Notes)
        .def_readwrite("readme", &squirrel::Readme)
        .def_readonly("squirrel_version", &squirrel::SquirrelVersion)
        .def_readonly("squirrel_build", &squirrel::SquirrelBuild)

        // operations: failures raise SquirrelError with the library log as the message
        .def("read", [](squirrel &s) {
            if (!s.Read()) throw SquirrelError(s.GetLog().toStdString());
        })
        .def("write", [](squirrel &s) {
            if (!s.Write()) throw SquirrelError(s.GetLog().toStdString());
        })
        .def("write_update", [](squirrel &s) {
            if (!s.WriteUpdate()) throw SquirrelError(s.GetLog().toStdString());
        })
        .def("extract", [](squirrel &s, const QString &dest) {
            QString msg;
            if (!s.Extract(dest, msg)) throw SquirrelError(msg.toStdString());
            return msg;
        }, py::arg("destination"))
        .def("validate", &squirrel::Validate)
        .def("print", &squirrel::Print, py::arg("detail") = false)
        .def("print_tree", &squirrel::PrintTree)

        // information
        .def("get_log", &squirrel::GetLog)
        .def("get_progress", &squirrel::GetProgress)
        .def("get_object_count", &squirrel::GetObjectCount, py::arg("object"))
        .def("get_subject_list", &squirrel::GetSubjectList)
        .def("find_subject", &squirrel::FindSubject, py::arg("id"));
}
