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

// Objects are created against a package's database connection. Subjects, studies,
// and series also need the package's directory formats to build virtual paths.
template <class T> void applyDirFormat(T &, squirrel &) {}
void applyDirFormat(squirrelSubject &o, squirrel &p) { o.SetDirFormat(p.SubjectDirFormat, p.StudyDirFormat, p.SeriesDirFormat); }
void applyDirFormat(squirrelStudy &o, squirrel &p) { o.SetDirFormat(p.SubjectDirFormat, p.StudyDirFormat, p.SeriesDirFormat); }
void applyDirFormat(squirrelSeries &o, squirrel &p) { o.SetDirFormat(p.SubjectDirFormat, p.StudyDirFormat, p.SeriesDirFormat); }

// Store the object, raising SquirrelError if it is invalid. Store() also returns
// true when a *new* object collided with an existing row (it adopts the existing
// row ID and sets Error()), which is surfaced as an error too.
template <class T> qint64 storeOrThrow(T &o) {
    bool isNew = o.GetObjectID() < 0;
    if (!o.Store()) throw SquirrelError(o.Error().toStdString());
    if (isNew && !o.Error().isEmpty()) throw SquirrelError(o.Error().toStdString());
    return o.GetObjectID();
}

// Members shared by every object class (subject, study, series, ...).
template <class T> py::class_<T> bindObject(py::module_ &m, const char *name) {
    return py::class_<T>(m, name)
        .def(py::init([](squirrel &pkg) {
            T o(pkg.GetDatabaseUUID());
            applyDirFormat(o, pkg);
            return o;
        }), py::arg("package"), py::keep_alive<1, 2>())
        .def_property_readonly("row_id", &T::GetObjectID)
        .def("store", &storeOrThrow<T>)
        .def("is_valid", &T::isValid)
        .def("error", &T::Error);
}

template <class T> void failIfFalse(bool ok, T &s) {
    if (!ok) throw SquirrelError(s.GetLog().toStdString());
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
        .value("SUBJECT", Subject)
        .value("UNKNOWN", UnknownObjectType);

    // ----- objects -----
    bindObject<squirrelSubject>(m, "Subject")
        .def_readwrite("id", &squirrelSubject::ID)
        .def_readwrite("guid", &squirrelSubject::GUID)
        .def_readwrite("date_of_birth", &squirrelSubject::DateOfBirth)
        .def_readwrite("sex", &squirrelSubject::Sex)
        .def_readwrite("gender", &squirrelSubject::Gender)
        .def_readwrite("ethnicity1", &squirrelSubject::Ethnicity1)
        .def_readwrite("ethnicity2", &squirrelSubject::Ethnicity2)
        .def_readwrite("enrollment_group", &squirrelSubject::EnrollmentGroup)
        .def_readwrite("enrollment_status", &squirrelSubject::EnrollmentStatus)
        .def_readwrite("alternate_ids", &squirrelSubject::AlternateIDs)
        .def_readwrite("notes", &squirrelSubject::Notes)
        .def_readwrite("sequence_number", &squirrelSubject::SequenceNumber)
        .def("virtual_path", &squirrelSubject::VirtualPath)
        .def("next_study_number", &squirrelSubject::GetNextStudyNumber)
        .def("__repr__", [](const squirrelSubject &s) {
            return "<squirrel.Subject id=" + s.ID.toStdString() + ">";
        });

    bindObject<squirrelStudy>(m, "Study")
        .def_readwrite("subject_row_id", &squirrelStudy::subjectRowID)
        .def_readwrite("study_number", &squirrelStudy::StudyNumber)
        .def_readwrite("datetime", &squirrelStudy::DateTime)
        .def_readwrite("description", &squirrelStudy::Description)
        .def_readwrite("equipment", &squirrelStudy::Equipment)
        .def_readwrite("modality", &squirrelStudy::Modality)
        .def_readwrite("notes", &squirrelStudy::Notes)
        .def_readwrite("study_uid", &squirrelStudy::StudyUID)
        .def_readwrite("visit_type", &squirrelStudy::VisitType)
        .def_readwrite("age_at_study", &squirrelStudy::AgeAtStudy)
        .def_readwrite("height", &squirrelStudy::Height)
        .def_readwrite("weight", &squirrelStudy::Weight)
        .def_readwrite("day_number", &squirrelStudy::DayNumber)
        .def_readwrite("time_point", &squirrelStudy::TimePoint)
        .def_readwrite("sequence_number", &squirrelStudy::SequenceNumber)
        .def("virtual_path", &squirrelStudy::VirtualPath)
        .def("next_series_number", &squirrelStudy::GetNextSeriesNumber)
        .def("__repr__", [](const squirrelStudy &s) {
            return "<squirrel.Study study_number=" + std::to_string(s.StudyNumber) + ">";
        });

    bindObject<squirrelSeries>(m, "Series")
        .def_readwrite("study_row_id", &squirrelSeries::studyRowID)
        .def_readwrite("subject_row_id", &squirrelSeries::subjectRowID)
        .def_readwrite("experiment_row_id", &squirrelSeries::experimentRowID)
        .def_readwrite("series_number", &squirrelSeries::SeriesNumber)
        .def_readwrite("datetime", &squirrelSeries::DateTime)
        .def_readwrite("params", &squirrelSeries::params)
        .def_readwrite("bids_entity", &squirrelSeries::BidsEntity)
        .def_readwrite("bids_suffix", &squirrelSeries::BidsSuffix)
        .def_readwrite("bids_task", &squirrelSeries::BidsTask)
        .def_readwrite("bids_run", &squirrelSeries::BidsRun)
        .def_readwrite("bids_phase_encoding_direction", &squirrelSeries::BidsPhaseEncodingDirection)
        .def_readwrite("description", &squirrelSeries::Description)
        .def_readwrite("protocol", &squirrelSeries::Protocol)
        .def_readwrite("series_uid", &squirrelSeries::SeriesUID)
        .def_readwrite("sequence_number", &squirrelSeries::SequenceNumber)
        .def_readwrite("run", &squirrelSeries::Run)
        .def_readwrite("size", &squirrelSeries::Size)
        .def_readwrite("file_count", &squirrelSeries::FileCount)
        .def_readwrite("behavioral_size", &squirrelSeries::BehavioralSize)
        .def_readwrite("behavioral_file_count", &squirrelSeries::BehavioralFileCount)
        .def_readwrite("staged_files", &squirrelSeries::stagedFiles)
        .def_readwrite("staged_beh_files", &squirrelSeries::stagedBehFiles)
        .def_readonly("files", &squirrelSeries::files)
        .def_readonly("beh_files", &squirrelSeries::behFiles)
        .def("virtual_path", &squirrelSeries::VirtualPath)
        .def("__repr__", [](const squirrelSeries &s) {
            return "<squirrel.Series series_number=" + std::to_string(s.SeriesNumber) + ">";
        });

    bindObject<squirrelAnalysis>(m, "Analysis")
        .def_readwrite("study_row_id", &squirrelAnalysis::studyRowID)
        .def_readwrite("pipeline_row_id", &squirrelAnalysis::pipelineRowID)
        .def_readwrite("date_cluster_end", &squirrelAnalysis::DateClusterEnd)
        .def_readwrite("date_cluster_start", &squirrelAnalysis::DateClusterStart)
        .def_readwrite("date_end", &squirrelAnalysis::DateEnd)
        .def_readwrite("date_start", &squirrelAnalysis::DateStart)
        .def_readwrite("analysis_name", &squirrelAnalysis::AnalysisName)
        .def_readwrite("hostname", &squirrelAnalysis::Hostname)
        .def_readwrite("status_message", &squirrelAnalysis::StatusMessage)
        .def_readwrite("pipeline_name", &squirrelAnalysis::PipelineName)
        .def_readwrite("status", &squirrelAnalysis::Status)
        .def_readwrite("successful", &squirrelAnalysis::Successful)
        .def_readwrite("pipeline_version", &squirrelAnalysis::PipelineVersion)
        .def_readwrite("series_count", &squirrelAnalysis::SeriesCount)
        .def_readwrite("run_time", &squirrelAnalysis::RunTime)
        .def_readwrite("setup_time", &squirrelAnalysis::SetupTime)
        .def_readwrite("size", &squirrelAnalysis::Size)
        .def_readwrite("staged_files", &squirrelAnalysis::stagedFiles)
        .def("virtual_path", &squirrelAnalysis::VirtualPath);

    bindObject<squirrelExperiment>(m, "Experiment")
        .def_readwrite("name", &squirrelExperiment::ExperimentName)
        .def_readwrite("file_count", &squirrelExperiment::FileCount)
        .def_readwrite("size", &squirrelExperiment::Size)
        .def_readwrite("staged_files", &squirrelExperiment::stagedFiles)
        .def("virtual_path", &squirrelExperiment::VirtualPath);

    bindObject<squirrelPipeline>(m, "Pipeline")
        .def_readwrite("create_date", &squirrelPipeline::PipelineCreateDate)
        .def_readwrite("cluster_engine", &squirrelPipeline::ClusterEngine)
        .def_readwrite("cluster_queue", &squirrelPipeline::ClusterQueue)
        .def_readwrite("cluster_submit_host", &squirrelPipeline::ClusterSubmitHost)
        .def_readwrite("cluster_user", &squirrelPipeline::ClusterUser)
        .def_readwrite("description", &squirrelPipeline::PipelineDescription)
        .def_readwrite("directory", &squirrelPipeline::PipelineDirectory)
        .def_readwrite("directory_structure", &squirrelPipeline::PipelineDirectoryStructure)
        .def_readwrite("name", &squirrelPipeline::PipelineName)
        .def_readwrite("notes", &squirrelPipeline::PipelineNotes)
        .def_readwrite("primary_script", &squirrelPipeline::PipelinePrimaryScript)
        .def_readwrite("result_script", &squirrelPipeline::PipelineResultScript)
        .def_readwrite("secondary_script", &squirrelPipeline::PipelineSecondaryScript)
        .def_readwrite("search_dependency_level", &squirrelPipeline::SearchDependencyLevel)
        .def_readwrite("search_dependency_link_type", &squirrelPipeline::SearchDependencyLinkType)
        .def_readwrite("search_group", &squirrelPipeline::SearchGroup)
        .def_readwrite("search_group_type", &squirrelPipeline::SearchGroupType)
        .def_readwrite("setup_bids_directory", &squirrelPipeline::SetupBidsDirectory)
        .def_readwrite("setup_data_copy_method", &squirrelPipeline::SetupDataCopyMethod)
        .def_readwrite("setup_dependency_directory", &squirrelPipeline::SetupDependencyDirectory)
        .def_readwrite("setup_temp_directory", &squirrelPipeline::SetupTempDirectory)
        .def_readwrite("setup_write_bids", &squirrelPipeline::SetupWriteBids)
        .def_readwrite("complete_files", &squirrelPipeline::PipelineCompleteFiles)
        .def_readwrite("search_parent_pipelines", &squirrelPipeline::SearchParentPipelines)
        .def_readwrite("cluster_max_wall_time", &squirrelPipeline::ClusterMaxWallTime)
        .def_readwrite("cluster_memory", &squirrelPipeline::ClusterMemory)
        .def_readwrite("cluster_number_concurrent_analyses", &squirrelPipeline::ClusterNumberConcurrentAnalyses)
        .def_readwrite("cluster_number_cores", &squirrelPipeline::ClusterNumberCores)
        .def_readwrite("cluster_submit_delay", &squirrelPipeline::ClusterSubmitDelay)
        .def_readwrite("analysis_level", &squirrelPipeline::PipelineAnalysisLevel)
        .def_readwrite("version", &squirrelPipeline::PipelineVersion)
        .def_property("setup_use_profile",
            [](const squirrelPipeline &p) { return p.flags.SetupUseProfile; },
            [](squirrelPipeline &p, bool v) { p.flags.SetupUseProfile = v; })
        .def_property("setup_use_temp_directory",
            [](const squirrelPipeline &p) { return p.flags.SetupUseTempDirectory; },
            [](squirrelPipeline &p, bool v) { p.flags.SetupUseTempDirectory = v; })
        .def_readwrite("staged_files", &squirrelPipeline::stagedFiles)
        .def("virtual_path", &squirrelPipeline::VirtualPath);

    bindObject<squirrelObservation>(m, "Observation")
        .def_readwrite("subject_row_id", &squirrelObservation::subjectRowID)
        .def_readwrite("date_end", &squirrelObservation::DateEnd)
        .def_readwrite("date_record_create", &squirrelObservation::DateRecordCreate)
        .def_readwrite("date_record_entry", &squirrelObservation::DateRecordEntry)
        .def_readwrite("date_record_modify", &squirrelObservation::DateRecordModify)
        .def_readwrite("date_start", &squirrelObservation::DateStart)
        .def_readwrite("description", &squirrelObservation::Description)
        .def_readwrite("instrument_name", &squirrelObservation::InstrumentName)
        .def_readwrite("name", &squirrelObservation::ObservationName)
        .def_readwrite("type", &squirrelObservation::ObservationType)
        .def_readwrite("notes", &squirrelObservation::Notes)
        .def_readwrite("rater", &squirrelObservation::Rater)
        .def_readwrite("value", &squirrelObservation::Value)
        .def_readwrite("duration", &squirrelObservation::Duration);

    bindObject<squirrelIntervention>(m, "Intervention")
        .def_readwrite("subject_row_id", &squirrelIntervention::subjectRowID)
        .def_readwrite("date_end", &squirrelIntervention::DateEnd)
        .def_readwrite("date_record_create", &squirrelIntervention::DateRecordCreate)
        .def_readwrite("date_record_entry", &squirrelIntervention::DateRecordEntry)
        .def_readwrite("date_record_modify", &squirrelIntervention::DateRecordModify)
        .def_readwrite("date_start", &squirrelIntervention::DateStart)
        .def_readwrite("administration_route", &squirrelIntervention::AdministrationRoute)
        .def_readwrite("description", &squirrelIntervention::Description)
        .def_readwrite("dose_frequency", &squirrelIntervention::DoseFrequency)
        .def_readwrite("dose_key", &squirrelIntervention::DoseKey)
        .def_readwrite("dose_string", &squirrelIntervention::DoseString)
        .def_readwrite("dose_unit", &squirrelIntervention::DoseUnit)
        .def_readwrite("intervention_class", &squirrelIntervention::InterventionClass)
        .def_readwrite("name", &squirrelIntervention::InterventionName)
        .def_readwrite("notes", &squirrelIntervention::Notes)
        .def_readwrite("rater", &squirrelIntervention::Rater)
        .def_readwrite("dose_amount", &squirrelIntervention::DoseAmount);

    bindObject<squirrelGroupAnalysis>(m, "GroupAnalysis")
        .def_readwrite("name", &squirrelGroupAnalysis::GroupAnalysisName)
        .def_readwrite("datetime", &squirrelGroupAnalysis::DateTime)
        .def_readwrite("description", &squirrelGroupAnalysis::Description)
        .def_readwrite("notes", &squirrelGroupAnalysis::Notes)
        .def_readwrite("file_count", &squirrelGroupAnalysis::FileCount)
        .def_readwrite("size", &squirrelGroupAnalysis::Size)
        .def_readwrite("staged_files", &squirrelGroupAnalysis::stagedFiles)
        .def("virtual_path", &squirrelGroupAnalysis::VirtualPath);

    bindObject<squirrelDataDictionary>(m, "DataDictionary")
        .def_readwrite("name", &squirrelDataDictionary::DataDictionaryName)
        .def_readwrite("file_count", &squirrelDataDictionary::FileCount)
        .def_readwrite("size", &squirrelDataDictionary::Size)
        .def_readwrite("staged_files", &squirrelDataDictionary::stagedFiles)
        .def("virtual_path", &squirrelDataDictionary::VirtualPath);

    // ----- package -----
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
        .def("set_use_embedded_database", &squirrel::SetUseEmbeddedDatabase, py::arg("use"))
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
        .def_readwrite("series_dir_format", &squirrel::SeriesDirFormat)
        .def_readwrite("study_dir_format", &squirrel::StudyDirFormat)
        .def_readwrite("subject_dir_format", &squirrel::SubjectDirFormat)
        .def_readonly("squirrel_version", &squirrel::SquirrelVersion)
        .def_readonly("squirrel_build", &squirrel::SquirrelBuild)

        // operations: failures raise SquirrelError with the library log as the message
        .def("read", [](squirrel &s) { failIfFalse(s.Read(), s); })
        .def("write", [](squirrel &s) { failIfFalse(s.Write(), s); })
        .def("write_update", [](squirrel &s) { failIfFalse(s.WriteUpdate(), s); })
        .def("extract", [](squirrel &s, const QString &dest) {
            QString msg;
            if (!s.Extract(dest, msg)) throw SquirrelError(msg.toStdString());
            return msg;
        }, py::arg("destination"))
        .def("extract_object", [](squirrel &s, ObjectType o, qint64 rowId, const QString &outDir) {
            QString msg;
            if (!s.ExtractObject(o, rowId, outDir, msg)) throw SquirrelError(msg.toStdString());
            return msg;
        }, py::arg("object"), py::arg("row_id"), py::arg("out_dir"))
        .def("remove_object", &squirrel::RemoveObject, py::arg("object"), py::arg("row_id"))
        .def("add_staged_files", [](squirrel &s, ObjectType o, qint64 rowId, const QStringList &files) {
            if (!s.AddStagedFiles(o, rowId, files)) throw SquirrelError("unable to stage files for row " + std::to_string(rowId));
        }, py::arg("object"), py::arg("row_id"), py::arg("files"))
        .def("validate", &squirrel::Validate)
        .def("print", &squirrel::Print, py::arg("detail") = false)
        .def("print_tree", &squirrel::PrintTree)
        .def("resequence_all", &squirrel::ResequenceAll)
        .def("is_valid", [](squirrel &s) { return s.IsValid(); })
        .def("is_modified", [](squirrel &s) { return s.IsModified(); })

        // information
        .def("get_log", &squirrel::GetLog)
        .def("get_log_buffer", &squirrel::GetLogBuffer)
        .def("get_progress", &squirrel::GetProgress)
        .def("get_database_uuid", [](squirrel &s) { return s.GetDatabaseUUID(); })
        .def("get_object_count", &squirrel::GetObjectCount, py::arg("object"))
        .def("get_file_count", &squirrel::GetFileCount)
        .def("get_unzip_size", &squirrel::GetUnzipSize)
        .def("count_studies", &squirrel::CountStudies, py::arg("subject_row_id"))
        .def("count_series", &squirrel::CountSeries, py::arg("study_row_id"))
        .def("count_observations", &squirrel::CountObservations, py::arg("subject_row_id"))
        .def("count_interventions", &squirrel::CountInterventions, py::arg("subject_row_id"))

        // lists
        .def("get_subject_list", &squirrel::GetSubjectList)
        .def("get_study_list", &squirrel::GetStudyList, py::arg("subject_row_id"))
        .def("get_series_list", &squirrel::GetSeriesList, py::arg("study_row_id"))
        .def("get_analysis_list", &squirrel::GetAnalysisList, py::arg("study_row_id"))
        .def("get_observation_list", &squirrel::GetObservationList, py::arg("subject_row_id"))
        .def("get_intervention_list", &squirrel::GetInterventionList, py::arg("subject_row_id"))
        .def("get_experiment_list", &squirrel::GetExperimentList)
        .def("get_pipeline_list", &squirrel::GetPipelineList)
        .def("get_group_analysis_list", &squirrel::GetGroupAnalysisList)
        .def("get_data_dictionary_list", &squirrel::GetDataDictionaryList)

        // single objects by row ID
        .def("get_subject", &squirrel::GetSubject, py::arg("row_id"))
        .def("get_study", &squirrel::GetStudy, py::arg("row_id"))
        .def("get_series", &squirrel::GetSeries, py::arg("row_id"))
        .def("get_analysis", &squirrel::GetAnalysis, py::arg("row_id"))
        .def("get_observation", &squirrel::GetObservation, py::arg("row_id"))
        .def("get_intervention", &squirrel::GetIntervention, py::arg("row_id"))
        .def("get_experiment", &squirrel::GetExperiment, py::arg("row_id"))
        .def("get_pipeline", &squirrel::GetPipeline, py::arg("row_id"))
        .def("get_group_analysis", &squirrel::GetGroupAnalysis, py::arg("row_id"))
        .def("get_data_dictionary", &squirrel::GetDataDictionary, py::arg("row_id"))

        // finders: return a row ID, or -1 if not found
        .def("find_subject", &squirrel::FindSubject, py::arg("id"))
        .def("find_study", &squirrel::FindStudy, py::arg("subject_id"), py::arg("study_number"))
        .def("find_study_by_uid", &squirrel::FindStudyByUID, py::arg("study_uid"))
        .def("find_series", &squirrel::FindSeries, py::arg("subject_id"), py::arg("study_number"), py::arg("series_number"))
        .def("find_series_by_uid", &squirrel::FindSeriesByUID, py::arg("series_uid"))
        .def("find_analysis", &squirrel::FindAnalysis, py::arg("subject_id"), py::arg("study_number"), py::arg("name"))
        .def("find_experiment", &squirrel::FindExperiment, py::arg("name"))
        .def("find_pipeline", &squirrel::FindPipeline, py::arg("name"), py::arg("version") = -1)
        .def("find_group_analysis", &squirrel::FindGroupAnalysis, py::arg("name"))
        .def("find_data_dictionary", &squirrel::FindDataDictionary, py::arg("name"))
        .def("find_observation", &squirrel::FindObservation, py::arg("subject_id"), py::arg("name"), py::arg("date_start"))
        .def("find_intervention", &squirrel::FindIntervention, py::arg("subject_id"), py::arg("name"), py::arg("date_start"));
}
